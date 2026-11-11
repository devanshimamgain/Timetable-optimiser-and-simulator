#include "clashdetect.h"
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

namespace {

const int MAX_THREADS = 8;
const int MAX_SECTIONS_PER_CLASS = 20;

vector<TimetableEntry*> listEntries;
vector<ClashInfo> clashes;
vector<string> dayList;
int threadsInUse = 1;

// Collect the linked-list timetable into a vector for indexed pair checking.
void collectEntries() {
    listEntries.clear();

    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {
        listEntries.push_back(t);
    }
}

int indexOf(const TimetableEntry* entry) {
    for (size_t i = 0; i < listEntries.size(); ++i) {
        if (listEntries[i] == entry)
            return static_cast<int>(i);
    }
    return -1;
}

// Keep days in the order in which they first appear in timetable.csv.
void buildDayList() {
    dayList.clear();

    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        if (find(dayList.begin(), dayList.end(), t->day) == dayList.end())
            dayList.push_back(t->day);
    }
}

int lastSlotNumber() {
    if (slotCount > 0)
        return slotCount;

    int last = 0;

    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        int endSlot = t->startSlot + t->duration - 1;
        if (endSlot > last)
            last = endSlot;
    }

    return last;
}

// Return the section IDs represented by an entry.
// A normal entry has one section ID; a merged-group entry has several.
vector<string> sectionsOf(const TimetableEntry* entry) {
    vector<string> result;

    MergedGroup* group = searchMergedGroupById(entry->sectionOrGroup);

    if (group == nullptr) {
        result.push_back(entry->sectionOrGroup);
        return result;
    }

    string current;
    const string& ids = group->sectionIds;

    for (size_t i = 0; i <= ids.size(); ++i) {
        if (i == ids.size() || ids[i] == ';') {
            current = trim(current);

            if (!current.empty())
                result.push_back(current);

            current.clear();
        } else {
            current += ids[i];
        }
    }

    return result;
}

bool sharesSection(const TimetableEntry* a, const TimetableEntry* b) {
    vector<string> sectionsA = sectionsOf(a);
    vector<string> sectionsB = sectionsOf(b);

    for (const string& x : sectionsA) {
        for (const string& y : sectionsB) {
            if (x == y)
                return true;
        }
    }

    return false;
}

bool overlaps(const TimetableEntry* a, const TimetableEntry* b) {
    if (a->day != b->day)
        return false;

    int endA = a->startSlot + a->duration - 1;
    int endB = b->startSlot + b->duration - 1;

    return a->startSlot <= endB && b->startSlot <= endA;
}

// Two entries with identical subject, faculty, venue, time and duration
// represent the same merged class rather than a clash between sections.
bool sameClass(const TimetableEntry* a, const TimetableEntry* b) {
    return a->subject == b->subject &&
           a->facultyId == b->facultyId &&
           a->venueId == b->venueId &&
           a->startSlot == b->startSlot &&
           a->duration == b->duration;
}

int strengthOf(const TimetableEntry* entry) {
    int total = 0;
    vector<string> ids = sectionsOf(entry);

    for (const string& id : ids) {
        Section* section = searchSectionById(id);
        if (section != nullptr)
            total += section->strength;
    }

    return total;
}

void addClash(vector<ClashInfo>& destination,
              const string& kind,
              const string& message,
              TimetableEntry* a,
              TimetableEntry* b) {
    ClashInfo clash;

    clash.day = a->day;
    clash.slot = max(a->startSlot, b->startSlot);
    clash.kind = kind;
    clash.text = message;
    clash.a = a;
    clash.b = b;

    destination.push_back(clash);
}

// Each worker owns its own result vector, so workers never write to the
// same vector. The timetable itself is only read during this function.
void checkPairsWorker(
    int threadId,
    int threadCount,
    const vector<vector<string>>& entrySections,
    vector<ClashInfo>& workerResults
) {
    const int count = static_cast<int>(listEntries.size());

    for (int i = threadId; i < count; i += threadCount) {
        for (int j = i + 1; j < count; ++j) {
            TimetableEntry* a = listEntries[i];
            TimetableEntry* b = listEntries[j];

            if (!overlaps(a, b))
                continue;

            bool sharedSection = false;

            for (const string& sectionA : entrySections[i]) {
                for (const string& sectionB : entrySections[j]) {
                    if (sectionA == sectionB) {
                        sharedSection = true;
                        break;
                    }
                }

                if (sharedSection)
                    break;
            }

            if (sharedSection) {
                addClash(
                    workerResults, "SECTION",
                    a->sectionOrGroup + " has " + a->subject +
                    " and " + b->subject + " at the same time",
                    a, b
                );
                continue;
            }

            // Same class delivered to different sections is allowed.
            if (sameClass(a, b))
                continue;

            if (a->facultyId == b->facultyId) {
                addClash(
                    workerResults, "FACULTY",
                    a->facultyId + " teaches " + a->subject +
                    " and " + b->subject + " at the same time",
                    a, b
                );
            }

            if (a->venueId == b->venueId) {
                addClash(
                    workerResults, "VENUE",
                    a->venueId + " is used by " + a->subject +
                    " and " + b->subject + " at the same time",
                    a, b
                );
            }
        }
    }
}

bool slotFreeFor(const TimetableEntry* entry,
                 const string& day,
                 int start) {
    if (start < 1 || start + entry->duration - 1 > lastSlotNumber())
        return false;

    int proposedEnd = start + entry->duration - 1;

    for (TimetableEntry* other = timetableHead;
         other != nullptr;
         other = other->next) {
        if (other == entry || other->day != day)
            continue;

        int otherEnd = other->startSlot + other->duration - 1;

        if (other->startSlot > proposedEnd || start > otherEnd)
            continue;

        if (other->facultyId == entry->facultyId ||
            other->venueId == entry->venueId ||
            sharesSection(entry, other)) {
            return false;
        }
    }

    return true;
}

bool venueFreeFor(const TimetableEntry* entry,
                  const string& venueId,
                  const string& day,
                  int start) {
    int proposedEnd = start + entry->duration - 1;

    for (TimetableEntry* other = timetableHead;
         other != nullptr;
         other = other->next) {
        if (other == entry ||
            other->venueId != venueId ||
            other->day != day) {
            continue;
        }

        int otherEnd = other->startSlot + other->duration - 1;

        if (other->startSlot <= proposedEnd && start <= otherEnd)
            return false;
    }

    return true;
}

// Try another suitable room first, then another time if allowed.
bool tryFix(TimetableEntry* entry, const string& kind) {
    bool roomProblem =
        kind == "VENUE" || kind == "CAPACITY" || kind == "MERGE-RULE";

    bool roomOnly =
        kind == "CAPACITY" || kind == "MERGE-RULE";

    if (roomProblem) {
        int requiredCapacity = strengthOf(entry);
        Venue* currentVenue = searchVenueById(entry->venueId);

        // First prefer the same venue type, then allow any suitable type.
        for (int pass = 0; pass < 2; ++pass) {
            for (Venue* venue = venueHead;
                 venue != nullptr;
                 venue = venue->next) {
                if (currentVenue != nullptr &&
                    venue->id == currentVenue->id) {
                    continue;
                }

                if (venue->capacity < requiredCapacity)
                    continue;

                if (pass == 0 &&
                    currentVenue != nullptr &&
                    venue->type != currentVenue->type) {
                    continue;
                }

                if (kind == "MERGE-RULE" &&
                    !auditoriumType.empty() &&
                    venue->type == auditoriumType) {
                    continue;
                }

                if (venueFreeFor(entry, venue->id,
                                 entry->day, entry->startSlot)) {
                    entry->venueId = venue->id;
                    return true;
                }
            }
        }
    }

    if (!roomOnly && !dayList.empty()) {
        int currentDayIndex = 0;

        for (size_t i = 0; i < dayList.size(); ++i) {
            if (dayList[i] == entry->day) {
                currentDayIndex = static_cast<int>(i);
                break;
            }
        }

        int last = lastSlotNumber();

        for (size_t offset = 0; offset < dayList.size(); ++offset) {
            const string& day =
                dayList[(currentDayIndex + static_cast<int>(offset))
                        % dayList.size()];

            for (int start = 1; start <= last; ++start) {
                if (day == entry->day && start == entry->startSlot)
                    continue;

                if (slotFreeFor(entry, day, start)) {
                    entry->day = day;
                    entry->startSlot = start;
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace

// ================= PUBLIC FUNCTIONS =================

void runClashCheck() {
    clashes.clear();
    collectEntries();

    const int count = static_cast<int>(listEntries.size());

    if (count == 0) {
        threadsInUse = 1;
        return;
    }

    vector<vector<string>> entrySections(count);

    for (int i = 0; i < count; ++i)
        entrySections[i] = sectionsOf(listEntries[i]);

    unsigned int available = thread::hardware_concurrency();

    threadsInUse = (available == 0) ? 1 : static_cast<int>(available);
    threadsInUse = min(threadsInUse, MAX_THREADS);
    threadsInUse = min(threadsInUse, count);
    threadsInUse = max(threadsInUse, 1);

    vector<vector<ClashInfo>> workerResults(threadsInUse);
    vector<thread> workers;

    for (int t = 0; t < threadsInUse; ++t) {
        workers.emplace_back(
            checkPairsWorker,
            t,
            threadsInUse,
            cref(entrySections),
            ref(workerResults[t])
        );
    }

    for (thread& worker : workers)
        worker.join();

    for (const vector<ClashInfo>& results : workerResults) {
        clashes.insert(clashes.end(), results.begin(), results.end());
    }
}

const vector<ClashInfo>& getClashes() {
    return clashes;
}

bool hasClash(const TimetableEntry* entry) {
    for (const ClashInfo& clash : clashes) {
        if (clash.a == entry || clash.b == entry)
            return true;
    }

    return false;
}

int getThreadsInUse() {
    return threadsInUse;
}

void printClashes() {
    cout << "\n===== CLASH CHECK (threads used: "
         << threadsInUse << ") =====\n";

    if (clashes.empty()) {
        cout << "No clashes found.\n";
        return;
    }

    for (const ClashInfo& clash : clashes) {
        cout << "[" << clash.kind << "] "
             << clash.day << ", slot " << clash.slot
             << ": " << clash.text << "\n";
    }

    cout << "Total clashes: " << clashes.size() << "\n";
}

// DEMO ONLY: intentionally moves entries to create clashes.
// Do not call this during normal timetable loading or final display.
void spoilTimetable() {
    collectEntries();

    if (listEntries.size() < 2) {
        cout << "Nothing to spoil.\n";
        return;
    }

    buildDayList();

    int howMany = static_cast<int>(listEntries.size()) / 10;
    howMany = max(howMany, 3);
    howMany = min(howMany, 30);

    int done = 0;

    cout << "\nSPOIL (demo): moving classes on purpose\n";

    for (int attempt = 0;
         attempt < howMany * 20 && done < howMany;
         ++attempt) {
        TimetableEntry* a =
            listEntries[rand() % listEntries.size()];

        int kind = done % 3; // 0 faculty, 1 section, 2 venue

        vector<TimetableEntry*> candidates;

        for (TimetableEntry* b : listEntries) {
            if (b == a)
                continue;

            if (b->day == a->day &&
                b->startSlot == a->startSlot) {
                continue;
            }

            if (b->startSlot + a->duration - 1 > lastSlotNumber())
                continue;

            if (sameClass(a, b))
                continue;

            bool related =
                (kind == 0 && b->facultyId == a->facultyId) ||
                (kind == 1 && sharesSection(a, b)) ||
                (kind == 2 && b->venueId == a->venueId);

            if (related)
                candidates.push_back(b);
        }

        if (candidates.empty())
            continue;

        TimetableEntry* b =
            candidates[rand() % candidates.size()];

        cout << "  " << a->subject << " (" << a->sectionOrGroup
             << ") moved from " << a->day << " slot " << a->startSlot
             << " to " << b->day << " slot " << b->startSlot << "\n";

        a->day = b->day;
        a->startSlot = b->startSlot;
        ++done;
    }

    runClashCheck();

    cout << "\nNow the timetable has " << clashes.size()
         << " clashes:\n";

    printClashes();
}

// Automatically attempts to resolve clashes by changing a room or time.
// The timetable entries are modified in place.
void optimise() {
    buildDayList();
    runClashCheck();

    if (clashes.empty()) {
        cout << "Nothing to optimise: there are no clashes.\n";
        return;
    }

    collectEntries();

    vector<string> originalDays;
    vector<string> originalVenues;
    vector<int> originalSlots;
    vector<bool> skip(listEntries.size(), false);

    for (TimetableEntry* entry : listEntries) {
        originalDays.push_back(entry->day);
        originalSlots.push_back(entry->startSlot);
        originalVenues.push_back(entry->venueId);
    }

    const int startingClashes = static_cast<int>(clashes.size());

    bool progress = true;

    while (progress && !clashes.empty()) {
        progress = false;

        // Work from a snapshot because runClashCheck rebuilds the clash list.
        vector<ClashInfo> currentClashes = clashes;

        for (const ClashInfo& clash : currentClashes) {
            TimetableEntry* entry = clash.b ? clash.b : clash.a;
            string kind = clash.kind;

            int index = indexOf(entry);

            if (index < 0 || skip[index])
                continue;

            string oldDay = entry->day;
            string oldVenue = entry->venueId;
            int oldSlot = entry->startSlot;

            int clashesBefore = static_cast<int>(clashes.size());

            if (!tryFix(entry, kind)) {
                skip[index] = true;
                continue;
            }

            runClashCheck();

            if (static_cast<int>(clashes.size()) >= clashesBefore) {
                // The move did not reduce the total number of clashes.
                entry->day = oldDay;
                entry->startSlot = oldSlot;
                entry->venueId = oldVenue;

                skip[index] = true;
                runClashCheck();
            }

            progress = true;
            break;
        }
    }

    cout << "\n===== OPTIMISATION RESULT =====\n";
    cout << "Changes made:\n";

    int moved = 0;

    for (size_t i = 0; i < listEntries.size(); ++i) {
        TimetableEntry* entry = listEntries[i];

        if (entry->day == originalDays[i] &&
            entry->startSlot == originalSlots[i] &&
            entry->venueId == originalVenues[i]) {
            continue;
        }

        ++moved;

        cout << "  " << entry->subject << " ("
             << entry->sectionOrGroup << "): "
             << originalDays[i] << " slot " << originalSlots[i]
             << " " << originalVenues[i]
             << " -> " << entry->day << " slot "
             << entry->startSlot << " " << entry->venueId << "\n";
    }

    if (moved == 0)
        cout << "  (none)\n";

    cout << "\nClashes: " << startingClashes
         << " -> " << clashes.size() << "\n";

    if (!clashes.empty()) {
        cout << "These could not be fixed automatically:\n";
        printClashes();
    } else {
        cout << "The timetable is now clash free.\n";
    }
}
