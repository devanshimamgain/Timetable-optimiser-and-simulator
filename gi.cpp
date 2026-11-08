// gi.cpp  (Person 3: clash detection)
//
// Checks the timetable that Person 1 loads, and finds three kinds of clash:
//   SECTION : one section has two classes at the same time
//   FACULTY : one teacher has two classes at the same time
//   VENUE   : one room has two classes at the same time
//
// Two classes that are the same class for several sections (same subject, teacher, room,
// start and length) are a MERGED class, not a clash.
//
// Compile: g++ -std=c++17 -Wall -Wextra gi.cpp -o clash_check
// Run:     ./clash_check        (run it in the folder that has the CSV files)
//
// Person 4 includes this file with GI_NO_MAIN defined, and uses runClashCheck() and hasClash().

#define main person1_main
#include "Person1_CSVHandler_FIXED.cpp"
#undef main

const int MAX_CLASHES = 5000;            // room for the problems found in one run
const int MAX_SECTIONS_PER_CLASS = 20;   // most sections that one class can have

struct ClashInfo {
    string day;
    int slot;                            // the later of the two starting slots
    string kind;                         // SECTION, FACULTY or VENUE
    string text;                         // what is wrong, in words
    TimetableEntry* a;
    TimetableEntry* b;
};

ClashInfo clashes[MAX_CLASHES];
int clashCount = 0;

// Fills out[] with the section ids of this class: its own id, or every member of its merged group.
// Returns how many ids were written.
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

void addClash(const string& kind, const string& text, TimetableEntry* a, TimetableEntry* b) {
    if (clashCount >= MAX_CLASHES) return;
    ClashInfo& c = clashes[clashCount++];
    c.day = a->day;
    c.slot = (b->startSlot > a->startSlot) ? b->startSlot : a->startSlot;
    c.kind = kind;
    c.text = text;
    c.a = a;
    c.b = b;
}

// Checks every pair of classes once.
void runClashCheck() {
    clashCount = 0;
    for (TimetableEntry* a = timetableHead; a != nullptr; a = a->next) {
        for (TimetableEntry* b = a->next; b != nullptr; b = b->next) {
            if (!overlaps(a, b)) continue;

            if (sharesSection(a, b)) {
                addClash("SECTION", a->sectionOrGroup + " has " + a->subject + " and " + b->subject +
                         " at the same time", a, b);
                continue;
            }
            if (sameClass(a, b)) continue;       // merged class: fine

            if (a->facultyId == b->facultyId)
                addClash("FACULTY", a->facultyId + " teaches " + a->subject + " and " + b->subject +
                         " at the same time", a, b);
            if (a->venueId == b->venueId)
                addClash("VENUE", a->venueId + " is used by " + a->subject + " and " + b->subject +
                         " at the same time", a, b);
        }
    }
}

// True when this class is part of at least one clash.
bool hasClash(const TimetableEntry* t) {
    for (int i = 0; i < clashCount; i++)
        if (clashes[i].a == t || clashes[i].b == t) return true;
    return false;
}

void printClashes() {
    cout << "\n===== CLASH CHECK =====\n";
    if (clashCount == 0) {
        cout << "No clashes found.\n";
        return;
    }
    for (int i = 0; i < clashCount; i++)
        cout << "[" << clashes[i].kind << "] " << clashes[i].day << ", slot " << clashes[i].slot
             << ": " << clashes[i].text << "\n";
    cout << "Total clashes: " << clashCount << "\n";
}

#ifndef GI_NO_MAIN
int main() {
    loadAllCSVFiles();
    runClashCheck();
    printClashes();
    freeAllMemory();
    return 0;
}
#endif
