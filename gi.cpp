#include <iostream>

using namespace std;

bool facultyBusy[10] = {false};
bool sectionBusy[10] = {false};
bool venueBusy[10] = {false};

struct Entry {
    int faculty;
    int section;
    int venue;
    int slot;
};

void checkFaculty(Entry e, bool &clash) {
    if (facultyBusy[e.faculty]) {
        clash = true;
        cout << "Faculty Clash in Slot " << e.slot << endl;
    }
}

void checkSection(Entry e, bool &clash) {
    if (sectionBusy[e.section]) {
        clash = true;
        cout << "Section Clash in Slot " << e.slot << endl;
    }
}

void checkVenue(Entry e, bool &clash) {
    if (venueBusy[e.venue]) {
        clash = true;
        cout << "Venue Clash in Slot " << e.slot << endl;
    }
}

void checkEntry(Entry e) {
    bool facultyClash = false;
    bool sectionClash = false;
    bool venueClash = false;

    checkFaculty(e, facultyClash);
    checkSection(e, sectionClash);
    checkVenue(e, venueClash);

    if (!facultyClash && !sectionClash && !venueClash) {
        cout << "Entry is VALID" << endl;
        facultyBusy[e.faculty] = true;
        sectionBusy[e.section] = true;
        venueBusy[e.venue] = true;
    } else {
        cout << "Entry has CLASH" << endl;
    }

    cout << "-------------------------" << endl;
}   

int main() {
    Entry e1 = {1, 2, 3, 1};
    Entry e2 = {1, 4, 5, 1};

    checkEntry(e1);
    checkEntry(e2);

    return 0;
}