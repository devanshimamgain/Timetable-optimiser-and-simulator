#define main person1_main
#include "../Person1_CSVHandler_FIXED.cpp"
#undef main
const int MAX_ENTRIES = 50;

// University rules, read from config.txt (see data/data_full/config.txt). 0 or empty = no rule.
int maxMergeAud = 0;
string auditoriumType = "";

void loadMergeConfig() {
    ifstream file("config.txt");
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        string key = trim(line.substr(0, eq));
        string val = trim(line.substr(eq + 1));
        if (key == "max_sections_in_auditorium") maxMergeAud = atoi(val.c_str());
        else if (key == "auditorium_type") auditoriumType = val;
    }
}

struct SlotEntry {
    string section;
    string subject;
    string faculty;
    string venue;
    int duration;
};

struct AdjNode {
    int to;
    AdjNode* next;
};

class Graph {
private:
    int n;
    AdjNode* head[MAX_ENTRIES];
    bool visited[MAX_ENTRIES];

public:
    explicit Graph(int nodes) : n(nodes) {
        for (int i = 0; i < n; i++) {
            head[i] = nullptr;
            visited[i] = false;
        }
    }

    ~Graph() {
        for (int i = 0; i < n; i++) {
            AdjNode* p = head[i];
            while (p) {
                AdjNode* tmp = p;
                p = p->next;
                delete tmp;
            }
        }
    }
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    void addEdge(int a, int b) {
        head[a] = new AdjNode{b, head[a]};
        head[b] = new AdjNode{a, head[b]};
    }
    bool isVisited(int u) const 
    { 
        return visited[u]; 
    }
    void dfs(int u, int members[], int& count) {
        visited[u] = true;
        members[count++] = u;
        for (AdjNode* p = head[u]; p != nullptr; p = p->next)
            if (!visited[p->to])
                dfs(p->to, members, count);
    }
};
class DetectedGroup {
private:
    int members[MAX_ENTRIES];     
    int count;
    int totalStrength;
    bool valid;
    string reason;
    DetectedGroup* next;
 
public:
    DetectedGroup() : count(0), totalStrength(0), valid(true), reason("ok"), next(nullptr) {}
 
    void addMember(int entryIndex) { members[count++] = entryIndex; }
    int getCount() const { return count; }
    int getMember(int i) const { return members[i]; }
 
    void setStrength(int s) { totalStrength = s; }
    int getStrength() const { return totalStrength; }
 
    void setInvalid(const string& why) { valid = false; reason = why; }
    bool isValid() const { return valid; }
    const string& getReason() const { return reason; }
 
    DetectedGroup* getNext() const { return next; }
    void setNext(DetectedGroup* p) { next = p; }
};
class GroupList {
private:
    DetectedGroup* head;
    DetectedGroup* tail;
    int size;
 
public:
    GroupList() : head(nullptr), tail(nullptr), size(0) {}
 
    ~GroupList() {
        while (head) {
            DetectedGroup* tmp = head;
            head = head->getNext();
            delete tmp;
        }
    }
 
    GroupList(const GroupList&) = delete;
    GroupList& operator=(const GroupList&) = delete;
 
    void add(DetectedGroup* g) {
        if (head == nullptr) head = g;
        else tail->setNext(g);
        tail = g;
        size++;
    }
 
    DetectedGroup* getHead() const { return head; }
    int getSize() const { return size; }
};
class MergeDetector {
private:
    const SlotEntry* entries;
    int n;
    Graph graph;
    GroupList groups;
 
    // Same class for different sections, and the same length. Entries start in the same slot here.
    static bool sameClass(const SlotEntry& x, const SlotEntry& y) {
        return x.section != y.section &&
               x.subject == y.subject &&
               x.faculty == y.faculty &&
               x.venue == y.venue &&
               x.duration == y.duration;
    }
 
    void buildGraph() {
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (sameClass(entries[i], entries[j]))
                    graph.addEdge(i, j);
    }
 
    void validateGroup(DetectedGroup* g) const {
        int total = 0;
        for (int i = 0; i < g->getCount(); i++) {
            Section* s = searchSectionById(entries[g->getMember(i)].section);
            if (s == nullptr) {
                g->setInvalid("unknown section " + entries[g->getMember(i)].section);
                return;
            }
            total += s->strength;
        }
        g->setStrength(total);
 
        Venue* v = searchVenueById(entries[g->getMember(0)].venue);
        if (v == nullptr)
            g->setInvalid("unknown venue");
        else if (maxMergeAud > 0 && !auditoriumType.empty() && v->type == auditoriumType && g->getCount() > maxMergeAud)
            g->setInvalid("too many sections in auditorium (limit from config.txt)");
        else if (total > v->capacity)
            g->setInvalid("venue capacity exceeded");
    }
 
public:
    MergeDetector(const SlotEntry e[], int count)
        : entries(e),
          n(count > MAX_ENTRIES ? MAX_ENTRIES : count),
          graph(n) {}
 
    void detect() {
        buildGraph();
        for (int i = 0; i < n; i++) {
            if (graph.isVisited(i)) continue;
 
            int found[MAX_ENTRIES];
            int cnt = 0;
            graph.dfs(i, found, cnt);
 
            if (cnt >= 2) {
                DetectedGroup* g = new DetectedGroup();
                for (int k = 0; k < cnt; k++) g->addMember(found[k]);
                validateGroup(g);
                groups.add(g);
            }
        }
    }
 
    int getGroupCount() const { return groups.getSize(); }
    const DetectedGroup* getFirstGroup() const { return groups.getHead(); }
    const SlotEntry& getEntry(int i) const { return entries[i]; }
 
    void print() const {
        int num = 1;
        for (const DetectedGroup* g = groups.getHead(); g != nullptr; g = g->getNext(), num++) {
            cout << "  Merged class " << num << ": { ";
            for (int i = 0; i < g->getCount(); i++)
                cout << entries[g->getMember(i)].section << " ";
            cout << "}  " << entries[g->getMember(0)].subject
                 << "  faculty=" << entries[g->getMember(0)].faculty
                 << "  venue=" << entries[g->getMember(0)].venue
                 << "  strength=" << g->getStrength()
                 << "  -> " << (g->isValid() ? "VALID" : "INVALID")
                 << " (" << g->getReason() << ")\n";
        }
    }
};
int collectSlotEntries(const string& day, int slot, SlotEntry out[], int maxCount) {
    int n = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next) {
        if (t->day != day) continue;
        // a class is counted only in the slot where it starts, so a 2-slot class is not counted twice
        if (slot != t->startSlot) continue;

        MergedGroup* declared = searchMergedGroupById(t->sectionOrGroup);
        if (declared == nullptr) {
            if (n < maxCount) out[n++] = SlotEntry{t->sectionOrGroup, t->subject, t->facultyId, t->venueId, t->duration};
        } else {
            string ids = declared->sectionIds;     
            string current;
            for (size_t i = 0; i <= ids.size(); i++) {
                if (i == ids.size() || ids[i] == ';') {
                    if (!current.empty() && n < maxCount)
                        out[n++] = SlotEntry{current, t->subject, t->facultyId, t->venueId, t->duration};
                    current.clear();
                } else {
                    current += ids[i];
                }
            }
        }
    }
    return n;
}

int main() {
    loadAllCSVFiles();
    loadMergeConfig();

    const string days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
 
    int maxSlot = 0;
    for (TimetableEntry* t = timetableHead; t != nullptr; t = t->next)
        if (t->startSlot + t->duration - 1 > maxSlot) maxSlot = t->startSlot + t->duration - 1;
 
    int totalGroups = 0, invalidGroups = 0;
 
    cout << "\n===== MERGED CLASSES FOUND =====\n";
    for (const string& day : days) {
        for (int slot = 1; slot <= maxSlot; slot++) {
            SlotEntry entries[MAX_ENTRIES];
            int n = collectSlotEntries(day, slot, entries, MAX_ENTRIES);
 
            MergeDetector detector(entries, n);
            detector.detect();
 
            if (detector.getGroupCount() > 0) {
                cout << day << ", slot " << slot << ":\n";
                detector.print();
                totalGroups += detector.getGroupCount();
                for (const DetectedGroup* g = detector.getFirstGroup(); g != nullptr; g = g->getNext())
                    if (!g->isValid()) invalidGroups++;
            }
        }
    }
 
    cout << "\nTotal merged classes: " << totalGroups
         << "   Invalid: " << invalidGroups << "\n";
 
    freeAllMemory();
    return 0;
}