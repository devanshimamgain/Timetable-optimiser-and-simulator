
#include "person4_simulator.h"

#include <algorithm>
#include <cctype>

// --------------------------------------------------
// HELPER FUNCTIONS
// --------------------------------------------------

// Convert a string to lowercase for day comparison.
static string toLowerCase(string value) {
    for (char& ch : value) {
        ch = static_cast<char>(
            tolower(static_cast<unsigned char>(ch))
        );
    }

    return value;
}

// Get the order of a weekday.
static int dayOrder(const string& day) {
    string d = toLowerCase(day);

    if (d == "monday" || d == "mon") return 1;
    if (d == "tuesday" || d == "tue") return 2;
    if (d == "wednesday" || d == "wed") return 3;
    if (d == "thursday" || d == "thu") return 4;
    if (d == "friday" || d == "fri") return 5;
    if (d == "saturday" || d == "sat") return 6;
    if (d == "sunday" || d == "sun") return 7;

    return 8;
}

// --------------------------------------------------
// CONSTRUCTOR
// --------------------------------------------------

TimetableSimulator::TimetableSimulator() {
    currentIndex = -1;
}

// --------------------------------------------------
// BUILD SIMULATION
// --------------------------------------------------

void TimetableSimulator::initialize() {
    refresh();
}

void TimetableSimulator::refresh() {
    string oldDay = getCurrentDay();
    int oldSlot = getCurrentSlotNumber();

    buildSimulationSlots();

    currentIndex = -1;

    // Try to preserve the previous position after refreshing.
    if (!oldDay.empty()) {
        for (int i = 0; i < static_cast<int>(simulationSlots.size()); i++) {
            if (simulationSlots[i].day == oldDay &&
                simulationSlots[i].slot == oldSlot) {

                currentIndex = i;
                return;
            }
        }
    }

    // Start from the beginning if the old position no longer exists.
    if (!simulationSlots.empty()) {
        currentIndex = 0;
    }
}

void TimetableSimulator::buildSimulationSlots() {
    simulationSlots.clear();

    vector<string> days;

    // Find all days present in the timetable.
    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {

        bool alreadyExists = false;

        for (const string& day : days) {
            if (day == t->day) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            days.push_back(t->day);
        }
    }

    // Sort days in weekday order.
    stable_sort(
        days.begin(),
        days.end(),
        [](const string& a, const string& b) {
            return dayOrder(a) < dayOrder(b);
        }
    );

    // Find the last occupied slot.
    int lastSlot = slotCount;

    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {

        int endingSlot = t->startSlot + t->duration - 1;

        if (endingSlot > lastSlot) {
            lastSlot = endingSlot;
        }
    }

    // Generate every slot for every timetable day.
    for (const string& day : days) {
        for (int slot = 1; slot <= lastSlot; slot++) {
            simulationSlots.push_back({day, slot});
        }
    }
}

// --------------------------------------------------
// SIMULATION CONTROLS
// --------------------------------------------------

bool TimetableSimulator::next() {
    if (!hasCurrentSlot()) {
        return false;
    }

    if (currentIndex + 1 >=
        static_cast<int>(simulationSlots.size())) {
        return false;
    }

    currentIndex++;
    return true;
}

bool TimetableSimulator::previous() {
    if (!hasCurrentSlot()) {
        return false;
    }

    if (currentIndex <= 0) {
        return false;
    }

    currentIndex--;
    return true;
}

void TimetableSimulator::reset() {
    if (simulationSlots.empty()) {
        currentIndex = -1;
    }
    else {
        currentIndex = 0;
    }
}

// --------------------------------------------------
// CURRENT SIMULATION INFORMATION
// --------------------------------------------------

bool TimetableSimulator::hasCurrentSlot() const {
    return currentIndex >= 0 &&
           currentIndex <
           static_cast<int>(simulationSlots.size());
}

string TimetableSimulator::getCurrentDay() const {
    if (!hasCurrentSlot()) {
        return "";
    }

    return simulationSlots[currentIndex].day;
}

int TimetableSimulator::getCurrentSlotNumber() const {
    if (!hasCurrentSlot()) {
        return 0;
    }

    return simulationSlots[currentIndex].slot;
}

int TimetableSimulator::getCurrentIndex() const {
    return currentIndex;
}

int TimetableSimulator::getTotalSlots() const {
    return static_cast<int>(simulationSlots.size());
}

// --------------------------------------------------
// GET CLASSES AT CURRENT SLOT
// --------------------------------------------------

vector<TimetableEntry*>
TimetableSimulator::getCurrentEntries() const {

    if (!hasCurrentSlot()) {
        return {};
    }

    return getEntriesAt(
        getCurrentDay(),
        getCurrentSlotNumber()
    );
}

// --------------------------------------------------
// GET CLASSES AT A PARTICULAR SLOT
// --------------------------------------------------

vector<TimetableEntry*>
TimetableSimulator::getEntriesAt(
    const string& day,
    int slot
) const {

    vector<TimetableEntry*> result;

    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {

        if (t->day != day) {
            continue;
        }

        // Include classes that are currently running.
        // This also handles classes lasting two or more slots.
        int endingSlot = t->startSlot + t->duration - 1;

        if (slot >= t->startSlot && slot <= endingSlot) {
            result.push_back(t);
        }
    }

    return result;
}

// --------------------------------------------------
// GET SECTIONS ATTENDING A CLASS
// --------------------------------------------------

vector<string> getEntrySections(const TimetableEntry* entry) {
    vector<string> sections;

    if (entry == nullptr) {
        return sections;
    }

    // Check whether this entry refers to a declared merged group.
    MergedGroup* group =
        searchMergedGroupById(entry->sectionOrGroup);

    // Individual section.
    if (group == nullptr) {
        sections.push_back(trim(entry->sectionOrGroup));
        return sections;
    }

    // Merged group: section IDs are separated by semicolons.
    const string& ids = group->sectionIds;

    string current;

    for (size_t i = 0; i <= ids.size(); i++) {

        if (i == ids.size() || ids[i] == ';') {

            string sectionId = trim(current);

            if (!sectionId.empty()) {
                sections.push_back(sectionId);
            }

            current.clear();
        }
        else {
            current += ids[i];
        }
    }

    return sections;
}

// --------------------------------------------------
// CHECK WHETHER A SECTION ATTENDS A CLASS
// --------------------------------------------------

bool entryIncludesSection(
    const TimetableEntry* entry,
    const string& sectionId
) {

    vector<string> sections = getEntrySections(entry);

    for (const string& id : sections) {
        if (id == sectionId) {
            return true;
        }
    }

    return false;
}
