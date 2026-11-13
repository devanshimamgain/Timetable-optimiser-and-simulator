
#ifndef PERSON4_SIMULATOR_H
#define PERSON4_SIMULATOR_H

#include <string>
#include <vector>
#include "person1_file_handling.h"

using namespace std;

// Represents one day and slot in the simulation.
struct SimulationSlot {
    string day;
    int slot;
};

// Handles the step-by-step timetable simulation.
class TimetableSimulator {
private:
    vector<SimulationSlot> simulationSlots;
    int currentIndex;

    void buildSimulationSlots();

public:
    // Constructor
    TimetableSimulator();

    // Build or rebuild the simulation from the loaded timetable.
    void initialize();
    void refresh();

    // Simulation controls
    bool next();
    bool previous();
    void reset();

    // Current simulation information
    bool hasCurrentSlot() const;
    string getCurrentDay() const;
    int getCurrentSlotNumber() const;

    int getCurrentIndex() const;
    int getTotalSlots() const;

    // Get all classes running during the current slot.
    vector<TimetableEntry*> getCurrentEntries() const;

    // Get all classes running during a particular day and slot.
    vector<TimetableEntry*> getEntriesAt(
        const string& day,
        int slot
    ) const;
};

// Returns the individual section IDs attending a timetable entry.
// Handles both individual sections and declared merged groups.
vector<string> getEntrySections(const TimetableEntry* entry);

// Checks whether a particular section attends a timetable entry.
bool entryIncludesSection(
    const TimetableEntry* entry,
    const string& sectionId
);

#endif
