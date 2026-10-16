#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <iomanip>
using namespace std;


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

/

string trim(const string& s) {
    size_t start = 0;
    size_t end = s.size();
    
    while (start < end && isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    
    return s.substr(start, end - start);
}

bool isBlank(const string& s) {
    return trim(s).empty();
}

bool validatePositiveInt(const string& s, int& out) {
    string trimmed = trim(s);
    if (trimmed.empty()) return false;
    
    stringstream ss(trimmed);
    int value;
    char extra;
    
    if (!(ss >> value) || (ss >> extra) || value <= 0) {
        return false;
    }
    
    out = value;
    return true;
}

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

// SEARCH FUNCTIONS (LINKED LIST TRAVERSAL)

Faculty* searchFacultyById(const string& id) {
    for (Faculty* current = facultyHead; current != nullptr; current = current->next) {
        if (current->id == id) {
            return current;
        }
    }
    return nullptr;
}

Section* searchSectionById(const string& id) {
    for (Section* current = sectionHead; current != nullptr; current = current->next) {
        if (current->id == id) {
            return current;
        }
    }
    return nullptr;
}

Venue* searchVenueById(const string& id) {
    for (Venue* current = venueHead; current != nullptr; current = current->next) {
        if (current->id == id) {
            return current;
        }
    }
    return nullptr;
}

MergedGroup* searchMergedGroupById(const string& id) {
    for (MergedGroup* current = mergedGroupHead; current != nullptr; current = current->next) {
        if (current->id == id) {
            return current;
        }
    }
    return nullptr;
}

bool validateSectionList(const string& sectionIds) {
    stringstream ss(sectionIds);
    string section;
    bool foundAny = false;
    
    while (getline(ss, section, ';')) {
        section = trim(section);
        if (section.empty()) continue;
        
        if (searchSectionById(section) == nullptr) {
            return false;
        }
        foundAny = true;
    }
    
    return foundAny;
}

// INSERT FUNCTIONS (LINKED LIST INSERTION)

void insertFaculty(const string& id, const string& name) {
    Faculty* newNode = new Faculty{ id, name, nullptr };
    
    if (facultyTail == nullptr) {
        facultyHead = newNode;
        facultyTail = newNode;
    } else {
        facultyTail->next = newNode;
        facultyTail = newNode;
    }
}

void insertSection(const string& id, const string& name, int strength) {
    Section* newNode = new Section{ id, name, strength, nullptr };
    
    if (sectionTail == nullptr) {
        sectionHead = newNode;
        sectionTail = newNode;
    } else {
        sectionTail->next = newNode;
        sectionTail = newNode;
    }
}

void insertVenue(const string& id, const string& name, const string& type, int capacity) {
    Venue* newNode = new Venue{ id, name, type, capacity, nullptr };
    
    if (venueTail == nullptr) {
        venueHead = newNode;
        venueTail = newNode;
    } else {
        venueTail->next = newNode;
        venueTail = newNode;
    }
}

void insertTimetableEntry(const string& day, int slot, int duration, const string& sectionOrGroup,
                         const string& subject, const string& facultyId, const string& venueId) {
    TimetableEntry* newNode = new TimetableEntry{
        day, slot, duration, sectionOrGroup, subject, facultyId, venueId, nullptr
    };
    
    if (timetableTail == nullptr) {
        timetableHead = newNode;
        timetableTail = newNode;
    } else {
        timetableTail->next = newNode;
        timetableTail = newNode;
    }
}

void insertMergedGroup(const string& id, const string& sectionIds) {
    MergedGroup* newNode = new MergedGroup{ id, sectionIds, nullptr };
    
    if (mergedGroupTail == nullptr) {
        mergedGroupHead = newNode;
        mergedGroupTail = newNode;
    } else {
        mergedGroupTail->next = newNode;
        mergedGroupTail = newNode;
    }
}

// CSV FILE LOADERS (WITH VALIDATION)

void loadFacultyCSV() {
    cout << "\n--- Loading faculty.csv ---\n";
    
    ifstream file("faculty.csv");
    if (!file.is_open()) {
        cout << "ERROR: Cannot open faculty.csv\n";
        return;
    }
    
    string line;
    int rowNum = 0;
    int successCount = 0;
    int rejectCount = 0;
    
    getline(file, line);
    
    while (getline(file, line)) {
        rowNum++;
        
        if (isBlank(line)) continue;
        
        string fields[2];
        if (!splitCSV(line, fields, 2)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid format (expected 2 fields)\n";
            rejectCount++;
            continue;
        }
        
        string facultyId = fields[0];
        string facultyName = fields[1];
        
        if (facultyId.empty() || facultyName.empty()) {
            cout << "  Row " << rowNum << ": REJECTED - Empty faculty ID or name\n";
            rejectCount++;
            continue;
        }
        
        if (searchFacultyById(facultyId) != nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Duplicate faculty ID: " << facultyId << "\n";
            rejectCount++;
            continue;
        }
        
        insertFaculty(facultyId, facultyName);
        successCount++;
    }
    
    file.close();
    cout << "RESULT: Loaded " << successCount << " | Rejected " << rejectCount << "\n";
}

void loadSectionsCSV() {
    cout << "\n--- Loading sections.csv ---\n";
    
    ifstream file("sections.csv");
    if (!file.is_open()) {
        cout << "ERROR: Cannot open sections.csv\n";
        return;
    }
    
    string line;
    int rowNum = 0;
    int successCount = 0;
    int rejectCount = 0;
    
    getline(file, line);
    
    while (getline(file, line)) {
        rowNum++;
        
        if (isBlank(line)) continue;
        
        string fields[3];
        if (!splitCSV(line, fields, 3)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid format (expected 3 fields)\n";
            rejectCount++;
            continue;
        }
        
        string sectionId = fields[0];
        string sectionName = fields[1];
        string strengthStr = fields[2];
        
        if (sectionId.empty() || sectionName.empty()) {
            cout << "  Row " << rowNum << ": REJECTED - Empty section ID or name\n";
            rejectCount++;
            continue;
        }
        
        int strength;
        if (!validatePositiveInt(strengthStr, strength)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid strength: " << strengthStr << "\n";
            rejectCount++;
            continue;
        }
        
        if (searchSectionById(sectionId) != nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Duplicate section ID: " << sectionId << "\n";
            rejectCount++;
            continue;
        }
        
        insertSection(sectionId, sectionName, strength);
        successCount++;
    }
    
    file.close();
    cout << "RESULT: Loaded " << successCount << " | Rejected " << rejectCount << "\n";
}

void loadVenuesCSV() {
    cout << "\n--- Loading venues.csv ---\n";
    
    ifstream file("venues.csv");
    if (!file.is_open()) {
        cout << "ERROR: Cannot open venues.csv\n";
        return;
    }
    
    string line;
    int rowNum = 0;
    int successCount = 0;
    int rejectCount = 0;
    
    getline(file, line);
    
    while (getline(file, line)) {
        rowNum++;
        
        if (isBlank(line)) continue;
        
        string fields[4];
        if (!splitCSV(line, fields, 4)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid format (expected 4 fields)\n";
            rejectCount++;
            continue;
        }
        
        string venueId = fields[0];
        string venueName = fields[1];
        string venueType = fields[2];
        string capacityStr = fields[3];
        
        if (venueId.empty() || venueName.empty() || venueType.empty()) {
            cout << "  Row " << rowNum << ": REJECTED - Empty venue ID, name, or type\n";
            rejectCount++;
            continue;
        }
        
        int capacity;
        if (!validatePositiveInt(capacityStr, capacity)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid capacity: " << capacityStr << "\n";
            rejectCount++;
            continue;
        }
        
        if (searchVenueById(venueId) != nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Duplicate venue ID: " << venueId << "\n";
            rejectCount++;
            continue;
        }
        
        insertVenue(venueId, venueName, venueType, capacity);
        successCount++;
    }
    
    file.close();
    cout << "RESULT: Loaded " << successCount << " | Rejected " << rejectCount << "\n";
}

void loadMergedGroupsCSV() {
    cout << "\n--- Loading merged_groups.csv ---\n";
    
    ifstream file("merged_groups.csv");
    if (!file.is_open()) {
        cout << "INFO: merged_groups.csv not found (optional)\n";
        return;
    }
    
    string line;
    int rowNum = 0;
    int successCount = 0;
    int rejectCount = 0;
    
    getline(file, line);
    
    while (getline(file, line)) {
        rowNum++;
        
        if (isBlank(line)) continue;
        
        string fields[2];
        if (!splitCSV(line, fields, 2)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid format (expected 2 fields)\n";
            rejectCount++;
            continue;
        }
        
        string groupId = fields[0];
        string sectionIds = fields[1];
        
        if (groupId.empty()) {
            cout << "  Row " << rowNum << ": REJECTED - Empty group ID\n";
            rejectCount++;
            continue;
        }
        
        if (!validateSectionList(sectionIds)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid section list: " << sectionIds << "\n";
            rejectCount++;
            continue;
        }
        
        if (searchMergedGroupById(groupId) != nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Duplicate group ID: " << groupId << "\n";
            rejectCount++;
            continue;
        }
        
        insertMergedGroup(groupId, sectionIds);
        successCount++;
    }
    
    file.close();
    cout << "RESULT: Loaded " << successCount << " | Rejected " << rejectCount << "\n";
}

void loadTimetableCSV() {
    cout << "\n--- Loading timetable.csv ---\n";
    
    ifstream file("timetable.csv");
    if (!file.is_open()) {
        cout << "ERROR: Cannot open timetable.csv\n";
        return;
    }
    
    string line;
    int rowNum = 0;
    int successCount = 0;
    int rejectCount = 0;
    
    getline(file, line);
    
    while (getline(file, line)) {
        rowNum++;
        
        if (isBlank(line)) continue;
        
        string fields[7];
        if (!splitCSV(line, fields, 7)) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid format (expected 7 fields)\n";
            rejectCount++;
            continue;
        }
        
        string day = fields[0];
        string slotStr = fields[1];
        string durationStr = fields[2];
        string sectionOrGroup = fields[3];
        string subject = fields[4];
        string facultyId = fields[5];
        string venueId = fields[6];
        
        if (day.empty() || sectionOrGroup.empty() || subject.empty() || 
            facultyId.empty() || venueId.empty()) {
            cout << "  Row " << rowNum << ": REJECTED - Empty fields\n";
            rejectCount++;
            continue;
        }
        
        int slot;
        if (!validatePositiveInt(slotStr, slot) || slot > 10) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid slot: " << slotStr << "\n";
            rejectCount++;
            continue;
        }
        
        int duration;
        if (!validatePositiveInt(durationStr, duration) || duration > 2) {
            cout << "  Row " << rowNum << ": REJECTED - Invalid duration: " << durationStr << "\n";
            rejectCount++;
            continue;
        }
        
        bool sectionExists = (searchSectionById(sectionOrGroup) != nullptr);
        bool groupExists = (searchMergedGroupById(sectionOrGroup) != nullptr);
        if (!sectionExists && !groupExists) {
            cout << "  Row " << rowNum << ": REJECTED - Section/group not found: " << sectionOrGroup << "\n";
            rejectCount++;
            continue;
        }
        
        if (searchFacultyById(facultyId) == nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Faculty not found: " << facultyId << "\n";
            rejectCount++;
            continue;
        }
        
        if (searchVenueById(venueId) == nullptr) {
            cout << "  Row " << rowNum << ": REJECTED - Venue not found: " << venueId << "\n";
            rejectCount++;
            continue;
        }
        
        insertTimetableEntry(day, slot, duration, sectionOrGroup, subject, facultyId, venueId);
        successCount++;
    }
    
    file.close();
    cout << "RESULT: Loaded " << successCount << " | Rejected " << rejectCount << "\n";
}

void loadAllCSVFiles() {
    cout << "\n========================================\n";
    cout << "LOADING ALL CSV FILES\n";
    cout << "========================================\n";
    
    loadFacultyCSV();
    loadSectionsCSV();
    loadVenuesCSV();
    loadMergedGroupsCSV();
    loadTimetableCSV();
    
    cout << "\nCSV loading complete!\n";
}

// DISPLAY FUNCTIONS (LINKED LIST TRAVERSAL)

void displayFacultyPool() {
    cout << "\n--- FACULTY POOL ---\n";
    if (facultyHead == nullptr) {
        cout << "(empty)\n";
        return;
    }
    
    cout << left << setw(12) << "Faculty ID" << setw(30) << "Name\n";
    cout << string(42, '-') << "\n";
    
    int count = 0;
    for (Faculty* current = facultyHead; current != nullptr; current = current->next) {
        cout << left << setw(12) << current->id << setw(30) << current->name << "\n";
        count++;
    }
    cout << "Total: " << count << " faculty members\n";
}

void displaySectionPool() {
    cout << "\n--- SECTION POOL ---\n";
    if (sectionHead == nullptr) {
        cout << "(empty)\n";
        return;
    }
    
    cout << left << setw(12) << "Section ID" << setw(20) << "Name" << setw(12) << "Strength\n";
    cout << string(44, '-') << "\n";
    
    int count = 0;
    for (Section* current = sectionHead; current != nullptr; current = current->next) {
        cout << left << setw(12) << current->id << setw(20) << current->name 
             << setw(12) << current->strength << "\n";
        count++;
    }
    cout << "Total: " << count << " sections\n";
}

void displayVenuePool() {
    cout << "\n--- VENUE POOL ---\n";
    if (venueHead == nullptr) {
        cout << "(empty)\n";
        return;
    }
    
    cout << left << setw(12) << "Venue ID" << setw(20) << "Name" << setw(15) << "Type" << setw(12) << "Capacity\n";
    cout << string(59, '-') << "\n";
    
    int count = 0;
    for (Venue* current = venueHead; current != nullptr; current = current->next) {
        cout << left << setw(12) << current->id << setw(20) << current->name 
             << setw(15) << current->type << setw(12) << current->capacity << "\n";
        count++;
    }
    cout << "Total: " << count << " venues\n";
}

void displayMergedGroups() {
    cout << "\n--- MERGED GROUPS ---\n";
    if (mergedGroupHead == nullptr) {
        cout << "(none loaded)\n";
        return;
    }
    
    cout << left << setw(12) << "Group ID" << setw(50) << "Sections\n";
    cout << string(62, '-') << "\n";
    
    int count = 0;
    for (MergedGroup* current = mergedGroupHead; current != nullptr; current = current->next) {
        cout << left << setw(12) << current->id << setw(50) << current->sectionIds << "\n";
        count++;
    }
    cout << "Total: " << count << " merged groups\n";
}

void displayTimetable() {
    cout << "\n--- TIMETABLE ---\n";
    if (timetableHead == nullptr) {
        cout << "(empty)\n";
        return;
    }
    
    cout << left << setw(10) << "Day" << setw(6) << "Slot" << setw(4) << "Dur" 
         << setw(10) << "Section" << setw(15) << "Subject" << setw(10) << "Faculty" << setw(10) << "Venue\n";
    cout << string(75, '-') << "\n";
    
    int count = 0;
    for (TimetableEntry* current = timetableHead; current != nullptr; current = current->next) {
        cout << left << setw(10) << current->day << setw(6) << current->startSlot 
             << setw(4) << current->duration << setw(10) << current->sectionOrGroup 
             << setw(15) << current->subject << setw(10) << current->facultyId 
             << setw(10) << current->venueId << "\n";
        count++;
    }
    cout << "Total: " << count << " timetable entries\n";
}

void displaySummary() {
    cout << "\n========================================\n";
    cout << "DATA SUMMARY\n";
    cout << "========================================\n";
    
    int facultyCount = 0, sectionCount = 0, venueCount = 0, groupCount = 0, timetableCount = 0;
    
    for (Faculty* p = facultyHead; p != nullptr; p = p->next) facultyCount++;
    for (Section* p = sectionHead; p != nullptr; p = p->next) sectionCount++;
    for (Venue* p = venueHead; p != nullptr; p = p->next) venueCount++;
    for (MergedGroup* p = mergedGroupHead; p != nullptr; p = p->next) groupCount++;
    for (TimetableEntry* p = timetableHead; p != nullptr; p = p->next) timetableCount++;
    
    cout << "\nFaculty Members:   " << facultyCount << "\n";
    cout << "Sections:          " << sectionCount << "\n";
    cout << "Venues:            " << venueCount << "\n";
    cout << "Merged Groups:     " << groupCount << "\n";
    cout << "Timetable Entries: " << timetableCount << "\n";
}

void searchFacultyDemo() {
    cout << "\n--- Search Faculty ---\n";
    cout << "Enter faculty ID: ";
    string id;
    getline(cin, id);
    
    Faculty* found = searchFacultyById(trim(id));
    if (found != nullptr) {
        cout << "FOUND: " << found->id << " | " << found->name << "\n";
    } else {
        cout << "NOT FOUND\n";
    }
}

void searchSectionDemo() {
    cout << "\n--- Search Section ---\n";
    cout << "Enter section ID: ";
    string id;
    getline(cin, id);
    
    Section* found = searchSectionById(trim(id));
    if (found != nullptr) {
        cout << "FOUND: " << found->id << " | " << found->name 
             << " | Strength: " << found->strength << "\n";
    } else {
        cout << "NOT FOUND\n";
    }
}

void searchVenueDemo() {
    cout << "\n--- Search Venue ---\n";
    cout << "Enter venue ID: ";
    string id;
    getline(cin, id);
    
    Venue* found = searchVenueById(trim(id));
    if (found != nullptr) {
        cout << "FOUND: " << found->id << " | " << found->name 
             << " | Type: " << found->type << " | Capacity: " << found->capacity << "\n";
    } else {
        cout << "NOT FOUND\n";
    }
}

// MEMORY CLEANUP

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

// MENU SYSTEM

void displayMenu() {
    cout << "\n========================================\n";
    cout << "PERSON 1: CSV Handler + Linked Lists\n";
    cout << "Data Loading & Validation\n";
    cout << "========================================\n\n";
    
    cout << "1. Load all CSV files (with validation)\n";
    cout << "2. Display faculty pool\n";
    cout << "3. Display section pool\n";
    cout << "4. Display venue pool\n";
    cout << "5. Display merged groups\n";
    cout << "6. Display timetable\n";
    cout << "7. Display summary\n";
    cout << "8. Search faculty\n";
    cout << "9. Search section\n";
    cout << "10. Search venue\n";
    cout << "0. Exit\n";
    cout << "\nChoose option: ";
}

int main() {
    cout << "\n========================================\n";
    cout << "TIMETABLE OPTIMIZER & SIMULATOR\n";
    cout << "PERSON 1: CSV Handler & Data Structures\n";
    cout << "========================================\n";
    
    while (true) {
        displayMenu();
        
        string input;
        if (!getline(cin, input)) break;
        
        stringstream ss(input);
        int choice;
        if (!(ss >> choice)) {
            cout << "Invalid input\n";
            continue;
        }
        
        switch (choice) {
            case 1:
                loadAllCSVFiles();
                break;
            case 2:
                displayFacultyPool();
                break;
            case 3:
                displaySectionPool();
                break;
            case 4:
                displayVenuePool();
                break;
            case 5:
                displayMergedGroups();
                break;
            case 6:
                displayTimetable();
                break;
            case 7:
                displaySummary();
                break;
            case 8:
                searchFacultyDemo();
                break;
            case 9:
                searchSectionDemo();
                break;
            case 10:
                searchVenueDemo();
                break;
            case 0:
                freeAllMemory();
                cout << "\nAll memory freed. Goodbye!\n";
                return 0;
            default:
                cout << "Invalid choice\n";
        }
    }
    
    freeAllMemory();
    return 0;
}
