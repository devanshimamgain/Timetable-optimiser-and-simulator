
#ifndef TIMETABLE_GUI_H
#define TIMETABLE_GUI_H

#include <SFML/Graphics.hpp>
#include <string>
#include "person4_simulator.h"

using namespace std;

class TimetableGUI {
private:
    sf::RenderWindow window;
    sf::Font font;

    TimetableSimulator simulator;

    enum class Page {
        Dashboard,
        Timetable,
        Clashes,
        MergedClasses,
        Optimisation,
        Simulation
    };

    Page currentPage;
    int mergedPage = 0;
    int simulationEntriesPage = 0;
    vector<string> timetableSections;
int selectedSectionIndex = 0;
    // Timetable navigation
bool showAllSections = false;

    // Window dimensions
    const unsigned int WINDOW_WIDTH = 1280;
    const unsigned int WINDOW_HEIGHT = 800;
    const float SIDEBAR_WIDTH = 230.0f;

    // Load fonts and initialise the GUI.
    bool loadFont();

    // Drawing helpers
    void drawText(
        const string& value,
        float x,
        float y,
        unsigned int size,
        sf::Color color = sf::Color::White
    );

    void drawRectangle(
        float x,
        float y,
        float width,
        float height,
        sf::Color color
    );

    void drawButton(
        const string& label,
        float x,
        float y,
        float width,
        float height,
        sf::Color color
    );

    // Main interface
    void drawSidebar();
    void drawHeader();
    void drawDashboard();
    void drawTimetable();
    void drawClashReport();
    void drawMergedClasses();
    void drawOptimisation();
    void drawSimulation();

    // Event handling
    void handleEvents();
    void handleClick(float x, float y);

    // Common helpers
    int countSections() const;
    int countFaculty() const;
    int countVenues() const;
    int countClasses() const;
    int countClashes() const;

    string getEntryName(const TimetableEntry* entry) const;

public:
    TimetableGUI();

    // Start the GUI.
    void run();
};

#endif
