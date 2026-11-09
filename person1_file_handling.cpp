// person1_file_handling.cpp  (Person 1: loading and checking the data)
//
// What this file does:
//   1. Reads the CSV files in the data folder.
//   2. Checks every row. A bad row is skipped, and the reason is printed.
//   3. Keeps the good rows in linked lists, one list for each kind of record.
//   4. Reads config.txt (auditorium rule) and slots.csv (number of slots in a day).
//
// The other modules use these names, so please keep them:
//   loadAllCSVFiles(), freeAllMemory(), the list heads (facultyHead, sectionHead, ...),
//   slotCount, maxAudSections, auditoriumType, and the search functions.
//
// Check it on its own:
//   g++ -std=c++17 -Wall -Wextra person1_file_handling.cpp -o person1.exe
//   (run it in the folder that has the CSV files)

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// ==================================================================================
// RECORDS  (every record is a node in a linked list; next points to the next node)
// ==================================================================================
struct Faculty {
    string id;
    string name;
    Faculty* next;
};

struct Section {
    string id;
    string name;
    int strength;
    Section* next;
};

struct Venue {
    string id;
    string name;
    string type;
    int capacity;
    Venue* next;
};

struct TimetableEntry {
    string day;
    int startSlot;
    int duration;
    string sectionOrGroup;
    string subject;
    string facultyId;
    string venueId;
    TimetableEntry* next;
};

struct MergedGroup {
    string id;
    string sectionIds;          // section ids separated by ';', e.g. "A;B;C"
    MergedGroup* next;
};

// ==================================================================================
// GLOBAL DATA
// ==================================================================================
Faculty* facultyHead = nullptr;
Faculty* facultyTail = nullptr;

Section* sectionHead = nullptr;
Section* sectionTail = nullptr;

Venue* venueHead = nullptr;
Venue* venueTail = nullptr;

TimetableEntry* timetableHead = nullptr;
TimetableEntry* timetableTail = nullptr;

MergedGroup* mergedGroupHead = nullptr;
MergedGroup* mergedGroupTail = nullptr;

// Number of slots in one day, from slots.csv. 0 = no slots file, so slot numbers are not limited.
int slotCount = 0;

// Auditorium rule, from config.txt. 0 or empty = no rule.
int maxAudSections = 0;
string auditoriumType = "";

// ==================================================================================
// SMALL HELPERS
// ==================================================================================

// Removes spaces and tabs from the start and the end of a text.
string trim(const string& s) {
    size_t start = 0;
    size_t end = s.size();
    while (start < end && isspace(static_cast<unsigned char>(s[start]))) start++;
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

bool isBlank(const string& s) {
    return trim(s).empty();
}

// Is the text a whole number above 0? If yes, the number is stored in out.
bool validatePositiveInt(const string& s, int& out) {
    string text = trim(s);
    if (text.empty()) return false;

    stringstream ss(text);
    int value;
    char extra;
    if (!(ss >> value) || (ss >> extra) || value <= 0) return false;

    out = value;
    return true;
}

// Splits one CSV line at the commas. It must give exactly `expected` fields.
bool splitCSV(const string& line, string out[], int expected) {
    int count = 0;
    size_t start = 0;
    while (true) {
        if (count >= expected) return false;

        size_t comma = line.find(',', start);
        if (comma == string::npos) {
            out[count++] = trim(line.substr(start));
            break;
        }
        out[count++] = trim(line.substr(start, comma - start));
        start = comma + 1;
    }
    return count == expected;
}

// ==================================================================================
// SEARCH  (walk the list from the start until the id is found)
// ==================================================================================
Faculty* searchFacultyById(const string& id) {
    for (Faculty* p = facultyHead; p != nullptr; p = p->next)
        if (p->id == id) return p;
    return nullptr;
}

Section* searchSectionById(const string& id) {
    for (Section* p = sectionHead; p != nullptr; p = p->next)
        if (p->id == id) return p;
    return nullptr;
}

Venue* searchVenueById(const string& id) {
    for (Venue* p = venueHead; p != nullptr; p = p->next)
        if (p->id == id) return p;
    return nullptr;
}

MergedGroup* searchMergedGroupById(const string& id) {
    for (MergedGroup* p = mergedGroupHead; p != nullptr; p = p->next)
        if (p->id == id) return p;
    return nullptr;
}

// Every section in "A;B;C" must exist, and there must be at least one.
bool validateSectionList(const string& sectionIds) {
    stringstream ss(sectionIds);
    string id;
    bool foundAny = false;
    while (getline(ss, id, ';')) {
        id = trim(id);
        if (id.empty()) continue;
        if (searchSectionById(id) == nullptr) return false;
        foundAny = true;
    }
    return foundAny;
}

// ==================================================================================
// ADD TO THE END OF A LIST  (the tail pointer makes this fast)
// ==================================================================================
void addFaculty(const string& id, const string& name) {
    Faculty* node = new Faculty{ id, name, nullptr };
    if (facultyTail == nullptr) facultyHead = node;
    else facultyTail->next = node;
    facultyTail = node;
}

void addSection(const string& id, const string& name, int strength) {
    Section* node = new Section{ id, name, strength, nullptr };
    if (sectionTail == nullptr) sectionHead = node;
    else sectionTail->next = node;
    sectionTail = node;
}

void addVenue(const string& id, const string& name, const string& type, int capacity) {
    Venue* node = new Venue{ id, name, type, capacity, nullptr };
    if (venueTail == nullptr) venueHead = node;
    else venueTail->next = node;
    venueTail = node;
}

void addTimetableEntry(const string& day, int slot, int duration, const string& sectionOrGroup,
                       const string& subject, const string& facultyId, const string& venueId) {
    TimetableEntry* node = new TimetableEntry{
        day, slot, duration, sectionOrGroup, subject, facultyId, venueId, nullptr
    };
    if (timetableTail == nullptr) timetableHead = node;
    else timetableTail->next = node;
    timetableTail = node;
}

void addMergedGroup(const string& id, const string& sectionIds) {
    MergedGroup* node = new MergedGroup{ id, sectionIds, nullptr };
    if (mergedGroupTail == nullptr) mergedGroupHead = node;
    else mergedGroupTail->next = node;
    mergedGroupTail = node;
}

// ==================================================================================
// ROW CHECKS
// Each function checks one row (its fields are in f[0], f[1], ...).
// It returns "" when the row is good, or the reason why the row is bad.
// ==================================================================================
string checkFacultyRow(const string* f) {
    if (f[0].empty() || f[1].empty()) return "empty faculty ID or name";
    if (searchFacultyById(f[0]) != nullptr) return "duplicate faculty ID: " + f[0];
    addFaculty(f[0], f[1]);
    return "";
}

string checkSectionRow(const string* f) {
    int strength;
    if (f[0].empty() || f[1].empty()) return "empty section ID or name";
    if (!validatePositiveInt(f[2], strength)) return "invalid strength: " + f[2];
    if (searchSectionById(f[0]) != nullptr) return "duplicate section ID: " + f[0];
    addSection(f[0], f[1], strength);
    return "";
}

string checkVenueRow(const string* f) {
    int capacity;
    if (f[0].empty() || f[1].empty() || f[2].empty()) return "empty venue ID, name, or type";
    if (!validatePositiveInt(f[3], capacity)) return "invalid capacity: " + f[3];
    if (searchVenueById(f[0]) != nullptr) return "duplicate venue ID: " + f[0];
    addVenue(f[0], f[1], f[2], capacity);
    return "";
}

string checkMergedGroupRow(const string* f) {
    if (f[0].empty()) return "empty group ID";
    if (!validateSectionList(f[1])) return "invalid section list: " + f[1];
    if (searchMergedGroupById(f[0]) != nullptr) return "duplicate group ID: " + f[0];
    addMergedGroup(f[0], f[1]);
    return "";
}

// slots.csv: slot,start,end. The biggest slot number becomes slotCount.
string checkSlotRow(const string* f) {
    int slot;
    if (!validatePositiveInt(f[0], slot)) return "invalid slot number: " + f[0];
    if (slot > slotCount) slotCount = slot;
    return "";
}

string checkTimetableRow(const string* f) {
    const string& day = f[0];
    const string& slotText = f[1];
    const string& durationText = f[2];
    const string& sectionOrGroup = f[3];
    const string& subject = f[4];
    const string& facultyId = f[5];
    const string& venueId = f[6];

    if (day.empty() || sectionOrGroup.empty() || subject.empty() || facultyId.empty() || venueId.empty())
        return "empty fields";

    int slot;
    if (!validatePositiveInt(slotText, slot)) return "invalid slot: " + slotText;
    if (slotCount > 0 && slot > slotCount) return "invalid slot: " + slotText;

    int duration;
    if (!validatePositiveInt(durationText, duration)) return "invalid duration: " + durationText;
    if (slotCount > 0 && slot + duration - 1 > slotCount)
        return "class goes past the last slot: " + durationText;

    bool sectionExists = searchSectionById(sectionOrGroup) != nullptr;
    bool groupExists = searchMergedGroupById(sectionOrGroup) != nullptr;
    if (!sectionExists && !groupExists) return "section/group not found: " + sectionOrGroup;
    if (searchFacultyById(facultyId) == nullptr) return "faculty not found: " + facultyId;
    if (searchVenueById(venueId) == nullptr) return "venue not found: " + venueId;

    addTimetableEntry(day, slot, duration, sectionOrGroup, subject, facultyId, venueId);
    return "";
}

// ==================================================================================
// FILE READING
// ==================================================================================

// Reads one CSV file. The first line is the header and is skipped.
// Every other non-empty line is checked with rowFn. Bad rows are printed and skipped.
// An optional file that is missing is not an error.
void loadCsvFile(const string& fileName, int columns, bool optional, string (*rowFn)(const string*)) {
    cout << "\n--- Loading " << fileName << " ---\n";

    ifstream file(fileName.c_str());
    if (!file.is_open()) {
        if (optional) cout << "INFO: " << fileName << " not found (optional)\n";
        else cout << "ERROR: Cannot open " << fileName << "\n";
        return;
    }

    string line;
    int rowNum = 0, loaded = 0, rejected = 0;
    getline(file, line);                               // skip the header

    while (getline(file, line)) {
        rowNum++;
        if (isBlank(line)) continue;

        string fields[16];
        if (!splitCSV(line, fields, columns)) {
            cout << "  Row " << rowNum << ": REJECTED - expected " << columns << " fields\n";
            rejected++;
            continue;
        }

        string problem = rowFn(fields);
        if (problem.empty()) {
            loaded++;
        } else {
            cout << "  Row " << rowNum << ": REJECTED - " << problem << "\n";
            rejected++;
        }
    }

    file.close();
    cout << "RESULT: Loaded " << loaded << " | Rejected " << rejected << "\n";
}

// Reads config.txt (optional). Lines look like: key=value. Lines that start with # are comments.
void loadConfigFile() {
    ifstream file("config.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == string::npos) continue;

        string key = trim(line.substr(0, eq));
        string value = trim(line.substr(eq + 1));
        if (key == "max_sections_in_auditorium") maxAudSections = atoi(value.c_str());
        else if (key == "auditorium_type") auditoriumType = value;
    }
    file.close();
}

// Loads every file of the data folder, in the right order.
// The order matters: a timetable row refers to faculty, sections and venues, so they come first.
void loadAllCSVFiles() {
    loadConfigFile();
    slotCount = 0;                                     // slots.csv sets it again if the file exists

    cout << "\n========================================\n";
    cout << "LOADING ALL CSV FILES\n";
    cout << "========================================\n";

    loadCsvFile("faculty.csv", 2, false, checkFacultyRow);
    loadCsvFile("sections.csv", 3, false, checkSectionRow);
    loadCsvFile("venues.csv", 4, false, checkVenueRow);
    loadCsvFile("merged_groups.csv", 2, true, checkMergedGroupRow);
    loadCsvFile("slots.csv", 3, true, checkSlotRow);
    loadCsvFile("timetable.csv", 7, false, checkTimetableRow);

    cout << "\nCSV loading complete!\n";
}

// ==================================================================================
// MEMORY CLEANUP  (delete every node, so that nothing is left in memory)
// ==================================================================================
void freeAllMemory() {
    while (facultyHead != nullptr) {
        Faculty* temp = facultyHead;
        facultyHead = facultyHead->next;
        delete temp;
    }
    facultyTail = nullptr;

    while (sectionHead != nullptr) {
        Section* temp = sectionHead;
        sectionHead = sectionHead->next;
        delete temp;
    }
    sectionTail = nullptr;

    while (venueHead != nullptr) {
        Venue* temp = venueHead;
        venueHead = venueHead->next;
        delete temp;
    }
    venueTail = nullptr;

    while (timetableHead != nullptr) {
        TimetableEntry* temp = timetableHead;
        timetableHead = timetableHead->next;
        delete temp;
    }
    timetableTail = nullptr;

    while (mergedGroupHead != nullptr) {
        MergedGroup* temp = mergedGroupHead;
        mergedGroupHead = mergedGroupHead->next;
        delete temp;
    }
    mergedGroupTail = nullptr;
}

// ==================================================================================
// STAND-ALONE CHECK  (only used when this file is compiled on its own)
// ==================================================================================
int main() {
    loadAllCSVFiles();

    int faculty = 0, sections = 0, venues = 0, groups = 0, classes = 0;
    for (Faculty* p = facultyHead; p; p = p->next) faculty++;
    for (Section* p = sectionHead; p; p = p->next) sections++;
    for (Venue* p = venueHead; p; p = p->next) venues++;
    for (MergedGroup* p = mergedGroupHead; p; p = p->next) groups++;
    for (TimetableEntry* p = timetableHead; p; p = p->next) classes++;

    cout << "\nLoaded: " << faculty << " faculty, " << sections << " sections, " << venues << " venues, "
         << groups << " merged groups, " << classes << " classes\n";

    freeAllMemory();
    return 0;
}
