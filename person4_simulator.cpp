// Person 4: timetable table, simulator and the menu (console, no graphics library)
//
// Uses Person 1 (loading the CSV files), and Person 3 (clash check, spoil, optimise) from multit.cpp.
//
// Compile: g++ -std=c++17 -Wall -Wextra person4_simulator.cpp -o simulator.exe
// Run (in the folder that has the CSV files): ./simulator.exe
//
// Menu:
//   1 show the weekly table of one section (! = class with a clash)
//   2 check clashes
//   3 spoil the timetable (demo)
//   4 optimise (fix the clashes)
//   5 simulate slot by slot (n = next, p = previous, r = reset, q = back to menu)
//   0 exit

#define MULTIT_NO_MAIN            // multit.cpp has its own main(); this file has its own
#include "multit.cpp"

#include <iomanip>
#include <iostream>
#include <string>

// ---------------------------------------------------------------- queue of slots (linked list)
struct SlotPos { int dayIdx; int slot; };

class SlotQueue {
    struct Node { SlotPos value; Node* next; };
    Node *front, *rear;
    int count;
public:
    SlotQueue() : front(NULL), rear(NULL), count(0) {}
    ~SlotQueue() { clear(); }
    SlotQueue(const SlotQueue&) = delete;
    SlotQueue& operator=(const SlotQueue&) = delete;
    void enqueue(SlotPos v) {                 // add at the back
        Node* n = new Node; n->value = v; n->next = NULL;
        if (rear) rear->next = n; else front = n;
        rear = n;
        count++;
    }
    bool dequeue(SlotPos& v) {                // take from the front
        if (!front) return false;
        Node* n = front;
        v = n->value;
        front = n->next;
        if (!front) rear = NULL;
        delete n;
        count--;
        return true;
    }
    bool empty() const { return front == NULL; }
    void clear() { SlotPos t; while (dequeue(t)) {} }
};

// The occupied slots in time order, filled by buildSimulation().
const int MAX_SIM_SLOTS = 500;
SlotPos simSlots[MAX_SIM_SLOTS];
int simCount = 0;

// Does class t run in slot number `slot` of day dayIdx?
bool runsAt(const TimetableEntry* t, int dayIdx, int slot) {
    return t->day == dayList[dayIdx] && slot >= t->startSlot && slot <= t->startSlot + t->duration - 1;
}

// Puts every slot that has at least one class into the queue, day by day, and then into simSlots.
void buildSimulation() {
    SlotQueue q;
    int last = lastSlotNumber();
    for (int d = 0; d < dayListCount; d++)
        for (int s = 1; s <= last; s++) {
            bool busy = false;
            for (TimetableEntry* t = timetableHead; t != nullptr && !busy; t = t->next)
                if (runsAt(t, d, s)) busy = true;
            if (busy) { SlotPos p; p.dayIdx = d; p.slot = s; q.enqueue(p); }
        }
    simCount = 0;
    SlotPos p;
    while (q.dequeue(p) && simCount < MAX_SIM_SLOTS) simSlots[simCount++] = p;
}

// Prints one slot of the simulation: its classes, and the reason for each clash.
void showSimSlot(int i) {
    SlotPos p = simSlots[i];
    cout << "\n===== SLOT " << (i + 1) << " OF " << simCount << " | " << dayList[p.dayIdx]
         << " | slot " << p.slot << " =====\n";
    int shown = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        if (!runsAt(t, p.dayIdx, p.slot)) continue;
        shown++;
        bool bad = hasClash(t);
        cout << (bad ? "[CLASH] " : "[OK]    ") << t->subject << " | " << t->sectionOrGroup << " | "
             << t->facultyId << " | " << t->venueId << " | " << t->duration << " slot(s)\n";
        if (bad) {
            for (int c = 0; c < clashCount; c++)
                if (clashes[c].a == t || clashes[c].b == t)
                    cout << "         reason: [" << clashes[c].kind << "] " << clashes[c].text << "\n";
        }
    }
    if (shown == 0) cout << "(no class in this slot)\n";
}

// ---------------------------------------------------------------- table of one section
// Shows the week of one section. Columns are slots, rows are days.
// A cell shows the class that starts there ("!" = clash), and "^" for the rest of a long class.
void showTable(const string& sectionId) {
    int last = lastSlotNumber();
    const int W = 18;
    cout << "\nTIMETABLE OF SECTION " << sectionId << "   (! = class with a clash)\n";
    cout << left << setw(12) << "Day";
    for (int s = 1; s <= last; s++) {
        string head = "Slot " + to_string(s);
        cout << left << setw(W) << head;
    }
    cout << "\n";
    cout << string(12 + W * last, '-') << "\n";

    for (int d = 0; d < dayListCount; d++) {
        cout << left << setw(12) << dayList[d];
        for (int s = 1; s <= last; s++) {
            string cell = "";
            for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
                if (!runsAt(t, d, s) || !attends(t, sectionId)) continue;
                if (t->startSlot == s) {
                    string subj = t->subject;
                    if (subj.size() > 11) subj = subj.substr(0, 11);
                    cell = (hasClash(t) ? "!" : "") + subj + "@" + t->venueId;
                } else {
                    cell = "^";
                }
                break;
            }
            if (cell.size() >= (size_t)W) cell = cell.substr(0, W - 1);
            cout << left << setw(W) << cell;
        }
        cout << "\n";
    }
}

// Lists the section ids so that the user can choose one.
void listSections() {
    cout << "Sections: ";
    for (Section* s = sectionHead; s != nullptr; s = s->next) cout << s->id << " ";
    cout << "\n";
}

// Reads a line from the user; returns "" at the end of input.
string askLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) return "";
    return trim(s) == "" ? "" : s;
}

// Simulation mode: n / p / r / q.
void runSimulation() {
    buildSimulation();
    if (simCount == 0) { cout << "No occupied slots to simulate.\n"; return; }
    int current = 0;
    showSimSlot(current);
    while (true) {
        string cmd = askLine("[n] next  [p] previous  [r] reset  [q] back to menu: ");
        if (cmd.empty()) break;
        char c = (char)tolower((unsigned char)cmd[0]);
        if (c == 'n') {
            if (current < simCount - 1) { current++; showSimSlot(current); }
            else cout << "This was the last slot.\n";
        } else if (c == 'p') {
            if (current > 0) { current--; showSimSlot(current); }
            else cout << "Already at the first slot.\n";
        } else if (c == 'r') {
            current = 0;
            showSimSlot(current);
        } else if (c == 'q') {
            break;
        } else {
            cout << "Use n, p, r or q.\n";
        }
    }
}

int main() {
    loadAllCSVFiles();
    if (timetableHead == nullptr) {
        cerr << "Could not load timetable.csv from this folder. Run the program where the CSV files are.\n";
        return 1;
    }
    buildDayList();
    runClashCheck();
    cout << "\nLoaded " << listCount << " classes. Clashes found: " << clashCount << "\n";
    srand(7);                                   // same "random" spoil every run

    while (true) {
        cout << "\n===== MENU =====\n"
             << "1 Show a section's timetable\n2 Check clashes\n3 Spoil timetable (demo)\n"
             << "4 Optimise (fix clashes)\n5 Simulate\n0 Exit\n";
        string choice = askLine("Choose: ");
        if (choice.empty()) break;
        char c = choice[0];

        if (c == '0') break;
        else if (c == '1') {
            listSections();
            string id = askLine("Section id: ");
            if (searchSectionById(id) != nullptr) showTable(id);
            else cout << "Section not found.\n";
        } else if (c == '2') {
            runClashCheck();
            printClashes();
        } else if (c == '3') {
            spoilTimetable();
        } else if (c == '4') {
            optimise();
        } else if (c == '5') {
            runSimulation();
        } else {
            cout << "Choose 0 to 5.\n";
        }
    }
    freeAllMemory();
    cout << "Goodbye.\n";
    return 0;
}
