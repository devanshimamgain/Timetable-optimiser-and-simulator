# PERSON 1: Testing & Debugging Guide

## QUICK START (5 MINUTES)

```bash
cd /home/claude
cp *_final.csv *.csv.bak  # Backup originals
cp faculty_final.csv faculty.csv
cp sections_final.csv sections.csv
cp venues_final.csv venues.csv
cp merged_groups_final.csv merged_groups.csv
cp timetable_final.csv timetable.csv

g++ -std=c++17 Person1_CSVHandler.cpp -o person1
./person1
```

Then:
1. Type `1` (Load all CSVs)
2. Type `7` (Summary) → Should show: 21, 5, 24, 6, 120
3. Type `0` (Exit)

**If you see zero rejections and correct counts → YOUR MODULE WORKS ✓**

---

## TEST CASE 1: Clean Load (No Errors)

**What to test:** All CSVs load with zero rejections

**Run this:**
```
Option 1: Load all CSV files
```

**Expected output:**
```
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

**What to check:**
- [ ] All "Rejected: 0"?
- [ ] Loaded counts match: 21, 5, 24, 6, 120?
- [ ] No error messages?

**If PASS:** ✓ Your file reading works
**If FAIL:** Check CSV file format (commas, headers, blank lines)

---

## TEST CASE 2: Data Display (Linked List Traversal)

**What to test:** All 5 linked lists traverse correctly

**Run these:**
```
Option 2: Display faculty pool
Option 3: Display section pool
Option 4: Display venue pool
Option 5: Display merged groups
Option 6: Display timetable
```

**Expected outputs:**

### Option 2 (Faculty):
```
--- FACULTY POOL (Linked List) ---
Faculty ID   Name
--------------------------
F001         Dr. Siddhant Thapliyal
F002         Dr. Prateek Srivastava
...
F021         Ms. Sameeksha Ahuja
Total: 21 faculty members
```

### Option 3 (Sections):
```
--- SECTION POOL (Linked List) ---
Section ID   Name                 Strength
--------------------------------------------
A            Section A            60
B            Section B            58
C            Section C            62
D            Section D            60
E            Section E            61
Total: 5 sections
```

### Option 4 (Venues):
```
--- VENUE POOL (Linked List) ---
Venue ID     Name                 Type            Capacity
-------------------------------------------------------------
LT-106       Lecture Theatre 106  lecture_theatre 120
LT-124       Lecture Theatre 124  lecture_theatre 120
...
AUDI-529     Auditorium 529       auditorium      200
Total: 24 venues
```

### Option 5 (Merged Groups):
```
--- MERGED GROUPS (Linked List) ---
Group ID     Sections
-----------------------------------------------
CEC-G1       A;B;C;D;E
ELEC-A       A
ELEC-B       B
ELEC-C       C
ELEC-D       D
ELEC-E       E
Total: 6 merged groups
```

### Option 6 (Timetable - partial):
```
--- TIMETABLE (Linked List) ---
Day        Slot  Dur Section Subject       Faculty   Venue
-------------------------------------------------------------
Monday     1     1   A       TCS307        F002      LT-124
Monday     2     1   A       ELEC-A        F009      LT-124
...
Saturday   7     1   E       PCS307        F021      LAB-002-A
Total: 120 timetable entries
```

**What to check:**
- [ ] All records display in order (head → tail)?
- [ ] All counts correct?
- [ ] No segmentation faults?
- [ ] Data formatted nicely in columns?

**If PASS:** ✓ Your linked lists and traversal work
**If FAIL:** Check for null pointer errors or memory corruption

---

## TEST CASE 3: Search Functions

**What to test:** Search by ID in each pool

### Search Faculty:
```
Option 8: Search faculty
Enter faculty ID: F001
```
**Expected:** `✓ Found: F001 | Dr. Siddhant Thapliyal`

### Search Section:
```
Option 9: Search section
Enter section ID: A
```
**Expected:** `✓ Found: A | Section A | Strength: 60`

### Search Venue:
```
Option 10: Search venue
Enter venue ID: LT-106
```
**Expected:** `✓ Found: LT-106 | Lecture Theatre 106 | Type: lecture_theatre | Capacity: 120`

**Try invalid IDs:**
```
Option 8: Search faculty
Enter faculty ID: FAKE001
```
**Expected:** `✗ Faculty not found`

**What to check:**
- [ ] Valid searches find correct records?
- [ ] Invalid searches report "not found"?
- [ ] No crash on bad input?

**If PASS:** ✓ Your search functions work
**If FAIL:** Check string comparison logic

---

## TEST CASE 4: Validation (Rejection Testing)

**Create a bad CSV file with intentional errors:**

```bash
cat > faculty_bad.csv << 'EOF'
faculty_id,name
F001,Dr. Siddhant Thapliyal
F001,Dr. Duplicate
,Ms. NoID
F003,
EOF

cat > sections_bad.csv << 'EOF'
section_id,name,strength
A,Section A,60
B,Section B,abc
C,Section C,-5
C,Section C Dup,62
EOF
```

**Modify Person1_CSVHandler.cpp to use bad files temporarily:**
```cpp
// In loadFacultyCSV():
ifstream file("faculty_bad.csv");  // Changed from "faculty.csv"
```

**What you should see:**
```
--- Loading faculty.csv ---
✓ Loaded: 1 | Rejected: 3
  Row 2: ✗ Duplicate faculty ID: F001
  Row 3: ✗ Empty faculty ID or name
  Row 4: ✗ Empty faculty ID or name
```

**Test each validation rule:**

| Rule | Bad CSV Row | Expected Rejection |
|------|-------------|-------------------|
| Duplicate ID | `F001,Name` (appears twice) | "Duplicate faculty ID" |
| Empty ID | `,Name` | "Empty faculty ID" |
| Empty name | `F001,` | "Empty faculty ID or name" |
| Invalid strength | `A,Name,abc` | "Invalid strength: abc" |
| Negative capacity | `V1,Name,type,-5` | "Invalid capacity: -5" |
| Bad section list | `G1,FAKE;A` | "Invalid section list" |
| Missing faculty | Timetable row with `F999` | "Faculty not found" |
| Bad slot | Timetable row with slot `99` | "Invalid slot: 99" |

**What to check:**
- [ ] Each bad row is rejected with clear reason?
- [ ] Good rows still load successfully?
- [ ] No false rejections of valid data?

**If PASS:** ✓ Your validation logic is robust
**If FAIL:** Review validation conditions in code

---

## TEST CASE 5: Memory Safety

**Test 1: Normal exit (no leaks)**
```bash
./person1
# Type: 1 (load)
# Type: 0 (exit)
```
**Check:** No crash, "All memory freed" message appears

**Test 2: With valgrind (optional, requires Linux):**
```bash
valgrind --leak-check=full --show-leak-kinds=all ./person1
```
**Then:** 1, 0

**Expected in valgrind output:**
```
ERROR SUMMARY: 0 errors from 0 contexts
```

**Test 3: Load multiple times:**
```
1 (load)
7 (summary)
1 (load again)  # This will reload into SAME lists!
0 (exit)
```
**Check:** Does it handle double-loading? Should reject with message about duplicates

**What to check:**
- [ ] No segmentation faults on exit?
- [ ] No memory leaks (if using valgrind)?
- [ ] No double-frees?

**If PASS:** ✓ Your memory management is correct
**If FAIL:** Check `delete` statements in `freeAllMemory()`

---

## TEST CASE 6: Data Integrity (Referential Consistency)

**What to test:** Timetable rows reference valid faculty, venues, and sections

**Run this:**
```
Option 1: Load all CSVs
Option 7: Summary
```

**Then manually verify:**
```
✓ All 120 timetable entries should load
✓ For each entry:
   - facultyId exists in faculty pool (21 faculty)
   - venueId exists in venue pool (24 venues)
   - sectionOrGroup exists (5 sections + 6 groups)
```

**To verify manually:**
- Option 8: Search for faculty F001 → Found
- Option 9: Search for section A → Found
- Option 10: Search for venue LT-106 → Found

**What to check:**
- [ ] Zero timetable rejections?
- [ ] All referenced IDs can be found?
- [ ] No orphaned references?

**If PASS:** ✓ Your referential integrity works
**If FAIL:** Check CSV files for typos in faculty/venue/section IDs

---

## TEST CASE 7: Edge Cases

### Empty CSV:
```bash
cat > faculty_empty.csv << 'EOF'
faculty_id,name
EOF
```
**Expected:** `✓ Loaded: 0 | Rejected: 0`

### Single entry:
```bash
cat > sections_single.csv << 'EOF'
section_id,name,strength
Z,Section Z,1
EOF
```
**Expected:** `✓ Loaded: 1 | Rejected: 0`

### Large numbers:
```bash
cat > venues_large.csv << 'EOF'
venue_id,name,type,capacity
BIG,Big Venue,auditorium,9999
EOF
```
**Expected:** `✓ Loaded: 1 | Rejected: 0`

### Whitespace handling:
```bash
cat > faculty_space.csv << 'EOF'
faculty_id,name
  F001  ,  Dr. Siddhant Thapliyal  
EOF
```
**Expected:** `✓ Loaded: 1 | Rejected: 0` (trim() removes spaces)

**What to check:**
- [ ] Empty CSVs handled gracefully?
- [ ] Single entries load correctly?
- [ ] Large numbers accepted?
- [ ] Whitespace trimmed correctly?

**If PASS:** ✓ Your edge cases are robust
**If FAIL:** Review `trim()` and validation bounds

---

## COMMON BUGS & DEBUGGING TIPS

### Bug 1: "Segmentation fault" on exit
**Cause:** Memory not freed or double-free
**Debug:**
```cpp
// Add before each delete in freeAllMemory():
if (temp == nullptr) continue;  // Safety check
```

### Bug 2: "Duplicate" error on second load
**Cause:** Program doesn't clear linked lists between loads
**Fix:** Either restart program for each load, OR implement `clearAllLists()`

### Bug 3: Search returns wrong record
**Cause:** Search function continuing past first match
**Fix:** Check that search returns immediately after finding

### Bug 4: Validation fails on valid CSV
**Cause:** `splitCSV()` counting fields wrong or `trim()` removing too much
**Debug:** Print each field after split:
```cpp
cout << "Field 0: '" << fields[0] << "'\n";
cout << "Field 1: '" << fields[1] << "'\n";
```

### Bug 5: Memory leak in `validateSectionList()`
**Cause:** Creating many stringstream objects
**Fix:** This is fine in C++ (streams auto-destroy), but move `ss` declaration closer to use

---

## STEP-BY-STEP DEBUG WORKFLOW

If something breaks:

1. **Check compilation:**
   ```bash
   g++ -std=c++17 -Wall -Wextra Person1_CSVHandler.cpp -o person1
   # Look for warnings (ignore -Wunused-variable for this simple code)
   ```

2. **Check CSV files:**
   ```bash
   head -5 faculty.csv sections.csv venues.csv
   wc -l *.csv  # Count rows
   ```

3. **Test single CSV:**
   ```
   Run program, type 1
   Does faculty.csv load? Then sections? Then venues?
   Which one fails?
   ```

4. **Check the bad CSV:**
   ```bash
   grep -n "^$" faculty.csv  # Empty lines?
   tail faculty.csv          # Ends properly?
   file faculty.csv          # Correct encoding?
   ```

5. **Add debug output:**
   Edit code, add print statements:
   ```cpp
   cout << "[DEBUG] Read line: '" << line << "'\n";
   cout << "[DEBUG] Split into " << count << " fields\n";
   ```

6. **Re-compile and test:**
   ```bash
   g++ -std=c++17 Person1_CSVHandler.cpp -o person1
   ./person1
   ```

7. **Use file redirection for large output:**
   ```bash
   ./person1 > output.txt 2>&1
   less output.txt
   ```

---

## VALIDATION CHECKLIST (Before Handing Off)

- [ ] **Compile:** No warnings, no errors
- [ ] **Load:** All 5 CSVs load with 0 rejections (21, 5, 24, 6, 120)
- [ ] **Display:** All pools display correctly with right counts
- [ ] **Search:** Can find faculty F001, section A, venue LT-106
- [ ] **Search Invalid:** Correctly says "not found" for fake IDs
- [ ] **Exit:** Program exits cleanly, frees all memory
- [ ] **Bad Data:** Correctly rejects duplicate IDs and invalid numbers
- [ ] **Referential Integrity:** Timetable references only valid faculty/venue/sections

**Once ALL checks pass → YOU'RE READY FOR PERSON 2 ✓**

---

## DEMO SCRIPT (Show to Evaluator)

```bash
./person1
# Type 1 (load)
# Show the output → 21, 5, 24, 6, 120 loaded with 0 rejections
# Type 7 (summary)
# Show counts
# Type 2 (faculty)
# Scroll through faculty list
# Type 6 (timetable)
# Show 120 entries
# Type 0 (exit)
# Show "All memory freed. Goodbye!"
```

**Talking points for evaluator:**
- "I'm using linked lists with head/tail pointers for O(1) insertion"
- "CSV validation checks for duplicates, referential integrity, and data types"
- "All dynamic memory is properly freed in `freeAllMemory()`"
- "The timetable was extracted from the actual college PDF"
- "Zero rejections means all data is valid and consistent"

---

## WHAT TO SHOW PERSON 2

Prepare a README for Person 2:
```
HANDOFF TO PERSON 2
===================

Module Status: ✓ COMPLETE
- All 5 linked lists working
- 121 total records loaded
- Zero validation errors
- Memory management verified

Data ready for graph representation:
- faculty pool (21 members)
- sections pool (5 sections)
- venues pool (24 venues)
- merged_groups (6 groups)
- timetable (120 entries)

All references validated:
✓ Every timetable row references valid faculty
✓ Every timetable row references valid venue
✓ Every timetable row references valid section/group
✓ No duplicates anywhere

Person 2 can now build graph of:
- Faculty → Sections (who teaches whom?)
- Sections → Venues (where does section go?)
- Sections → Subjects (what classes does section take?)

Contact me with questions about the data!
```

---

Good luck with testing! Once you pass all these cases, your module is **production-ready** ✓
