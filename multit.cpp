#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

bool facultyBusy[10] = {false};
bool sectionBusy[10] = {false};
bool venueBusy[10] = {false};

mutex clashMutex;

struct Entry
{
    int faculty;
    int section;
    int venue;
    int slot;
};

void checkFaculty(Entry e, bool &clash)
{
    if (facultyBusy[e.faculty])
    {
        clash = true;

        clashMutex.lock();
        cout << "Faculty Clash in Slot " << e.slot << endl;
        clashMutex.unlock();
    }
}

void checkSection(Entry e, bool &clash)
{
    if (sectionBusy[e.section])
    {
        clash = true;

        clashMutex.lock();
        cout << "Section Clash in Slot " << e.slot << endl;
        clashMutex.unlock();
    }
}

void checkVenue(Entry e, bool &clash)
{
    if (venueBusy[e.venue])
    {
        clash = true;

        clashMutex.lock();
        cout << "Venue Clash in Slot " << e.slot << endl;
        clashMutex.unlock();
    }
}

void checkEntry(Entry e)
{
    bool facultyClash = false;
    bool sectionClash = false;
    bool venueClash = false;

    thread t1(checkFaculty, e, ref(facultyClash));
    thread t2(checkSection, e, ref(sectionClash));
    thread t3(checkVenue, e, ref(venueClash));

    t1.join();
    t2.join();
    t3.join();

    if (!facultyClash && !sectionClash && !venueClash)
    {
        cout << "Entry is VALID" << endl;

        facultyBusy[e.faculty] = true;
        sectionBusy[e.section] = true;
        venueBusy[e.venue] = true;
    }
    else
    {
        cout << "Entry has CLASH" << endl;
    }

    cout << "----------------------" << endl;
}

int main()
{
    Entry entries[4] =
    {
        {0, 0, 0, 1},   // A, Dr X, LT-1
        {1, 1, 1, 1},   // B, Dr Y, LT-2
        {0, 2, 2, 1},   // C, Dr X, LT-3 -> Faculty clash
        {2, 3, 0, 1}    // D, Dr Z, LT-1 -> Venue clash
    };

    for (int i = 0; i < 4; i++)
    {
        cout << "Checking Entry " << i + 1 << endl;
        checkEntry(entries[i]);
    }

    return 0;
}