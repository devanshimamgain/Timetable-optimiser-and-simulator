// multit.cpp  (Person 3: clash detection on several threads, spoil demo and optimiser)
//
// Works on the timetable that Person 1 loads (person1_file_handling.cpp):
//   runClashCheck()   finds SECTION, FACULTY and VENUE clashes. The pairs of classes are split
//                     between several threads, which run at the same time.
//                     Two classes that are the same class for several sections are a MERGED
//                     class, not a clash.
//   spoilTimetable()  demo: moves some classes on purpose so that clashes appear
//   optimise()        fixes the clashes: first another room, then another time.
//                     A move that does not reduce the clashes is undone.
//
// Compile on Windows (MinGW):  g++ -std=c++17 -Wall -Wextra multit.cpp -o multit.exe
// Compile on Linux / macOS:    g++ -std=c++17 -Wall -Wextra -pthread multit.cpp -o multit
// Person 4 includes this file with MULTIT_NO_MAIN defined.

#define main person1_main
#include "person1_file_handling.cpp"
#undef main
#include "modules/merged_class.cpp"           // Person 2: merged classes and their room rules

#include <cstddef>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

const int MAX_CLASHES = 5000;                  // room for the problems of one run
const int MAX_CLASHES_PER_THREAD = 2000;       // room for the problems one thread finds
const int MAX_SECTIONS_PER_CLASS = 20;         // most sections that one class can have
const int MAX_TIMETABLE = 1000;                // most classes the checker and optimiser handle
const int MAX_DAY_LIST = 7;
const int MAX_THREADS = 8;                     // most threads used for the clash check

struct ClashInfo {
    string day;
    int slot;                                  // the later of the two starting slots
    string kind;                               // SECTION, FACULTY or VENUE
    string text;                               // what is wrong, in words
    TimetableEntry* a;
    TimetableEntry* b;
};

ClashInfo clashes[MAX_CLASHES];
int clashCount = 0;

// The classes in one array, so that the checker can use indexes (0 .. listCount-1).
TimetableEntry* listEntries[MAX_TIMETABLE];
int listCount = 0;
string secOf[MAX_TIMETABLE][MAX_SECTIONS_PER_CLASS];   // section ids of each class
int secCount[MAX_TIMETABLE];

// Days in the order they first appear in timetable.csv.
string dayList[MAX_DAY_LIST];
int dayListCount = 0;

// Every thread writes its own problems here, so the threads never write to the same place.
ClashInfo threadClashes[MAX_THREADS][MAX_CLASHES_PER_THREAD];
int threadClashCount[MAX_THREADS];
int threadsInUse = 1;
int threadDropped[MAX_THREADS];          // problems a thread could not store (list full)

// Number of merged classes found by Person 2's check in the last run.
int mergedGroupCount = 0;

// ---------------------------------------------------------------- basic helpers
void collectEntries() {
    listCount = 0;
    int total = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        total++;
        if (listCount < MAX_TIMETABLE) listEntries[listCount++] = t;
    }
    if (total > MAX_TIMETABLE)
        cout << "WARNING: " << total << " classes found, only the first " << MAX_TIMETABLE
             << " are checked. Increase MAX_TIMETABLE." << endl;
}

int indexOf(const TimetableEntry* t) {
    for (int i = 0; i < listCount; i++) if (listEntries[i] == t) return i;
    return -1;
}

void buildDayList() {
    dayListCount = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        bool known = false;
        for (int i = 0; i < dayListCount; i++) if (dayList[i] == t->day) known = true;
        if (!known && dayListCount < MAX_DAY_LIST) dayList[dayListCount++] = t->day;
    }
}

// The last slot of a day: from slots.csv if it exists, otherwise from the timetable itself.
int lastSlotNumber() {
    if (slotCount > 0) return slotCount;
    int last = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next)
        if (t->startSlot + t->duration - 1 > last) last = t->startSlot + t->duration - 1;
    return last;
}

// Fills out[] with the section ids of this class: its own id, or every member of its merged group.
int sectionsOf(const TimetableEntry* t, string out[], int maxCount) {
    MergedGroup* group = searchMergedGroupById(t->sectionOrGroup);
    if (group == nullptr) {
        if (maxCount > 0) out[0] = t->sectionOrGroup;
        return 1;
    }
    int n = 0;
    string current;
    const string& ids = group->sectionIds;
    for (size_t i = 0; i <= ids.size(); i++) {
        if (i == ids.size() || ids[i] == ';') {
            current = trim(current);
            if (!current.empty() && n < maxCount) out[n++] = current;
            current.clear();
        } else {
            current += ids[i];
        }
    }
    return n;
}

// Does this class attend section id? (also true for a merged group that contains the section)
bool attends(const TimetableEntry* t, const string& id) {
    string ids[MAX_SECTIONS_PER_CLASS];
    int n = sectionsOf(t, ids, MAX_SECTIONS_PER_CLASS);
    for (int i = 0; i < n; i++) if (ids[i] == id) return true;
    return false;
}

// Do the two classes have a section in common?
bool sharesSection(const TimetableEntry* a, const TimetableEntry* b) {
    string sa[MAX_SECTIONS_PER_CLASS], sb[MAX_SECTIONS_PER_CLASS];
    int na = sectionsOf(a, sa, MAX_SECTIONS_PER_CLASS);
    int nb = sectionsOf(b, sb, MAX_SECTIONS_PER_CLASS);
    for (int i = 0; i < na; i++)
        for (int j = 0; j < nb; j++)
            if (sa[i] == sb[j]) return true;
    return false;
}

// Do the two classes overlap in time? They must be on the same day and share at least one slot.
bool overlaps(const TimetableEntry* a, const TimetableEntry* b) {
    if (a->day != b->day) return false;
    return a->startSlot <= b->startSlot + b->duration - 1 && b->startSlot <= a->startSlot + a->duration - 1;
}

// The same class for different sections: same subject, teacher, room, start and length.
bool sameClass(const TimetableEntry* a, const TimetableEntry* b) {
    return a->subject == b->subject &&
           a->facultyId == b->facultyId &&
           a->venueId == b->venueId &&
           a->startSlot == b->startSlot &&
           a->duration == b->duration;
}

// Total students of a class (all its sections together).
int strengthOf(const TimetableEntry* t) {
    string ids[MAX_SECTIONS_PER_CLASS];
    int n = sectionsOf(t, ids, MAX_SECTIONS_PER_CLASS);
    int total = 0;
    for (int i = 0; i < n; i++) {
        Section* s = searchSectionById(ids[i]);
        if (s) total += s->strength;
    }
    return total;
}

// ---------------------------------------------------------------- threads
typedef void (*WorkFunction)(int threadId);

struct ThreadJob {
    WorkFunction work;
    int id;
};

#ifdef _WIN32
static DWORD WINAPI threadEntry(LPVOID arg) {
    ThreadJob* job = (ThreadJob*)arg;
    job->work(job->id);
    return 0;
}

static int processorCount() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int)info.dwNumberOfProcessors;
}

// Runs work(0), work(1), ... on `count` threads, and waits until all of them are finished.
static void runOnThreads(int count, WorkFunction work) {
    HANDLE handles[MAX_THREADS];
    ThreadJob jobs[MAX_THREADS];
    bool started[MAX_THREADS];
    for (int i = 0; i < count; i++) {
        jobs[i].work = work;
        jobs[i].id = i;
        handles[i] = CreateThread(NULL, 0, threadEntry, &jobs[i], 0, NULL);
        started[i] = (handles[i] != NULL);
        if (!started[i]) work(i);                  // could not start a thread: do its work here
    }
    for (int i = 0; i < count; i++)
        if (started[i]) { WaitForSingleObject(handles[i], INFINITE); CloseHandle(handles[i]); }
}
#else
static void* threadEntry(void* arg) {
    ThreadJob* job = (ThreadJob*)arg;
    job->work(job->id);
    return NULL;
}

static int processorCount() {
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

// Runs work(0), work(1), ... on `count` threads, and waits until all of them are finished.
static void runOnThreads(int count, WorkFunction work) {
    pthread_t threads[MAX_THREADS];
    ThreadJob jobs[MAX_THREADS];
    bool started[MAX_THREADS];
    for (int i = 0; i < count; i++) {
        jobs[i].work = work;
        jobs[i].id = i;
        started[i] = (pthread_create(&threads[i], NULL, threadEntry, &jobs[i]) == 0);
        if (!started[i]) work(i);                  // could not start a thread: do its work here
    }
    for (int i = 0; i < count; i++)
        if (started[i]) pthread_join(threads[i], NULL);
}
#endif

// ---------------------------------------------------------------- clash check
// Stores one problem found by thread t.
void addThreadClash(int t, const string& kind, const string& text, TimetableEntry* a, TimetableEntry* b) {
    if (threadClashCount[t] >= MAX_CLASHES_PER_THREAD) {
        threadDropped[t]++;
        return;
    }
    ClashInfo& c = threadClashes[t][threadClashCount[t]++];
    c.day = a->day;
    c.slot = (b->startSlot > a->startSlot) ? b->startSlot : a->startSlot;
    c.kind = kind;
    c.text = text;
    c.a = a;
    c.b = b;
}

// Thread t checks the pairs (i, j) where i = t, t + threadsInUse, t + 2*threadsInUse, ...
// It only reads the shared data, and writes into its own part of threadClashes.
void checkPairsWorker(int t) {
    threadClashCount[t] = 0;
    threadDropped[t] = 0;
    for (int i = t; i < listCount; i += threadsInUse) {
        for (int j = i + 1; j < listCount; j++) {
            TimetableEntry* a = listEntries[i];
            TimetableEntry* b = listEntries[j];
            if (!overlaps(a, b)) continue;

            bool shared = false;
            for (int x = 0; x < secCount[i] && !shared; x++)
                for (int y = 0; y < secCount[j] && !shared; y++)
                    if (secOf[i][x] == secOf[j][y]) shared = true;

            if (shared) {
                addThreadClash(t, "SECTION", a->sectionOrGroup + " has " + a->subject + " and " +
                               b->subject + " at the same time", a, b);
                continue;
            }
            if (sameClass(a, b)) continue;         // merged class: fine

            if (a->facultyId == b->facultyId)
                addThreadClash(t, "FACULTY", a->facultyId + " teaches " + a->subject + " and " +
                               b->subject + " at the same time", a, b);
            if (a->venueId == b->venueId)
                addThreadClash(t, "VENUE", a->venueId + " is used by " + a->subject + " and " +
                               b->subject + " at the same time", a, b);
        }
    }
}

// Checks every pair of classes. The work is shared between the threads, then the
// results of all threads are put together into clashes[].
// Adds one problem to the main clash list (used for the merged-class problems).
void addMainClash(const string& kind, const string& text, TimetableEntry* a) {
    if (clashCount >= MAX_CLASHES) return;
    ClashInfo& c = clashes[clashCount++];
    c.day = a->day;
    c.slot = a->startSlot;
    c.kind = kind;
    c.text = text;
    c.a = a;
    c.b = nullptr;
}

// Turns the reason of an invalid merged group into a clash kind.
string mergeProblemKind(const string& reason) {
    if (reason.find("capacity") != string::npos) return "CAPACITY";
    if (reason.find("auditorium") != string::npos) return "MERGE-RULE";
    return "MERGED";
}

void runClashCheck() {
    clashCount = 0;
    collectEntries();
    for (int i = 0; i < listCount; i++)
        secCount[i] = sectionsOf(listEntries[i], secOf[i], MAX_SECTIONS_PER_CLASS);

    threadsInUse = processorCount();
    if (threadsInUse > MAX_THREADS) threadsInUse = MAX_THREADS;
    if (threadsInUse > listCount) threadsInUse = listCount;
    if (threadsInUse < 1) threadsInUse = 1;

    runOnThreads(threadsInUse, checkPairsWorker);

    int dropped = 0;
    for (int t = 0; t < threadsInUse; t++) {
        dropped += threadDropped[t];
        for (int k = 0; k < threadClashCount[t] && clashCount < MAX_CLASHES; k++)
            clashes[clashCount++] = threadClashes[t][k];
        if (threadClashCount[t] > 0 && clashCount >= MAX_CLASHES) dropped++;
    }
    if (dropped > 0)
        cout << "WARNING: " << dropped << " clash(es) were not stored (list full). Increase the limits." << endl;

    // Person 2: merged classes, slot by slot. Invalid groups become clashes.
    if (dayListCount == 0) buildDayList();
    mergedGroupCount = 0;
    int last = lastSlotNumber();
    MergeProblem problems[200];
    for (int d = 0; d < dayListCount; d++)
        for (int s = 1; s <= last; s++) {
            int badGroups = 0;
            mergedGroupCount += checkMergedClassesInSlot(dayList[d], s, problems, 200, badGroups);
            for (int k = 0; k < badGroups; k++)
                addMainClash(mergeProblemKind(problems[k].reason), problems[k].reason, problems[k].row);
        }
}

// True when this class is part of at least one clash.
bool hasClash(const TimetableEntry* t) {
    for (int i = 0; i < clashCount; i++)
        if (clashes[i].a == t || clashes[i].b == t) return true;
    return false;
}

void printClashes() {
    cout << "\n===== CLASH CHECK (threads used: " << threadsInUse << ") =====\n";
    cout << "Merged classes found: " << mergedGroupCount << "\n";
    if (clashCount == 0) {
        cout << "No clashes found.\n";
        return;
    }
    for (int i = 0; i < clashCount; i++)
        cout << "[" << clashes[i].kind << "] " << clashes[i].day << ", slot " << clashes[i].slot
             << ": " << clashes[i].text << "\n";
    cout << "Total clashes: " << clashCount << "\n";
}

// ---------------------------------------------------------------- spoil (demo)
void spoilTimetable() {
    collectEntries();
    if (listCount < 2) { cout << "Nothing to spoil.\n"; return; }
    int howMany = listCount / 10;                  // about 10% of the classes
    if (howMany < 3) howMany = 3;
    if (howMany > 30) howMany = 30;
    int done = 0;
    cout << "\nSPOIL (demo): moving classes on purpose\n";
    for (int attempt = 0; attempt < howMany * 20 && done < howMany; attempt++) {
        TimetableEntry* a = listEntries[rand() % listCount];
        int kind = done % 3;                       // 0 = same teacher, 1 = same section, 2 = same room
        TimetableEntry* cand[MAX_TIMETABLE];
        int nc = 0;
        for (int i = 0; i < listCount; i++) {
            TimetableEntry* b = listEntries[i];
            if (b == a || (b->day == a->day && b->startSlot == a->startSlot)) continue;
            if (b->startSlot + a->duration - 1 > lastSlotNumber()) continue;
            if (sameClass(a, b)) continue;         // would be a merged class, not a clash
            bool related = (kind == 0 && b->facultyId == a->facultyId) ||
                           (kind == 1 && sharesSection(a, b)) ||
                           (kind == 2 && b->venueId == a->venueId);
            if (related) cand[nc++] = b;
        }
        if (nc == 0) continue;
        TimetableEntry* b = cand[rand() % nc];
        cout << "  " << a->subject << " (" << a->sectionOrGroup << ") moved from " << a->day
             << " slot " << a->startSlot << " to " << b->day << " slot " << b->startSlot << "\n";
        a->day = b->day;
        a->startSlot = b->startSlot;
        done++;
    }
    runClashCheck();
    cout << "\nNow the timetable has " << clashCount << " clashes:\n";
    printClashes();
}

// ---------------------------------------------------------------- optimise
// Can class e be placed on (day, start) without a new clash? Its own venue and sections must be free.
bool slotFreeFor(const TimetableEntry* e, const string& day, int start) {
    if (start < 1 || start + e->duration - 1 > lastSlotNumber()) return false;
    for (TimetableEntry* o = timetableHead; o != nullptr; o = o->next) {
        if (o == e || o->day != day) continue;
        if (o->startSlot > start + e->duration - 1 || start > o->startSlot + o->duration - 1) continue;
        if (o->facultyId == e->facultyId || o->venueId == e->venueId || sharesSection(e, o)) return false;
    }
    return true;
}

// Is room venueId empty for the time of class e (on day, start)?
bool venueFreeFor(const TimetableEntry* e, const string& venueId, const string& day, int start) {
    for (TimetableEntry* o = timetableHead; o != nullptr; o = o->next) {
        if (o == e || o->venueId != venueId || o->day != day) continue;
        if (o->startSlot > start + e->duration - 1 || start > o->startSlot + o->duration - 1) continue;
        return false;
    }
    return true;
}

// Tries to fix one problem of class e. First another room, then another time.
// Capacity and auditorium problems can only be fixed with another room.
bool tryFix(TimetableEntry* e, const string& kind) {
    bool roomProblem = kind == "VENUE" || kind == "CAPACITY" || kind == "MERGE-RULE";
    bool roomOnly = kind == "CAPACITY" || kind == "MERGE-RULE";
    if (roomProblem) {
        int need = strengthOf(e);
        Venue* cur = searchVenueById(e->venueId);
        for (int pass = 0; pass < 2; pass++)                 // pass 0: same type of room, pass 1: any room
            for (Venue* v = venueHead; v != nullptr; v = v->next) {
                if (cur && v->id == cur->id) continue;
                if (v->capacity < need) continue;
                if (pass == 0 && cur && v->type != cur->type) continue;
                if (kind == "MERGE-RULE" && !auditoriumType.empty() && v->type == auditoriumType) continue;
                if (venueFreeFor(e, v->id, e->day, e->startSlot)) { e->venueId = v->id; return true; }
            }
    }
    if (!roomOnly) {
        int base = 0;
        for (int i = 0; i < dayListCount; i++) if (dayList[i] == e->day) base = i;
        int last = lastSlotNumber();
        for (int dd = 0; dd < dayListCount; dd++) {
            string d = dayList[(base + dd) % dayListCount];
            for (int s = 1; s <= last; s++) {
                if (d == e->day && s == e->startSlot) continue;
                if (slotFreeFor(e, d, s)) { e->day = d; e->startSlot = s; return true; }
            }
        }
    }
    return false;
}

void optimise() {
    runClashCheck();
    if (clashCount == 0) { cout << "Nothing to optimise: there are no clashes.\n"; return; }

    // remember the positions before the change
    string origDay[MAX_TIMETABLE], origVenue[MAX_TIMETABLE];
    int origSlot[MAX_TIMETABLE];
    bool skip[MAX_TIMETABLE];
    for (int i = 0; i < listCount; i++) {
        origDay[i] = listEntries[i]->day;
        origSlot[i] = listEntries[i]->startSlot;
        origVenue[i] = listEntries[i]->venueId;
        skip[i] = false;
    }
    int startClashes = clashCount;

    bool progress = true;
    while (progress && clashCount > 0) {
        progress = false;
        for (int c = 0; c < clashCount; c++) {
            TimetableEntry* e = clashes[c].b ? clashes[c].b : clashes[c].a;   // move the second class
            string kind = clashes[c].kind;
            int idx = indexOf(e);
            if (idx < 0 || skip[idx]) continue;

            string oldDay = e->day, oldVenue = e->venueId;
            int oldSlot = e->startSlot;
            int before = clashCount;
            if (!tryFix(e, kind)) { skip[idx] = true; continue; }

            runClashCheck();                             // clashes are rebuilt: leave this loop
            if (clashCount >= before) {                  // the move did not help: undo it
                e->day = oldDay; e->startSlot = oldSlot; e->venueId = oldVenue;
                skip[idx] = true;
                runClashCheck();
            }
            progress = true;
            break;
        }
    }

    cout << "\n===== OPTIMISATION RESULT =====\nChanges made:\n";
    int moved = 0;
    for (int i = 0; i < listCount; i++) {
        TimetableEntry* e = listEntries[i];
        if (e->day == origDay[i] && e->startSlot == origSlot[i] && e->venueId == origVenue[i]) continue;
        moved++;
        cout << "  " << e->subject << " (" << e->sectionOrGroup << "): " << origDay[i] << " slot " << origSlot[i]
             << " " << origVenue[i] << "  ->  " << e->day << " slot " << e->startSlot << " " << e->venueId << "\n";
    }
    if (moved == 0) cout << "  (none)\n";
    cout << "\nClashes: " << startClashes << "  ->  " << clashCount << "\n";
    if (clashCount > 0) {
        cout << "These could not be fixed automatically:\n";
        printClashes();
    } else {
        cout << "The timetable is now clash free.\n";
    }
}

#ifndef MULTIT_NO_MAIN
int main() {
    loadAllCSVFiles();
    buildDayList();
    runClashCheck();
    printClashes();
    freeAllMemory();
    return 0;
}
#endif
