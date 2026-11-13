#include <stdexcept>
#include "merged_class.h"

#include <algorithm>

// ==================================================
// GRAPH IMPLEMENTATION
// ==================================================

Graph::Graph(int nodes) {
    n = (nodes < 0) ? 0 :
        (nodes > MAX_ENTRIES ? MAX_ENTRIES : nodes);

    for (int i = 0; i < MAX_ENTRIES; i++) {
        head[i] = nullptr;
        visited[i] = false;
    }
}

Graph::~Graph() {
    for (int i = 0; i < n; i++) {
        AdjNode* p = head[i];

        while (p != nullptr) {
            AdjNode* temp = p;
            p = p->next;
            delete temp;
        }
    }
}

void Graph::addEdge(int a, int b) {
    if (a < 0 || b < 0 || a >= n || b >= n || a == b) {
        return;
    }

    head[a] = new AdjNode{b, head[a]};
    head[b] = new AdjNode{a, head[b]};
}

bool Graph::isVisited(int u) const {
    if (u < 0 || u >= n) {
        return true;
    }

    return visited[u];
}

void Graph::dfs(int u, int members[], int& count) {
    if (u < 0 || u >= n || visited[u]) {
        return;
    }

    visited[u] = true;
    members[count++] = u;

    for (AdjNode* p = head[u]; p != nullptr; p = p->next) {
        if (!visited[p->to]) {
            dfs(p->to, members, count);
        }
    }
}

// ==================================================
// DETECTED GROUP IMPLEMENTATION
// ==================================================

DetectedGroup::DetectedGroup()
    : count(0),
      totalStrength(0),
      valid(true),
      reason("ok"),
      next(nullptr) {
}

void DetectedGroup::addMember(int entryIndex) {
    if (count < MAX_ENTRIES) {
        members[count++] = entryIndex;
    }
}

int DetectedGroup::getCount() const {
    return count;
}

int DetectedGroup::getMember(int i) const {
    if (i < 0 || i >= count) {
        return -1;
    }

    return members[i];
}

void DetectedGroup::setStrength(int s) {
    totalStrength = s;
}

int DetectedGroup::getStrength() const {
    return totalStrength;
}

void DetectedGroup::setInvalid(const string& why) {
    valid = false;
    reason = why;
}

bool DetectedGroup::isValid() const {
    return valid;
}

const string& DetectedGroup::getReason() const {
    return reason;
}

DetectedGroup* DetectedGroup::getNext() const {
    return next;
}

void DetectedGroup::setNext(DetectedGroup* p) {
    next = p;
}

// ==================================================
// GROUP LIST IMPLEMENTATION
// ==================================================

GroupList::GroupList()
    : head(nullptr), tail(nullptr), size(0) {
}

GroupList::~GroupList() {
    while (head != nullptr) {
        DetectedGroup* temp = head;
        head = head->getNext();
        delete temp;
    }

    tail = nullptr;
    size = 0;
}

void GroupList::add(DetectedGroup* g) {
    if (g == nullptr) {
        return;
    }

    if (head == nullptr) {
        head = g;
    }
    else {
        tail->setNext(g);
    }

    tail = g;
    size++;
}

DetectedGroup* GroupList::getHead() const {
    return head;
}

int GroupList::getSize() const {
    return size;
}

// ==================================================
// MERGE DETECTOR IMPLEMENTATION
// ==================================================

MergeDetector::MergeDetector(const SlotEntry e[], int count)
    : entries(e),
      n(count < 0 ? 0 :
        (count > MAX_ENTRIES ? MAX_ENTRIES : count)),
      graph(n) {
}

bool MergeDetector::sameClass(
    const SlotEntry& x,
    const SlotEntry& y
) {
    return x.section != y.section &&
           x.subject == y.subject &&
           x.faculty == y.faculty &&
           x.venue == y.venue &&
           x.duration == y.duration;
}

void MergeDetector::buildGraph() {
    if (entries == nullptr) {
        return;
    }

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (sameClass(entries[i], entries[j])) {
                graph.addEdge(i, j);
            }
        }
    }
}

void MergeDetector::validateGroup(DetectedGroup* g) const {
    if (g == nullptr || g->getCount() == 0) {
        return;
    }

    int total = 0;

    // Calculate the combined strength.
    for (int i = 0; i < g->getCount(); i++) {
        int index = g->getMember(i);

        if (index < 0 || index >= n) {
            g->setInvalid("invalid entry index");
            return;
        }

        Section* s = searchSectionById(entries[index].section);

        if (s == nullptr) {
            g->setInvalid(
                "unknown section " + entries[index].section
            );
            return;
        }

        total += s->strength;
    }

    g->setStrength(total);

    // Check the venue.
    Venue* v = searchVenueById(
        entries[g->getMember(0)].venue
    );

    if (v == nullptr) {
        g->setInvalid("unknown venue");
    }
    else if (
        maxAudSections > 0 &&
        !auditoriumType.empty() &&
        v->type == auditoriumType &&
        g->getCount() > maxAudSections
    ) {
        g->setInvalid(
            "too many sections in auditorium (limit from config.txt)"
        );
    }
    else if (total > v->capacity) {
        g->setInvalid("venue capacity exceeded");
    }
}

void MergeDetector::detect() {
    if (entries == nullptr) {
        return;
    }

    buildGraph();

    for (int i = 0; i < n; i++) {
        if (graph.isVisited(i)) {
            continue;
        }

        int found[MAX_ENTRIES];
        int count = 0;

        graph.dfs(i, found, count);

        // A merged class must contain at least two sections.
        if (count >= 2) {
            DetectedGroup* group = new DetectedGroup();

            for (int j = 0; j < count; j++) {
                group->addMember(found[j]);
            }

            validateGroup(group);
            groups.add(group);
        }
    }
}

int MergeDetector::getGroupCount() const {
    return groups.getSize();
}

const DetectedGroup* MergeDetector::getFirstGroup() const {
    return groups.getHead();
}

const SlotEntry& MergeDetector::getEntry(int i) const {
    if (entries == nullptr || i < 0 || i >= n) {
        throw out_of_range("Invalid merged-class entry index");
    }

    return entries[i];
}

void MergeDetector::print() const {
    if (groups.getSize() == 0) {
        cout << "No merged classes detected.\n";
        return;
    }

    const DetectedGroup* group = groups.getHead();

    while (group != nullptr) {
        cout << "\nMerged class:\n";

        for (int i = 0; i < group->getCount(); i++) {
            int index = group->getMember(i);

            cout << "  Section: "
                 << entries[index].section << '\n';
        }

        cout << "Total strength: "
             << group->getStrength() << '\n';

        if (group->isValid()) {
            cout << "Status: VALID\n";
        }
        else {
            cout << "Status: INVALID - "
                 << group->getReason() << '\n';
        }

        group = group->getNext();
    }
}

// ==================================================
// COLLECT ENTRIES FOR ONE SLOT
// ==================================================

int collectSlotEntries(
    const string& day,
    int slot,
    SlotEntry out[],
    int maxCount
) {
    if (out == nullptr || maxCount <= 0) {
        return 0;
    }

    int count = 0;

    for (TimetableEntry* t = timetableHead;
         t != nullptr && count < maxCount;
         t = t->next) {

        if (t->day != day || t->startSlot != slot) {
            continue;
        }

        string ids = t->sectionOrGroup;

        // Expand declared merged groups into individual sections.
        MergedGroup* declared =
            searchMergedGroupById(t->sectionOrGroup);

        if (declared != nullptr) {
            ids = declared->sectionIds;
        }

        string current;

        for (size_t i = 0;
             i <= ids.size() && count < maxCount;
             i++) {

            if (i == ids.size() || ids[i] == ';') {
                current = trim(current);

                if (!current.empty()) {
                    SlotEntry& entry = out[count++];

                    entry.section = current;
                    entry.subject = t->subject;
                    entry.faculty = t->facultyId;
                    entry.venue = t->venueId;
                    entry.duration = t->duration;
                    entry.row = t;
                }

                current.clear();
            }
            else {
                current += ids[i];
            }
        }
    }

    return count;
}

// ==================================================
// CHECK MERGED CLASSES IN ONE SLOT
// ==================================================

int checkMergedClassesInSlot(
    const string& day,
    int slot,
    MergeProblem problems[],
    int maxProblems,
    int& problemCount
) {
    problemCount = 0;

    SlotEntry entries[MAX_ENTRIES];

    int count = collectSlotEntries(
        day,
        slot,
        entries,
        MAX_ENTRIES
    );

    MergeDetector detector(entries, count);
    detector.detect();

    const DetectedGroup* group =
        detector.getFirstGroup();

    while (group != nullptr) {
        if (
            !group->isValid() &&
            problems != nullptr &&
            problemCount < maxProblems
        ) {
            int index = group->getMember(0);

            problems[problemCount].row =
                detector.getEntry(index).row;

            problems[problemCount].reason =
                group->getReason();

            problemCount++;
        }

        group = group->getNext();
    }

    return detector.getGroupCount();
}

// ==================================================
// DISPLAY MERGED CLASSES FOR THE TIMETABLE
// ==================================================

void showMergedClasses() {
    const string days[] = {
        "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday", "Sunday"
    };

    int lastSlot = slotCount;

    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {

        int endingSlot = t->startSlot + t->duration - 1;

        if (endingSlot > lastSlot) {
            lastSlot = endingSlot;
        }
    }

    cout << "\n========== MERGED CLASS REPORT ==========\n";

    int totalGroups = 0;

    for (const string& day : days) {
        for (int slot = 1; slot <= lastSlot; slot++) {
            SlotEntry entries[MAX_ENTRIES];

            int count = collectSlotEntries(
                day,
                slot,
                entries,
                MAX_ENTRIES
            );

            if (count == 0) {
                continue;
            }

            MergeDetector detector(entries, count);
            detector.detect();

            if (detector.getGroupCount() > 0) {
                cout << "\n" << day
                     << " - Slot " << slot << '\n';

                detector.print();

                totalGroups += detector.getGroupCount();
            }
        }
    }

    if (totalGroups == 0) {
        cout << "No merged classes found.\n";
    }

    cout << "\nTotal merged groups: "
         << totalGroups << '\n';

    cout << "=========================================\n";
}
