//merge_classes moudule
#include <iostream>
#include <cstring>
using namespace std;
const int MAX_ENTRIES = 50;   // max entries expected in one time slot
const int MAX_MERGE_AUD = 6;  // university rule: max 6 sections in an auditorium
//data input will be done by Anushka while linking so for now working with this
struct Entry {
    char section[8];
    char subject[12];
    char faculty[8];
    char venue[8];
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
    // Destructor frees every list node, so no manual cleanup is needed by the caller
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
    // A graph owns raw pointers, so copying it is disabled
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    // Undirected edge;if A merges with B, B merges with A
    void addEdge(int a, int b) {
        head[a] = new AdjNode{b, head[a]};
        head[b] = new AdjNode{a, head[b]};
    }
    int size() const { return n; }
    bool isVisited(int u) const { return visited[u]; }
    // Collects every node reachable from u into members[]
    void dfs(int u, int members[], int& count) {
        visited[u] = true;
        members[count++] = u;
        for (AdjNode* p = head[u]; p != nullptr; p = p->next)
            if (!visited[p->to])
                dfs(p->to, members, count);
    }
};
class MergedGroup {
private:
    int members[MAX_ENTRIES];
    int count;
    int totalStrength;
    bool valid;
    char reason[40];
    MergedGroup* next;
public:
    MergedGroup() : count(0), totalStrength(0), valid(true), next(nullptr) {
        strcpy(reason, "ok");
    }
    void addMember(int entryIndex) { members[count++] = entryIndex; }
    int getCount() const { return count; }
    int getMember(int i) const { return members[i]; }

    void setStrength(int s) { totalStrength = s; }
    int getStrength() const { return totalStrength; }

    void setInvalid(const char* why) {
        valid = false;
        strncpy(reason, why, sizeof(reason) - 1);
        reason[sizeof(reason) - 1] = '\0';
    }
    bool isValid() const { return valid; }
    const char* getReason() const { return reason; }

    MergedGroup* getNext() const { return next; }
    void setNext(MergedGroup* p) { next = p; }
};
class GroupList {
private:
    MergedGroup* head;
    MergedGroup* tail;
public:
    GroupList() : head(nullptr), tail(nullptr) {}

    ~GroupList() {
        while (head) {
            MergedGroup* tmp = head;
            head = head->getNext();
            delete tmp;
        }
    }
    GroupList(const GroupList&) = delete;
    GroupList& operator=(const GroupList&) = delete;

    void add(MergedGroup* g) {
        if (head == nullptr) head = g;
        else tail->setNext(g);
        tail = g;
    }
    MergedGroup* getHead() const { return head; }
};
// ResourceTable: lookups (Person 1's pools will replace these tables)
struct SectionInfo { const char* id; int strength; };
struct VenueInfo { const char* id; int capacity; bool isAuditorium; };
class ResourceTable {
public:
    static int getStrength(const char* section) {
        static const SectionInfo table[] = {
            {"A", 60}, {"B", 55}, {"C", 62}, {"D", 58}, {"E", 50}
        };
        for (const SectionInfo& s : table)
            if (strcmp(s.id, section) == 0) return s.strength;
        return 0;
    }

    static const VenueInfo* findVenue(const char* venue) {
        static const VenueInfo table[] = {
            {"CR5", 70, false}, {"LT1", 150, false}, {"LT2", 250, false}, {"AUD1", 600, true}
        };
        for (const VenueInfo& v : table)
            if (strcmp(v.id, venue) == 0) return &v;
        return nullptr;
    }
};
// MergeDetector: finds and validates merged groups for one slot
class MergeDetector {
private:
    const Entry* entries;
    int n;
    Graph graph;
    GroupList groups;
    static bool sameClass(const Entry& x, const Entry& y) {
        return strcmp(x.subject, y.subject) == 0 &&
               strcmp(x.faculty, y.faculty) == 0 &&
               strcmp(x.venue, y.venue) == 0;
    }
    // Step 1: compare entries, every match becomes an edge
    void buildGraph() {
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (sameClass(entries[i], entries[j]))
                    graph.addEdge(i, j);
    }
    // Step 2: university rules for a merged group
    void validateGroup(MergedGroup* g) const {
        int total = 0;
        for (int i = 0; i < g->getCount(); i++)
            total += ResourceTable::getStrength(entries[g->getMember(i)].section);
        g->setStrength(total);

        const VenueInfo* v = ResourceTable::findVenue(entries[g->getMember(0)].venue);
        if (v == nullptr)
            g->setInvalid("unknown venue");
        else if (v->isAuditorium && g->getCount() > MAX_MERGE_AUD)
            g->setInvalid("more than 6 sections in auditorium");
        else if (total > v->capacity)
            g->setInvalid("venue capacity exceeded");
    }
public:
    MergeDetector(const Entry e[], int count)
        : entries(e),
          n(count > MAX_ENTRIES ? MAX_ENTRIES : count),
          graph(n) {}
    // Step 3: DFS collects components; 2+ entries = merged group
    void detect() {
        buildGraph();
        for (int i = 0; i < n; i++) {
            if (graph.isVisited(i)) continue;

            int found[MAX_ENTRIES];
            int cnt = 0;
            graph.dfs(i, found, cnt);

            if (cnt >= 2) {
                MergedGroup* g = new MergedGroup();
                for (int k = 0; k < cnt; k++) g->addMember(found[k]);
                validateGroup(g);
                groups.add(g);
            }
        }
    }
    void print() const {
        int num = 1;
        const MergedGroup* g = groups.getHead();
        if (g == nullptr) cout << "No merged groups in this slot.\n";
        for (; g != nullptr; g = g->getNext(), num++) {
            cout << "Merged group " << num << ": { ";
            for (int i = 0; i < g->getCount(); i++)
                cout << entries[g->getMember(i)].section << " ";
            cout << "}  subject=" << entries[g->getMember(0)].subject
                 << "  venue=" << entries[g->getMember(0)].venue
                 << "  strength=" << g->getStrength()
                 << "  -> " << (g->isValid() ? "VALID" : "INVALID")
                 << " (" << g->getReason() << ")\n";
        }
    }
};
// Test: one slot (Mon 10:10) with hardcoded entries
int main() {
    Entry slot[] = { {"A", "Maths", "F01", "LT1"}, {"B", "Maths", "F01", "LT1"},
        {"C", "DSA",   "F02", "CR5"}, {"D", "DSA",   "F02", "CR5"},
        {"E", "OOP",   "F03", "LT2"}
    };
    int n = sizeof(slot) / sizeof(slot[0]);
    cout << "Slot: MON 10:10-11:05\n";
    MergeDetector detector(slot, n);
    detector.detect();
    detector.print();
    // In the full program: create a new MergeDetector for every slot.
    return 0;
}