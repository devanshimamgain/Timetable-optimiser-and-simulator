
#include "modules/file_handling/person1_file_handling.h"
#include "modules/clash_detection/clashdetect.h"
#include "modules/simulation/timetable_gui.h"

#include <iostream>

int main() {
    // Load all input CSV files
    loadAllCSVFiles();

    // Build the list of days
    buildDayList();

    // Run clash detection
    runClashCheck();

    // Launch the graphical simulator
    TimetableGUI gui;
    gui.run();

    // Free dynamically allocated memory
    freeAllMemory();

    return 0;
}
