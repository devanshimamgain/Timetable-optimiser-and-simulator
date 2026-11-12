// modules/merged_class.cpp  (Person 2: merged classes)
//
// A merged class is one class that several sections attend together: the same subject, teacher
// and room, the same start slot and the same length, but different sections.
//
// For every slot the rows that start there are joined into groups with a graph (adjacency lists)
// and a depth first search (DFS). Each group is then checked against the university rules:
//   - the room must have enough seats for all the students of the group
//   - the auditorium limit (max_sections_in_auditorium) must be respected
//
// This file is not a program on its own. multit.cpp includes it, after person1_file_handling.cpp.

const int MAX_ENTRIES = 500;                  // most rows in one slot

// One row of one slot. A row that names a merged group (e.g. CEC-G1 = A;B;C) becomes one
// SlotEntry per section. `row` points back to the timetable row it came from.
struct SlotEntry {
    string section;
    string subject;
    string faculty;
    string venue;
    int duration;
    TimetableEntry* row;
};

// A problem found in one merged group: the timetable row and the reason.
struct MergeProblem {
    TimetableEntry* row;
    string reason;
};

// ---------------------------------------------------------------- graph
struct AdjNode {
    int to;
    AdjNode* next;
};

class Graph {
private:
    int n;
    AdjNode* head[MAX_ENTRIES];
    bool visited[MAX_ENTRIES];

public:
    explicit Graph(int nodes) : n(nodes) {
        for (int i = 0; i < n; i++) {
            head[i] = nullptr;
            visited[i] = false;
        }
    }

    ~Graph() {
        for (int i = 0; i < n; i++) {
            AdjNode* p = head[i];
            while (p) {
                AdjNode* tmp = p;
                p = p->next;
                delete tmp;
            }
        }
    }

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    // An undirected edge: a and b are the same merged class.
    void addEdge(int a, int b) {
        head[a] = new AdjNode{b, head[a]};
        head[b] = new AdjNode{a, head[b]};
    }

    bool isVisited(int u) const { return visited[u]; }

    // Depth first search from u. Writes every node it reaches into members[].
    void dfs(int u, int members[], int& count) {
        visited[u] = true;
        members[count++] = u;
        for (AdjNode* p = head[u]; p != nullptr; p = p->next)
            if (!visited[p->to]) dfs(p->to, members, count);
    }
};

// ---------------------------------------------------------------- groups
// One merged group found by the DFS.
class DetectedGroup {
private:
    int members[MAX_ENTRIES];
    int count;
    int totalStrength;
    bool valid;
    string reason;
    DetectedGroup* next;

public:
    DetectedGroup() : count(0), totalStrength(0), valid(true), reason("ok"), next(nullptr) {}

    void addMember(int entryIndex) { members[count++] = entryIndex; }
    int getCount() const { return count; }
    int getMember(int i) const { return members[i]; }

    void setStrength(int s) { totalStrength = s; }
    int getStrength() const { return totalStrength; }

    void setInvalid(const string& why) { valid = false; reason = why; }
    bool isValid() const { return valid; }
    const string& getReason() const { return reason; }

    DetectedGroup* getNext() const { return next; }
    void setNext(DetectedGroup* p) { next = p; }
};

// A list of the groups of one slot.
class GroupList {
private:
    DetectedGroup* head;
    DetectedGroup* tail;
    int size;

public:
    GroupList() : head(nullptr), tail(nullptr), size(0) {}

    ~GroupList() {
        while (head) {
            DetectedGroup* tmp = head;
            head = head->getNext();
            delete tmp;
        }
    }

    GroupList(const GroupList&) = delete;
    GroupList& operator=(const GroupList&) = delete;

    void add(DetectedGroup* g) {
        if (head == nullptr) head = g;
        else tail->setNext(g);
        tail = g;
        size++;
    }

    DetectedGroup* getHead() const { return head; }
    int getSize() const { return size; }
};

// ---------------------------------------------------------------- the detector
class MergeDetector {
private:
    const SlotEntry* entries;
    int n;
    Graph graph;
    GroupList groups;

    // The same class for different sections, with the same length.
    static bool sameClass(const SlotEntry& x, const SlotEntry& y) {
        return x.section != y.section &&
               x.subject == y.subject &&
               x.faculty == y.faculty &&
               x.venue == y.venue &&
               x.duration == y.duration;
    }

    // Joins every two rows that are the same class.
    void buildGraph() {
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (sameClass(entries[i], entries[j])) graph.addEdge(i, j);
    }

    // The university rules for one group.
    void validateGroup(DetectedGroup* g) const {
        int total = 0;
        for (int i = 0; i < g->getCount(); i++) {
            Section* s = searchSectionById(entries[g->getMember(i)].section);
            if (s == nullptr) {
                g->setInvalid("unknown section " + entries[g->getMember(i)].section);
                return;
            }
            total += s->strength;
        }
        g->setStrength(total);

        Venue* v = searchVenueById(entries[g->getMember(0)].venue);
        if (v == nullptr)
            g->setInvalid("unknown venue");
        else if (maxAudSections > 0 && !auditoriumType.empty() && v->type == auditoriumType &&
                 g->getCount() > maxAudSections)
            g->setInvalid("too many sections in auditorium (limit from config.txt)");
        else if (total > v->capacity)
            g->setInvalid("venue capacity exceeded");
    }

public:
    MergeDetector(const SlotEntry e[], int count)
        : entries(e), n(count > MAX_ENTRIES ? MAX_ENTRIES : count), graph(n) {}

    // Finds every group of two or more rows, and checks each one.
    void detect() {
        buildGraph();
        for (int i = 0; i < n; i++) {
            if (graph.isVisited(i)) continue;

            int found[MAX_ENTRIES];
            int cnt = 0;
            graph.dfs(i, found, cnt);

            if (cnt >= 2) {
                DetectedGroup* g = new DetectedGroup();
                for (int k = 0; k < cnt; k++) g->addMember(found[k]);
                validateGroup(g);
                groups.add(g);
            }
        }
    }

    int getGroupCount() const { return groups.getSize(); }
    const DetectedGroup* getFirstGroup() const { return groups.getHead(); }
    const SlotEntry& getEntry(int i) const { return entries[i]; }
};

// ---------------------------------------------------------------- rows of one slot
// Gathers the rows that start in this slot. A row that names a merged group becomes one row per section.
int collectSlotEntries(const string& day, int slot, SlotEntry out[], int maxCount) {
    int n = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        if (t->day != day || t->startSlot != slot) continue;   // a class is counted in its start slot only

        string ids = t->sectionOrGroup;
        MergedGroup* declared = searchMergedGroupById(t->sectionOrGroup);
        if (declared != nullptr) ids = declared->sectionIds;

        string current;
        for (size_t i = 0; i <= ids.size(); i++) {
            if (i == ids.size() || ids[i] == ';') {
                current = trim(current);
                if (!current.empty() && n < maxCount) {
                    SlotEntry& e = out[n++];
                    e.section = current;
                    e.subject = t->subject;
                    e.faculty = t->facultyId;
                    e.venue = t->venueId;
                    e.duration = t->duration;
                    e.row = t;
                }
                current.clear();
            } else {
                current += ids[i];
            }
        }
    }
    return n;
}

// Runs the merged-class check for one slot.
// Returns how many merged groups were found. The invalid ones are written into problems[]
// (up to maxProblems), and problemCount says how many.
int checkMergedClassesInSlot(const string& day, int slot, MergeProblem problems[], int maxProblems,
                             int& problemCount) {
    SlotEntry entries[MAX_ENTRIES];
    int n = collectSlotEntries(day, slot, entries, MAX_ENTRIES);

    MergeDetector detector(entries, n);
    detector.detect();

    problemCount = 0;
    for (const DetectedGroup* g = detector.getFirstGroup(); g != nullptr; g = g->getNext()) {
        if (g->isValid() || problemCount >= maxProblems) continue;
        problems[problemCount].row = detector.getEntry(g->getMember(0)).row;
        problems[problemCount].reason = g->getReason();
        problemCount++;
    }
    return detector.getGroupCount();
}
