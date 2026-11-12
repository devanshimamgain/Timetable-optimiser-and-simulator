// run_tests.cpp
// Tests for Person 1 (loading and validation) and Person 3 (clash check and optimiser).
// Every test writes its own small CSV files into the folder tests_tmp/, so the tests do not
// depend on the real data.
//
// Build and run (from the repo root):
//   g++ -std=c++17 -Wall -Wextra tests/run_tests.cpp -o run_tests.exe
//   ./run_tests.exe

#define MULTIT_NO_MAIN
#include "../multit.cpp"

#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(p) _mkdir(p)
#define CHANGE_DIR(p) _chdir(p)
#define CURRENT_DIR(b, n) _getcwd(b, n)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MAKE_DIR(p) mkdir(p, 0755)
#define CHANGE_DIR(p) chdir(p)
#define CURRENT_DIR(b, n) getcwd(b, n)
#endif

const char* TEST_DIR = "tests_tmp";
const char* TIMETABLE_HEADER = "day,start_slot,duration,section_or_group,subject,faculty_id,venue_id\n";
char startDir[1024];
int passes = 0, failures = 0;

#define CHECK(cond) do { \
    if (cond) passes++; \
    else { failures++; printf("  FAILED (line %d): %s\n", __LINE__, #cond); } \
} while (0)

// ---------------------------------------------------------------- folder and files
void enterDir() {
    CURRENT_DIR(startDir, sizeof startDir);
    MAKE_DIR(TEST_DIR);
    CHANGE_DIR(TEST_DIR);
}
void leaveDir() { CHANGE_DIR(startDir); }

void writeFile(const char* name, const std::string& text) {
    FILE* f = fopen(name, "w");
    if (f) { fputs(text.c_str(), f); fclose(f); }
}

// Writes a valid faculty, sections and venues file, and the timetable rows given (without header).
void setupBase(const std::string& rows) {
    remove("slots.csv");
    remove("config.txt");
    remove("merged_groups.csv");
    writeFile("faculty.csv", "faculty_id,name\nF1,Dr A\nF2,Dr B\n");
    writeFile("sections.csv", "section_id,name,strength\nS1,Sec 1,50\nS2,Sec 2,50\n");
    writeFile("venues.csv", "venue_id,name,type,capacity\nR1,Room 1,classroom,60\nR2,Room 2,classroom,60\n");
    writeFile("timetable.csv", TIMETABLE_HEADER + rows);
}

// ---------------------------------------------------------------- helpers
// Clears every list and setting, so that one test does not affect the next one.
void resetState() {
    freeAllMemory();
    slotCount = 0;
    maxAudSections = 0;
    auditoriumType = "";
    dayListCount = 0;
    clashCount = 0;
    listCount = 0;
}

void loadAndCheck() {
    loadAllCSVFiles();
    buildDayList();
    runClashCheck();
}

int countFaculty() { int n = 0; for (Faculty* p = facultyHead; p; p = p->next) n++; return n; }
int countSections() { int n = 0; for (Section* p = sectionHead; p; p = p->next) n++; return n; }
int countVenues() { int n = 0; for (Venue* p = venueHead; p; p = p->next) n++; return n; }
int countEntries() { int n = 0; for (TimetableEntry* p = timetableHead; p; p = p->next) n++; return n; }
int countKind(const char* kind) {
    int n = 0;
    for (int i = 0; i < clashCount; i++) if (clashes[i].kind == kind) n++;
    return n;
}

// ---------------------------------------------------------------- Person 1: loading
void testLoaderRejectsBadRows() {
    printf("loader: bad rows are rejected\n");
    enterDir();
    resetState();
    remove("slots.csv");
    remove("config.txt");
    remove("merged_groups.csv");
    writeFile("faculty.csv", "faculty_id,name\nF1,Dr A\nF1,Duplicate\nF2,Dr B\n");        // F1 twice
    writeFile("sections.csv", "section_id,name,strength\nS1,Sec 1,50\nS1,Duplicate,20\n"); // S1 twice
    writeFile("venues.csv", "venue_id,name,type,capacity\nR1,Room 1,classroom,60\nR2,Room 2,classroom,-5\n");
    writeFile("timetable.csv", std::string(TIMETABLE_HEADER) +
              "Monday,1,1,S1,MATH,F1,R1\n" +      // good
              "Monday,1,1,S1,PHY,F9,R1\n" +       // unknown teacher
              "Monday,x,1,S1,CHEM,F1,R1\n" +      // slot is not a number
              "Monday,1,1,S9,BIO,F1,R1\n");       // unknown section
    loadAllCSVFiles();
    CHECK(countFaculty() == 2);
    CHECK(countSections() == 1);
    CHECK(countVenues() == 1);
    CHECK(countEntries() == 1);
    leaveDir();
}

void testSlotLimit() {
    printf("loader: slots.csv limits the slot numbers\n");
    enterDir();
    resetState();
    setupBase("");
    writeFile("slots.csv", "slot,start,end\n1,08:00,08:55\n2,08:55,09:50\n");
    writeFile("timetable.csv", std::string(TIMETABLE_HEADER) +
              "Monday,2,1,S1,B,F2,R2\n" +         // ok: slot 2 of 2
              "Monday,3,1,S1,C,F1,R1\n" +         // slot 3 does not exist
              "Monday,2,2,S1,D,F1,R1\n");         // runs from slot 2 to 3: goes past the last slot
    loadAllCSVFiles();
    CHECK(slotCount == 2);
    CHECK(countEntries() == 1);
    leaveDir();
}

void testConfigIsRead() {
    printf("loader: config.txt sets the auditorium rule\n");
    enterDir();
    resetState();
    setupBase("Monday,1,1,S1,MATH,F1,R1\n");
    writeFile("config.txt", "# comment\nmax_sections_in_auditorium=6\nauditorium_type=auditorium\n");
    loadAllCSVFiles();
    CHECK(maxAudSections == 6);
    CHECK(auditoriumType == "auditorium");
    leaveDir();
}

// ---------------------------------------------------------------- Person 3: clash check
struct ClashCase {
    const char* name;
    std::string rows;
    const char* kind;        // SECTION, FACULTY or VENUE; NONE = no clash at all
    int expect;
};

void testClashKinds() {
    printf("clash check: each kind of clash\n");
    ClashCase cases[] = {
        {"section clash", "Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S1,PHY,F2,R2\n", "SECTION", 1},
        {"teacher clash", "Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S2,PHY,F1,R2\n", "FACULTY", 1},
        {"room clash", "Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S2,PHY,F2,R1\n", "VENUE", 1},
        {"overlap by length", "Monday,1,2,S1,MATH,F1,R1\nMonday,2,1,S1,PHY,F2,R2\n", "SECTION", 1},
        {"same room, other slot", "Monday,1,1,S1,MATH,F1,R1\nMonday,2,1,S1,PHY,F2,R1\n", "NONE", 0},
        {"merged class is not a clash", "Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S2,MATH,F1,R1\n", "MERGED", 1},
    };
    for (const ClashCase& c : cases) {
        printf("  %s\n", c.name);
        enterDir();
        resetState();
        setupBase(c.rows);
        loadAndCheck();
        if (std::string(c.kind) == "NONE") {
            CHECK(clashCount == 0);
        } else if (std::string(c.kind) == "MERGED") {
            // the two rows are one merged class: no section, teacher or room clash,
            // and it is found as one group; 50 + 50 students do not fit in R1 (60 seats)
            CHECK(countKind("SECTION") + countKind("FACULTY") + countKind("VENUE") == 0);
            CHECK(mergedGroupCount == 1);
            CHECK(countKind("CAPACITY") == 1);
        } else {
            CHECK(countKind(c.kind) == c.expect);
        }
        leaveDir();
    }
}

// ---------------------------------------------------------------- Person 3: optimiser
void testOptimise() {
    printf("optimise: clashes are fixed\n");

    printf("  room clash: one class moves to the other room\n");
    enterDir();
    resetState();
    setupBase("Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S2,PHY,F2,R1\n");
    writeFile("slots.csv", "slot,start,end\n1,08:00,08:55\n2,08:55,09:50\n");
    loadAndCheck();
    CHECK(clashCount == 1);
    optimise();
    CHECK(clashCount == 0);
    leaveDir();

    printf("  section clash: one class moves to a free slot\n");
    enterDir();
    resetState();
    setupBase("Monday,1,1,S1,MATH,F1,R1\nMonday,1,1,S1,PHY,F2,R2\n");
    writeFile("slots.csv", "slot,start,end\n1,08:00,08:55\n2,08:55,09:50\n");
    loadAndCheck();
    CHECK(clashCount == 1);
    optimise();
    CHECK(clashCount == 0);
    leaveDir();
}

int main() {
    testLoaderRejectsBadRows();
    testSlotLimit();
    testConfigIsRead();
    testClashKinds();
    testOptimise();
    printf("\n%d checks passed, %d failed\n", passes, failures);
    return failures == 0 ? 0 : 1;
}
