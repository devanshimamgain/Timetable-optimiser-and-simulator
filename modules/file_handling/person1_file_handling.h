
#ifndef PERSON1_FILE_HANDLING_H
#define PERSON1_FILE_HANDLING_H

#include <string>
using namespace std;

// Structures

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
    string sectionIds;
    MergedGroup* next;
};

// Global variables

extern Faculty* facultyHead;
extern Faculty* facultyTail;

extern Section* sectionHead;
extern Section* sectionTail;

extern Venue* venueHead;
extern Venue* venueTail;

extern TimetableEntry* timetableHead;
extern TimetableEntry* timetableTail;

extern MergedGroup* mergedGroupHead;
extern MergedGroup* mergedGroupTail;

extern int slotCount;
extern int maxAudSections;
extern string auditoriumType;

// Function declarations

string trim(const string& s);
bool isBlank(const string& s);
bool validatePositiveInt(const string& s, int& out);
bool splitCSV(const string& line, string out[], int expected);

Faculty* searchFacultyById(const string& id);
Section* searchSectionById(const string& id);
Venue* searchVenueById(const string& id);
MergedGroup* searchMergedGroupById(const string& id);

bool validateSectionList(const string& sectionIds);

void addFaculty(const string& id, const string& name);
void addSection(const string& id, const string& name, int strength);
void addVenue(const string& id, const string& name,
              const string& type, int capacity);

void addTimetableEntry(
    const string& day,
    int slot,
    int duration,
    const string& sectionOrGroup,
    const string& subject,
    const string& facultyId,
    const string& venueId
);

void addMergedGroup(const string& id, const string& sectionIds);

string checkFacultyRow(const string* f);
string checkSectionRow(const string* f);
string checkVenueRow(const string* f);
string checkMergedGroupRow(const string* f);
string checkSlotRow(const string* f);
string checkTimetableRow(const string* f);

void loadCsvFile(
    const string& fileName,
    int columns,
    bool optional,
    string (*rowFn)(const string*)
);

void loadConfigFile();
void loadAllCSVFiles();
void freeAllMemory();

#endif
