# PERSON 1: CSV Handler & Linked List Data Storage
## Timetable Optimizer & Simulator (Graphic Era University - B.Tech CSE Sem III)

---

## YOUR RESPONSIBILITIES

### 1. **CSV File Reading** (C++ File Handling)
   - Open and parse CSV files using `ifstream` and `getline()`
   - Split comma-separated values using custom `splitCSV()` function
   - Handle errors: missing files, malformed lines, invalid formats

### 2. **Linked List Data Structures** (No STL)
   - Implement 5 linked lists with head/tail pointers:
     - **Faculty**: faculty_id → name
     - **Sections**: section_id → name → strength
     - **Venues**: venue_id → name → type → capacity
     - **Merged Groups**: group_id → section_ids (semicolon-separated)
     - **Timetable**: day, slot, duration, section, subject, faculty, venue
   - O(1) insertion at tail using tail pointer
   - Proper memory management (dynamic allocation, deallocation)

### 3. **Data Validation** (NO Clash Detection)
   - **Duplicate IDs**: No two faculty/section/venue/group with same ID
   - **Invalid Fields**: Empty strings, missing data
   - **Invalid Numbers**: Negative strengths, capacities, invalid slot numbers
   - **Referential Integrity**: 
     - Faculty IDs in timetable must exist in faculty pool
     - Section/group IDs must exist in section/merged_group pools
     - Venue IDs must exist in venue pool
   - **Slot Validation**: Slots must be 1-10, duration must be 1 or 2
   - Report each rejected row with reason

### 4. **Search & Display Functions**
   - Traverse linked lists to display all records
   - Search by ID
   - Show summary counts

---

## YOUR CSV FILE FORMAT (FINAL)

### 1. **faculty.csv** (2 columns)
```
faculty_id,name
F001,Dr. Siddhant Thapliyal
F002,Dr. Prateek Srivastava
...
F021,Ms. Sameeksha Ahuja
```
- **21 faculty members** for Sections A-E
- Extracted from actual timetable PDF

### 2. **sections.csv** (3 columns)
```
section_id,name,strength
A,Section A,60
B,Section B,58
C,Section C,62
D,Section D,60
E,Section E,61
```
- **5 sections** (A, B, C, D, E)
- Strength = number of students per section

### 3. **venues.csv** (4 columns)
```
venue_id,name,type,capacity
LT-106,Lecture Theatre 106,lecture_theatre,120
CR-017,Classroom 017,classroom,60
LAB-009,Lab 009,lab,40
AUDI-529,Auditorium 529,auditorium,200
...
```
- **24 venues** total
- Types: lecture_theatre, classroom, lab, auditorium
- Capacity based on venue type

### 4. **merged_groups.csv** (2 columns, semicolon-separated sections)
```
merged_id,section_ids
CEC-G1,A;B;C;D;E
ELEC-A,A
ELEC-B,B
...
```
- **CEC-G1**: All 5 sections attend together (Career Excellence)
- **ELEC-***: Each section's elective (single section)

### 5. **timetable.csv** (7 columns)
```
day,startSlot,duration,sectionOrGroup,subject,facultyId,venueId
Monday,1,1,A,TCS307,F002,LT-124
Monday,5,2,A,PCS302,F012,LAB-302-A
...
```
- **Days**: Monday - Saturday (case-sensitive)
- **Slots**: 1-10 (see slot times below)
- **Duration**: 1 = 1-hour class, 2 = 2-hour lab (consecutive slots)
- **sectionOrGroup**: Single section ID (A, B, C, D, E) OR merged group ID (CEC-G1, ELEC-A, etc.)
- **Subject**: Course code (TCS302, PCS307, etc.)
- **facultyId**: Must exist in faculty.csv
- **venueId**: Must exist in venues.csv

**Time Slots:**
```
Slot 1:  08:00 - 08:55
Slot 2:  08:55 - 09:50
Slot 3:  10:10 - 11:05
Slot 4:  11:05 - 12:00
Slot 5:  12:00 - 12:55
Slot 6:  12:55 - 01:50 PM
Slot 7:  02:10 - 03:05 PM
Slot 8:  03:05 - 04:00 PM
Slot 9:  04:00 - 04:55 PM
Slot 10: 04:55 - 05:50 PM
```

**Subject Codes (Sections A-E):**
- TCS302: Data Structures with C
- TCS307: OOP with C++
- TCS308: Logic Design
- TMA316: Discrete Structures
- XCS301: Career Skills-I
- PCS302: Lab (Data Structures)
- PCS307: Lab (OOP with C++)
- PCS308: Lab (Logic Design)
- CEC: Career Excellence
- ELEC-*: Electives (Cloud Computing, Cryptography, Python, Swayam)

---

## COMPILATION & RUNNING

### Step 1: Prepare CSV Files
Copy the final CSV files to your working directory:
```bash
cp faculty_final.csv faculty.csv
cp sections_final.csv sections.csv
cp venues_final.csv venues.csv
cp merged_groups_final.csv merged_groups.csv
cp timetable_final.csv timetable.csv
```

**OR** directly use the final versions by editing the program to load from `*_final.csv`.

### Step 2: Compile
```bash
g++ -std=c++17 Person1_CSVHandler.cpp -o person1
```

### Step 3: Run
```bash
./person1
```

---

## MENU SYSTEM (10 OPTIONS)

```
1. Load all CSV files (with validation)
2. Display faculty pool
3. Display section pool
4. Display venue pool
5. Display merged groups
6. Display timetable
7. Display summary
8. Search faculty
9. Search section
10. Search venue
0. Exit
```

### Typical Workflow:
1. **Option 1**: Load all CSVs → See validation report (accepted/rejected counts)
2. **Option 7**: View summary (total counts of each data type)
3. **Option 2-5**: Browse each pool individually
4. **Option 6**: View all timetable entries
5. **Option 8-10**: Search for specific records
6. **Option 0**: Exit (frees all dynamically allocated memory)

---

## EXPECTED OUTPUT EXAMPLE

### After Option 1 (Load all CSVs):
```
╔════════════════════════════════════╗
║  LOADING ALL CSV FILES             ║
╚════════════════════════════════════╝

--- Loading faculty.csv ---
✓ Loaded: 21 | Rejected: 0

--- Loading sections.csv ---
✓ Loaded: 5 | Rejected: 0

--- Loading venues.csv ---
✓ Loaded: 24 | Rejected: 0

--- Loading merged_groups.csv ---
✓ Loaded: 6 | Rejected: 0

--- Loading timetable.csv ---
✓ Loaded: 120 | Rejected: 0

✓ CSV loading complete!
```

### After Option 7 (Summary):
```
╔════════════════════════════════════╗
║  DATA SUMMARY                      ║
╚════════════════════════════════════╝

Faculty Members:   21
Sections:          5
Venues:            24
Merged Groups:     6
Timetable Entries: 120
```

---

## VALIDATION RULES (COMPREHENSIVE CHECKLIST)

### Faculty Pool:
- ✓ No empty faculty_id or name
- ✓ No duplicate faculty_id
- ✓ Exactly 2 fields per row

### Sections Pool:
- ✓ No empty section_id or name
- ✓ Strength must be positive integer
- ✓ No duplicate section_id
- ✓ Exactly 3 fields per row

### Venues Pool:
- ✓ No empty venue_id, name, or type
- ✓ Capacity must be positive integer
- ✓ No duplicate venue_id
- ✓ Exactly 4 fields per row

### Merged Groups:
- ✓ No empty group_id
- ✓ At least one valid section ID in list
- ✓ All sections must exist in sections pool
- ✓ No duplicate group_id
- ✓ Exactly 2 fields per row

### Timetable Entries:
- ✓ No empty day, section, subject, faculty, or venue
- ✓ Day must be Monday-Saturday
- ✓ Slot must be 1-10 (positive integer)
- ✓ Duration must be 1 or 2
- ✓ sectionOrGroup must exist (either as section_id OR merged_id)
- ✓ facultyId must exist in faculty pool
- ✓ venueId must exist in venue pool
- ✓ Exactly 7 fields per row

---

## DATA STRUCTURES IN CODE

### Node Structures (Linked List):
```cpp
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

struct MergedGroup {
    string id;
    string sectionIds;  // "A;B;C"
    MergedGroup* next;
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
```

### Global Pointers (for each linked list):
```cpp
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
```

### Key Functions You Must Understand:

**Insertion (O(1) with tail pointer):**
```cpp
void insertFaculty(const string& id, const string& name);
void insertSection(const string& id, const string& name, int strength);
void insertVenue(const string& id, const string& name, const string& type, int capacity);
void insertTimetableEntry(...);
void insertMergedGroup(...);
```

**Traversal (O(n)):**
```cpp
void displayFacultyPool();
void displaySectionPool();
void displayVenuePool();
void displayTimetable();
```

**Search (O(n)):**
```cpp
Faculty* searchFacultyById(const string& id);
Section* searchSectionById(const string& id);
Venue* searchVenueById(const string& id);
MergedGroup* searchMergedGroupById(const string& id);
bool validateSectionList(const string& sectionIds);  // for merged groups
```

**Memory Management:**
```cpp
void freeAllMemory();  // Deallocates all nodes when exiting
```

---

## TESTING CHECKLIST

### Test 1: Clean Load (No Errors)
- [ ] Run Option 1 → All counts should be: Faculty 21, Sections 5, Venues 24, Groups 6, Timetable 120
- [ ] Run Option 7 → Same counts in summary

### Test 2: Display All Pools
- [ ] Option 2 → Display faculty (21 total)
- [ ] Option 3 → Display sections (5 total)
- [ ] Option 4 → Display venues (24 total)
- [ ] Option 5 → Display merged groups (6 total)
- [ ] Option 6 → Display timetable (120 total)

### Test 3: Search Functions
- [ ] Option 8 → Search for "F001" → Should find Dr. Siddhant Thapliyal
- [ ] Option 9 → Search for "A" → Should find Section A, Strength 60
- [ ] Option 10 → Search for "LT-106" → Should find Lecture Theatre 106, Cap 120

### Test 4: Rejection Testing (Prepare Invalid CSVs)
Create a test file with known errors:
```csv
faculty_id,name
F001,Dr. Siddhant Thapliyal
F001,Dr. Duplicate
,Ms. NoID
F003,
```
Expected: Rows 2, 3, 4 should be rejected with reasons

### Test 5: Memory Safety
- [ ] Run program, load CSVs, exit → No segmentation faults
- [ ] Use `valgrind ./person1` (optional) → No memory leaks

---

## WHAT YOU DO **NOT** DO

❌ **NOT YOUR RESPONSIBILITY:**
- Clash detection (faculty double-booked, section in 2 places, etc.) → Person 3
- Graph representation / BFS / DFS → Person 2
- Multithreading → Person 3
- GUI / SFML visualization → Person 4

---

## HANDOFF TO PERSON 2

Once your module is working, provide Person 2 with:
1. **Compiled executable**: `person1`
2. **Header file** (optional): Extract the struct definitions for reuse
3. **Sample data**: The 5 CSV files loaded successfully
4. **Module summary**:
   - Data loaded into 5 linked lists
   - All validation passed (zero rejections)
   - Ready for graph representation

---

## HANDOFF TO PERSON 3

Person 3 will:
- Take your linked list data
- Add clash detection logic
- NOT modify your CSV reading or validation

---

## KEY LEARNING OUTCOMES

By completing this module, you will have:
- ✓ Mastered **C++ file I/O** (ifstream, getline, is_open)
- ✓ Built **custom linked lists** from scratch (no STL vector/list)
- ✓ Implemented **O(1) insertion** using tail pointers
- ✓ Practiced **data validation** and error handling
- ✓ Learned **memory management** (new/delete)
- ✓ Worked with **real college timetable data**
- ✓ Collaborated on a **multi-person project**

---

## COMMON ERRORS & FIXES

| Error | Cause | Fix |
|-------|-------|-----|
| "Cannot open faculty.csv" | File not in working directory | Copy `faculty_final.csv` to `faculty.csv` |
| "segmentation fault" | Memory not freed | Call `freeAllMemory()` before exit |
| "Duplicate faculty ID" | CSV has repeat IDs | Check CSV file for duplicates |
| "Faculty not found" in timetable | Typo in faculty_id column | Verify all faculty IDs match exactly |
| "Slot must be 1-10" | Invalid slot number | Check timetable.csv slot values |

---

## NEXT STEPS

1. **Download/Copy the CSV files** (faculty_final.csv, sections_final.csv, etc.)
2. **Compile the code**: `g++ -std=c++17 Person1_CSVHandler.cpp -o person1`
3. **Run and load CSVs**: Choose Option 1
4. **Verify all counts** match expected (21, 5, 24, 6, 120)
5. **Run test displays**: Options 2-6
6. **Run searches**: Options 8-10
7. **Demonstrate to team** before moving to Person 2's work

---

## QUESTIONS FOR REFLECTION

As you work through this module, think about:

1. **Efficiency**: Why use a tail pointer for insertion? What's the time complexity difference?
2. **Validation**: Which validation rule is most critical? (Referential integrity? Duplicates?)
3. **Data Integrity**: If validation is weak, what could go wrong for Person 3's clash detection?
4. **Linked Lists vs Vectors**: When would a linked list be better than STL vector? When worse?
5. **Memory**: How do you know you've freed all memory without a memory leak?

---

## CONTACTS & COLLABORATION

- **Your role**: Data loading & validation
- **Person 2**: Graph structure (starting from your linked lists)
- **Person 3**: Clash detection (building on your validated data)
- **Person 4**: GUI/visualization (displaying Person 3's results)

**Before Person 2 starts**: Show them your working program with all CSVs loaded successfully.

---

**Good luck! Your module is critical — clean, validated data is the foundation for the entire project.**
