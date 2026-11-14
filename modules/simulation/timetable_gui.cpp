
#include "timetable_gui.h"

#include "../file_handling/person1_file_handling.h"
#include "../merged_classes/merged_class.h"
#include "../clash_detection/clashdetect.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

// --------------------------------------------------
// CONSTRUCTOR
// --------------------------------------------------
static int guiDayOrder(const string& day)
{
    if (day == "Monday") return 1;
    if (day == "Tuesday") return 2;
    if (day == "Wednesday") return 3;
    if (day == "Thursday") return 4;
    if (day == "Friday") return 5;
    if (day == "Saturday") return 6;
    if (day == "Sunday") return 7;

    return 8;
}
TimetableGUI::TimetableGUI()
    : window(
        sf::VideoMode({1280u, 800u}),
        "Timetable Optimiser and Simulator"
    ),
      currentPage(Page::Dashboard) {

    window.setFramerateLimit(60);
    

    loadFont();

    simulator.initialize();
}

// --------------------------------------------------
// FONT LOADING
// --------------------------------------------------

bool TimetableGUI::loadFont() {

    // First try the default Windows Arial font.
    if (font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        return true;
    }

    // Alternative: a font stored in the project.
    if (font.openFromFile("assets/arial.ttf")) {
        return true;
    }

    cerr << "Font could not be loaded!\n";
    cerr << "Copy arial.ttf into the assets folder.\n";

    return false;
}

// --------------------------------------------------
// DRAWING HELPERS
// --------------------------------------------------

void TimetableGUI::drawText(
    const string& value,
    float x,
    float y,
    unsigned int size,
    sf::Color color
) {

    sf::Text text(font, value, size);

    text.setFillColor(color);
    text.setPosition({x, y});

    window.draw(text);
}

void TimetableGUI::drawRectangle(
    float x,
    float y,
    float width,
    float height,
    sf::Color color
) {

    sf::RectangleShape rectangle({width, height});

    rectangle.setPosition({x, y});
    rectangle.setFillColor(color);

    window.draw(rectangle);
}

void TimetableGUI::drawButton(
    const string& label,
    float x,
    float y,
    float width,
    float height,
    sf::Color color
) {

    drawRectangle(x, y, width, height, color);

    drawText(
        label,
        x + 15,
        y + height / 2 - 12,
        17,
        sf::Color::White
    );
}

// --------------------------------------------------
// COUNTING HELPERS
// --------------------------------------------------

int TimetableGUI::countSections() const {

    int count = 0;

    for (Section* s = sectionHead; s != nullptr; s = s->next) {
        count++;
    }

    return count;
}

int TimetableGUI::countFaculty() const {

    int count = 0;

    for (Faculty* f = facultyHead; f != nullptr; f = f->next) {
        count++;
    }

    return count;
}

int TimetableGUI::countVenues() const {

    int count = 0;

    for (Venue* v = venueHead; v != nullptr; v = v->next) {
        count++;
    }

    return count;
}

int TimetableGUI::countClasses() const {

    int count = 0;

    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {
        count++;
    }

    return count;
}

int TimetableGUI::countClashes() const {
    return static_cast<int>(getClashes().size());
}

// --------------------------------------------------
// ENTRY DISPLAY NAME
// --------------------------------------------------

string TimetableGUI::getEntryName(
    const TimetableEntry* entry
) const {

    if (entry == nullptr) {
        return "Unknown class";
    }

    string result = entry->subject + " | ";

    vector<string> sections = getEntrySections(entry);

    for (size_t i = 0; i < sections.size(); i++) {

        result += sections[i];

        if (i + 1 < sections.size()) {
            result += ", ";
        }
    }

    return result;
}

// --------------------------------------------------
// SIDEBAR
// --------------------------------------------------

void TimetableGUI::drawSidebar() {

    drawRectangle(
        0, 0,
        SIDEBAR_WIDTH,
        WINDOW_HEIGHT,
        sf::Color(25, 30, 47)
    );

    drawText(
        "TIMEWISE",
        30, 35,
        27,
        sf::Color(90, 190, 255)
    );

    drawText(
        "Timetable Manager",
        30, 75,
        13,
        sf::Color(170, 180, 200)
    );

    struct MenuItem {
        string name;
        Page page;
    };

    MenuItem items[] = {
        {"Dashboard", Page::Dashboard},
        {"Timetable", Page::Timetable},
        {"Clash Report", Page::Clashes},
        {"Merged Classes", Page::MergedClasses},
        {"Optimisation", Page::Optimisation},
        {"Simulation", Page::Simulation}
    };

    float y = 145;

    for (const MenuItem& item : items) {

        bool selected = (currentPage == item.page);

        if (selected) {
            drawRectangle(
                15, y - 8,
                200, 48,
                sf::Color(49, 85, 135)
            );
        }

        drawText(
            item.name,
            35, y,
            17,
            selected
                ? sf::Color::White
                : sf::Color(180, 190, 210)
        );

        y += 65;
    }

    drawText(
        "PERSON 4 | GUI",
        25, 745,
        13,
        sf::Color(130, 145, 165)
    );
}

// --------------------------------------------------
// HEADER
// --------------------------------------------------

void TimetableGUI::drawHeader() {

    drawRectangle(
        SIDEBAR_WIDTH, 0,
        WINDOW_WIDTH - SIDEBAR_WIDTH,
        85,
        sf::Color(20, 25, 40)
    );

    drawText(
        "Timetable Optimiser and Simulator",
        265, 27,
        23,
        sf::Color::White
    );

    drawText(
        "University Scheduling System",
        1000, 33,
        13,
        sf::Color(150, 165, 185)
    );
}

// --------------------------------------------------
// DASHBOARD
// --------------------------------------------------

void TimetableGUI::drawDashboard() {

    drawText(
        "Dashboard",
        270, 115,
        30
    );

    drawText(
        "Overview of your current timetable",
        270, 155,
        15,
        sf::Color(155, 170, 190)
    );

    struct DashboardCard {
        string title;
        int value;
    };

    DashboardCard cards[] = {
        {"Total Classes", countClasses()},
        {"Sections", countSections()},
        {"Faculty", countFaculty()},
        {"Venues", countVenues()},
        {"Total Clashes", countClashes()}
    };

    float x = 270;
    float y = 210;

    for (int i = 0; i < 5; i++) {

        drawRectangle(
            x, y,
            175, 125,
            sf::Color(34, 43, 63)
        );

        drawText(
            cards[i].title,
            x + 15, y + 18,
            14,
            sf::Color(175, 190, 210)
        );

        sf::Color valueColor =
            (i == 4 && cards[i].value > 0)
            ? sf::Color(255, 100, 100)
            : sf::Color(90, 200, 155);

        drawText(
            to_string(cards[i].value),
            x + 15, y + 55,
            30,
            valueColor
        );

        x += 190;
    }

    drawText(
        "System Status",
        270, 390,
        23
    );

    drawRectangle(
        270, 435,
        920, 105,
        sf::Color(34, 43, 63)
    );

    if (countClasses() == 0) {

        drawText(
            "No timetable data loaded.",
            295, 475,
            18,
            sf::Color(255, 190, 90)
        );
    }
    else if (countClashes() == 0) {

        drawText(
            "No clashes currently detected.",
            295, 475,
            18,
            sf::Color(90, 210, 150)
        );
    }
    else {

        drawText(
            to_string(countClashes()) +
            " clash records detected. Open Clash Report to inspect them.",
            295, 475,
            17,
            sf::Color(255, 115, 115)
        );
    }

    drawText(
        "Use the sidebar to explore the timetable and simulation.",
        270, 590,
        16,
        sf::Color(150, 165, 185)
    );
}

// --------------------------------------------------
// WEEKLY TIMETABLE
// --------------------------------------------------



void TimetableGUI::drawTimetable() {

    // Load section IDs once.
    // Load only main section IDs, not groups or electives.
if (timetableSections.empty()) {
    for (Section* s = sectionHead;
         s != nullptr;
         s = s->next) {

        if (s->name.find(" - Section ") != string::npos) {
            timetableSections.push_back(s->id);
        }
    }
}

    // Check that sections are available.
    if (timetableSections.empty()) {
        drawText("No sections loaded!", 400, 300, 22);
        return;
    }

    // Keep selected index within valid range.
    if (selectedSectionIndex < 0 ||
        selectedSectionIndex >= static_cast<int>(timetableSections.size())) {
        selectedSectionIndex = 0;
    }

    // Heading.
    drawText(
        "Weekly Timetable - Section " +
        timetableSections[selectedSectionIndex],
        270, 110, 25
    );

    drawText(
        "Green = Clash-free    Red = Clash detected",
        270, 150, 14,
        sf::Color(170, 185, 200)
    );

    // Section navigation.
    drawButton(
        "PREVIOUS",
        270, 170, 110, 32,
        selectedSectionIndex > 0
            ? sf::Color(65, 80, 110)
            : sf::Color(65, 70, 80)
    );

    drawButton(
        "NEXT",
        390, 170, 110, 32,
        selectedSectionIndex <
            static_cast<int>(timetableSections.size()) - 1
            ? sf::Color(49, 110, 160)
            : sf::Color(65, 70, 80)
    );

    // Show selected section name.
    Section* selected = searchSectionById(
        timetableSections[selectedSectionIndex]
    );

    string sectionLabel =
        "Viewing: " + timetableSections[selectedSectionIndex];

    if (selected != nullptr) {
        sectionLabel += " - " + selected->name;
    }

    drawText(
        sectionLabel,
        530, 178, 15,
        sf::Color(100, 190, 255)
    );

    const string days[] = {
        "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };

    const float startX = 330;
    const float startY = 215;
    const float cellWidth = 140;
    const float cellHeight = 55;

    int slots = slotCount;

    if (slots <= 0) {
        slots = 8;
    }

    // Find the largest ending slot.
    for (TimetableEntry* t = timetableHead;
         t != nullptr;
         t = t->next) {

        int ending = t->startSlot + t->duration - 1;

        if (ending > slots) {
            slots = ending;
        }
    }

    slots = min(slots, 10);

    // Draw day headings.
    for (int d = 0; d < 6; d++) {

        drawRectangle(
            startX + d * cellWidth,
            startY,
            cellWidth - 3,
            35,
            sf::Color(49, 85, 135)
        );

        drawText(
            days[d],
            startX + d * cellWidth + 5,
            startY + 8,
            12
        );
    }

    // Draw slot labels and empty cells.
    for (int s = 1; s <= slots; s++) {

        drawText(
            "Slot " + to_string(s),
            255,
            startY + 35 + (s - 1) * cellHeight + 17,
            12,
            sf::Color(170, 185, 205)
        );

        for (int d = 0; d < 6; d++) {

            drawRectangle(
                startX + d * cellWidth,
                startY + 35 + (s - 1) * cellHeight,
                cellWidth - 3,
                cellHeight - 3,
                sf::Color(34, 43, 63)
            );
        }
    }

    // Merged classes appear in every participating section.
string selectedSection =
    timetableSections[selectedSectionIndex];

// Draw only entries belonging to the selected section.
for (TimetableEntry* t = timetableHead;
     t != nullptr;
     t = t->next) {

    bool belongsToSection =
        (t->sectionOrGroup == selectedSection) ||
        (t->sectionOrGroup.rfind(selectedSection + "-", 0) == 0) ||
        entryIncludesSection(t, selectedSection);

    if (!belongsToSection) {
        continue;
    }

        int day = guiDayOrder(t->day) - 1;

        if (day < 0 || day >= 6 ||
            t->startSlot < 1 ||
            t->startSlot > slots) {
            continue;
        }

        int visibleDuration = min(
            t->duration,
            slots - t->startSlot + 1
        );

        float x = startX + day * cellWidth + 3;

        float y = startY + 35 +
                  (t->startSlot - 1) * cellHeight + 3;

        float height = visibleDuration * cellHeight - 6;

        sf::Color blockColor = hasClash(t)
            ? sf::Color(160, 58, 65)
            : sf::Color(40, 120, 90);

        drawRectangle(
            x, y,
            cellWidth - 9,
            height,
            blockColor
        );

        drawText(
            t->subject,
            x + 5, y + 5,
            12
        );

        drawText(
            t->sectionOrGroup,
            x + 5, y + 24,
            10,
            sf::Color(235, 240, 245)
        );
    }
}

// --------------------------------------------------
// CLASH REPORT
// --------------------------------------------------


void TimetableGUI::drawClashReport() {

    drawText("Clash Report", 270, 110, 29);

    const vector<ClashInfo>& clashes = getClashes();

    drawText(
        "Total clashes: " + to_string(clashes.size()),
        270, 155, 16,
        sf::Color(255, 120, 120)
    );

    float y = 205;

    if (clashes.empty()) {
        drawRectangle(
            270, y, 900, 90,
            sf::Color(34, 70, 57)
        );

        drawText(
            "No clashes detected!",
            295, y + 30, 20,
            sf::Color(100, 220, 160)
        );

        return;
    }

    int shown = 0;

    for (const ClashInfo& clash : clashes) {

        if (shown >= 6)
            break;

        drawRectangle(
            270, y, 900, 100,
            sf::Color(55, 37, 48)
        );

        drawText(
            clash.kind + " CLASH",
            290, y + 7, 14,
            sf::Color(255, 115, 115)
        );

        drawText(
            clash.day + " | Slot " + to_string(clash.slot),
            480, y + 7, 13,
            sf::Color(210, 220, 235)
        );

        if (clash.a && clash.b) {

            drawText(
                "Entry 1: " + clash.a->sectionOrGroup +
                " - " + clash.a->subject,
                290, y + 30, 13
            );

            drawText(
                "Entry 2: " + clash.b->sectionOrGroup +
                " - " + clash.b->subject,
                290, y + 50, 13
            );
        }

        drawText(
            clash.text,
            290, y + 73, 12,
            sf::Color(225, 225, 235)
        );

        y += 112;
        shown++;
    }

    if (clashes.size() > 6) {
        drawText(
            "Showing first 6 of " +
            to_string(clashes.size()) + " clashes.",
            270, y + 5, 13,
            sf::Color(160, 175, 195)
        );
    }
}

// --------------------------------------------------
// MERGED CLASSES
// --------------------------------------------------


void TimetableGUI::drawMergedClasses() {

    drawText("Merged Classes", 270, 110, 29);

    drawText(
        "Groups detected using Graph and DFS",
        270, 150, 14,
        sf::Color(160, 180, 200)
    );

    // Store information about every detected group.
    struct MergedGroupDisplay {
        string day;
        string subject;
        string sectionNames;
        string reason;
        int slot;
        int strength;
        bool valid;
    };

    vector<MergedGroupDisplay> groups;

    const string days[] = {
        "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };

    int invalidGroups = 0;

    // Collect ALL detected groups first.
    for (const string& day : days) {

        int maxSlot = slotCount;

        for (TimetableEntry* t = timetableHead;
             t != nullptr;
             t = t->next) {

            if (t->day == day) {

                int ending = t->startSlot + t->duration - 1;

                if (ending > maxSlot) {
                    maxSlot = ending;
                }
            }
        }

        for (int slot = 1; slot <= maxSlot; slot++) {

            SlotEntry entries[MAX_ENTRIES];

            int n = collectSlotEntries(
                day, slot, entries, MAX_ENTRIES
            );

            MergeDetector detector(entries, n);
            detector.detect();

            for (const DetectedGroup* g = detector.getFirstGroup();
                 g != nullptr;
                 g = g->getNext()) {

                MergedGroupDisplay item;

                item.day = day;
                item.slot = slot;
                item.valid = g->isValid();
                item.strength = g->getStrength();
                item.reason = g->getReason();

                const SlotEntry& first =
                    detector.getEntry(g->getMember(0));

                item.subject = first.subject;

                for (int i = 0; i < g->getCount(); i++) {

                    item.sectionNames +=
                        detector.getEntry(g->getMember(i)).section;

                    if (i + 1 < g->getCount()) {
                        item.sectionNames += ", ";
                    }
                }

                if (!item.valid) {
                    invalidGroups++;
                }

                groups.push_back(item);
            }
        }
    }

    // Pagination settings.
    const int GROUPS_PER_PAGE = 5;

    int totalGroups = static_cast<int>(groups.size());

    int totalPages = (totalGroups + GROUPS_PER_PAGE - 1)
                     / GROUPS_PER_PAGE;

    if (totalPages == 0) {
        totalPages = 1;
    }

    // Keep the selected page within valid limits.
    if (mergedPage >= totalPages) {
        mergedPage = totalPages - 1;
    }

    if (mergedPage < 0) {
        mergedPage = 0;
    }

    int startIndex = mergedPage * GROUPS_PER_PAGE;
    int endIndex = min(startIndex + GROUPS_PER_PAGE, totalGroups);

    if (totalGroups == 0) {

        drawText(
            "No merged classes detected.",
            290, 220, 18,
            sf::Color(180, 190, 210)
        );
    }

    // Display only the groups belonging to the current page.
    float y = 195;

    for (int i = startIndex; i < endIndex; i++) {

        const MergedGroupDisplay& g = groups[i];

        sf::Color background = g.valid
            ? sf::Color(35, 75, 60)
            : sf::Color(90, 45, 50);

        drawRectangle(
            270, y, 900, 75,
            background
        );

        drawText(
            g.subject + " | " + g.sectionNames,
            290, y + 8, 15
        );

        drawText(
            g.day + " | Slot " + to_string(g.slot) +
            " | Strength: " + to_string(g.strength),
            290, y + 35, 12,
            sf::Color(205, 215, 230)
        );

        string status = g.valid
            ? "VALID"
            : "INVALID: " + g.reason;

        drawText(
            status,
            900, y + 22, 13,
            g.valid
                ? sf::Color(100, 230, 160)
                : sf::Color(255, 130, 130)
        );

        y += 85;
    }

    // Overall statistics.
    drawText(
        "Total groups: " + to_string(totalGroups) +
        " | Invalid: " + to_string(invalidGroups),
        270, 635, 15,
        sf::Color(180, 195, 215)
    );

    // Pagination buttons.
    drawButton(
        "PREVIOUS",
        650, 690, 150, 45,
        mergedPage > 0
            ? sf::Color(49, 110, 160)
            : sf::Color(65, 70, 80)
    );

    drawText(
        "Page " + to_string(mergedPage + 1) +
        " / " + to_string(totalPages),
        820, 703, 15,
        sf::Color(210, 220, 235)
    );

    drawButton(
        "NEXT",
        930, 690, 150, 45,
        mergedPage + 1 < totalPages
            ? sf::Color(49, 110, 160)
            : sf::Color(65, 70, 80)
    );
}


// --------------------------------------------------
// OPTIMISATION PAGE
// --------------------------------------------------

void TimetableGUI::drawOptimisation() {

    drawText(
        "Timetable Optimisation",
        270, 110,
        29
    );

    drawText(
        "Resolve detected timetable clashes",
        270, 155,
        15,
        sf::Color(165, 180, 200)
    );

    drawRectangle(
        270, 220,
        900, 140,
        sf::Color(34, 43, 63)
    );

    drawText(
        "Current Clash Count",
        300, 245,
        17,
        sf::Color(180, 195, 215)
    );

    drawText(
        to_string(countClashes()),
        300, 285,
        35,
        countClashes() > 0
            ? sf::Color(255, 110, 110)
            : sf::Color(100, 220, 160)
    );

    drawRectangle(
        270, 410,
        900, 130,
        sf::Color(34, 43, 63)
    );

    drawText(
        "Optimisation uses Person 3's algorithm.",
        300, 435,
        17
    );

    drawText(
        "It may modify class times or venues to resolve clashes.",
        300, 475,
        14,
        sf::Color(170, 185, 205)
    );

    drawButton(
        "RUN OPTIMISATION",
        300, 580,
        230, 50,
        sf::Color(49, 110, 160)
    );

    drawText(
        "Click the button to run the optimisation algorithm.",
        300, 650,
        13,
        sf::Color(155, 175, 195)
    );
}

// --------------------------------------------------
// SIMULATION PAGE
// --------------------------------------------------


void TimetableGUI::drawSimulation() {

    drawText("Live Timetable Simulation", 270, 110, 29);

    drawText(
        "Move through the timetable one slot at a time.",
        270, 150, 14,
        sf::Color(160, 180, 200)
    );

    if (!simulator.hasCurrentSlot()) {

        drawText(
            "No simulation data available.",
            290, 240, 20,
            sf::Color(255, 190, 100)
        );

        return;
    }

    drawRectangle(
        270, 205, 900, 110,
        sf::Color(34, 43, 63)
    );

    drawText(
        simulator.getCurrentDay(),
        300, 225, 23,
        sf::Color(100, 190, 255)
    );

    drawText(
        "Slot " + to_string(simulator.getCurrentSlotNumber()),
        300, 265, 19
    );

    vector<TimetableEntry*> entries =
        simulator.getCurrentEntries();

    int totalEntries = static_cast<int>(entries.size());

    drawText(
        "Classes currently running: " + to_string(totalEntries),
        270, 345, 19
    );

    const int ENTRIES_PER_PAGE = 3;

    int totalPages =
        (totalEntries + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;

    if (totalPages == 0) {
        totalPages = 1;
    }

    if (simulationEntriesPage >= totalPages) {
        simulationEntriesPage = totalPages - 1;
    }

    if (simulationEntriesPage < 0) {
        simulationEntriesPage = 0;
    }

    int startIndex = simulationEntriesPage * ENTRIES_PER_PAGE;

    int endIndex = startIndex + ENTRIES_PER_PAGE;

    if (endIndex > totalEntries) {
        endIndex = totalEntries;
    }

    float y = 395;

    if (entries.empty()) {

        drawText(
            "No classes scheduled during this slot.",
            290, y, 17,
            sf::Color(170, 185, 205)
        );
    }

    for (int i = startIndex; i < endIndex; i++) {

        TimetableEntry* t = entries[i];

        bool clash = hasClash(t);

        drawRectangle(
            270, y, 900, 65,
            clash
                ? sf::Color(110, 48, 55)
                : sf::Color(35, 85, 65)
        );

        drawText(
            getEntryName(t),
            290, y + 8, 16
        );

        drawText(
            "Faculty: " + t->facultyId +
            " | Venue: " + t->venueId,
            290, y + 35, 13,
            sf::Color(210, 220, 230)
        );

        drawText(
            clash ? "CLASH" : "CLEAR",
            1030, y + 22, 14,
            clash
                ? sf::Color(255, 125, 125)
                : sf::Color(110, 230, 165)
        );

        y += 85;
    }

    // Class-list pagination.
    drawButton(
        "PREVIOUS",
        760, 635, 130, 40,
        simulationEntriesPage > 0
            ? sf::Color(49, 110, 160)
            : sf::Color(65, 70, 80)
    );

    drawText(
        "Page " + to_string(simulationEntriesPage + 1) +
        " / " + to_string(totalPages),
        900, 647, 14,
        sf::Color(210, 220, 235)
    );

    drawButton(
        "NEXT",
        1000, 635, 130, 40,
        simulationEntriesPage + 1 < totalPages
            ? sf::Color(49, 110, 160)
            : sf::Color(65, 70, 80)
    );

    // Slot navigation controls.
    drawButton(
        "PREVIOUS",
        270, 695, 140, 45,
        sf::Color(65, 80, 110)
    );

    drawButton(
        "NEXT",
        430, 695, 140, 45,
        sf::Color(49, 110, 160)
    );

    drawButton(
        "RESET",
        590, 695, 140, 45,
        sf::Color(100, 80, 125)
    );

    drawText(
        to_string(simulator.getCurrentIndex() + 1) +
        " / " + to_string(simulator.getTotalSlots()),
        770, 708, 15,
        sf::Color(190, 205, 220)
    );
}


// --------------------------------------------------
// EVENT HANDLING
// --------------------------------------------------

void TimetableGUI::handleEvents() {

    while (const auto event = window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        else if (const auto* mouse =
                     event->getIf<sf::Event::MouseButtonPressed>()) {

            if (mouse->button == sf::Mouse::Button::Left) {

                sf::Vector2i pixelPosition = sf::Mouse::getPosition(window);
sf::Vector2f worldPosition =
    window.mapPixelToCoords(pixelPosition);

handleClick(worldPosition.x, worldPosition.y);
            }
        }
    }
}

// --------------------------------------------------
// MOUSE CLICK HANDLING
// --------------------------------------------------


void TimetableGUI::handleClick(float x, float y) {

    // Sidebar navigation.
    if (x >= 0 && x <= SIDEBAR_WIDTH) {

        if (y >= 137 && y < 190) {
            currentPage = Page::Dashboard;
        }
        else if (y >= 202 && y < 255) {
            currentPage = Page::Timetable;
        }
        else if (y >= 267 && y < 320) {
            currentPage = Page::Clashes;
        }
        else if (y >= 332 && y < 385) {
            currentPage = Page::MergedClasses;
        }
        else if (y >= 397 && y < 450) {
            currentPage = Page::Optimisation;
        }
        else if (y >= 462 && y < 515) {
            currentPage = Page::Simulation;
        }

        return;
    }

    // Timetable section navigation.
    if (currentPage == Page::Timetable) {

        if (y >= 170 && y <= 205) {

            if (x >= 270 && x <= 380) {
                // Previous section.
                if (selectedSectionIndex > 0) {
                    selectedSectionIndex--;
                }
            }

            else if (x >= 390 && x <= 500) {
                // Next section.
                if (selectedSectionIndex <
                    static_cast<int>(timetableSections.size()) - 1) {
                    selectedSectionIndex++;
                }
            }

            return;
        }
    }

    // Optimisation button.
    if (currentPage == Page::Optimisation) {

        if (x >= 300 && x <= 530 &&
            y >= 580 && y <= 630) {

            optimise();

            // Refresh clash information after optimisation.
            runClashCheck();

            // Rebuild simulation.
            simulator.refresh();
        }
    }

    // Merged Classes pagination.
    if (currentPage == Page::MergedClasses) {

        if (y >= 690 && y <= 740) {

            if (x >= 650 && x <= 800) {
                if (mergedPage > 0) {
                    mergedPage--;
                }
            }

            else if (x >= 930 && x <= 1080) {
                mergedPage++;
            }
        }
    }

    // Simulation class-list pagination.
    if (currentPage == Page::Simulation) {

        if (y >= 635 && y <= 675) {

            if (x >= 760 && x <= 890) {
                if (simulationEntriesPage > 0) {
                    simulationEntriesPage--;
                }
            }

            else if (x >= 1000 && x <= 1130) {
                simulationEntriesPage++;
            }

            return;
        }
    }

    // Simulation slot controls.
    if (currentPage == Page::Simulation) {

        if (y >= 695 && y <= 740) {

            if (x >= 270 && x <= 410) {
                simulator.previous();
                simulationEntriesPage = 0;
            }

            else if (x >= 430 && x <= 570) {
                simulator.next();
                simulationEntriesPage = 0;
            }

            else if (x >= 590 && x <= 730) {
                simulator.reset();
                simulationEntriesPage = 0;
            }
        }
    }
}

// --------------------------------------------------
// MAIN GUI LOOP
// --------------------------------------------------

void TimetableGUI::run() {

    if (!font.getInfo().family.empty()) {
        // Font successfully loaded.
    }
    else {
        cerr << "Cannot start GUI without a font.\n";
        return;
    }

    while (window.isOpen()) {

        handleEvents();

        window.clear(sf::Color(17, 21, 34));

        drawSidebar();
        drawHeader();

        switch (currentPage) {

            case Page::Dashboard:
                drawDashboard();
                break;

            case Page::Timetable:
                drawTimetable();
                break;

            case Page::Clashes:
                drawClashReport();
                break;

            case Page::MergedClasses:
                drawMergedClasses();
                break;

            case Page::Optimisation:
                drawOptimisation();
                break;

            case Page::Simulation:
                drawSimulation();
                break;
        }

        window.display();
    }
}
