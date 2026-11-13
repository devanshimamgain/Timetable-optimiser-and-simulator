#pragma once
#include "../file_handling/person1_file_handling.h"
#include <string>
#include <vector>

// One record for each detected clash between two timetable entries.
struct ClashInfo {
    std::string day;
    int slot;                   // Later of the two starting slots
    std::string kind;           // SECTION, FACULTY or VENUE
    std::string text;           // Human-readable explanation
    TimetableEntry* a;
    TimetableEntry* b;
};

// Runs the multithreaded clash check on the timetable loaded by Person 1.
void runClashCheck();

// Returns the current clash list. The pointers refer to entries in Person 1's
// timetable, so do not delete them here.
const std::vector<ClashInfo>& getClashes();

// Returns true if a timetable entry is involved in at least one clash.
bool hasClash(const TimetableEntry* entry);

// Number of worker threads used by the most recent clash check.
int getThreadsInUse();

// Console output helpers (optional; the GUI does not need to call these).
void printClashes();

// DEMO ONLY: deliberately changes some timetable entries to create clashes.
void spoilTimetable();

// Attempts to resolve clashes by changing a venue and/or time.
// This function modifies the timetable entries in place.
void optimise();
void buildDayList();