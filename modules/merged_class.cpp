//merged section detection code
#include <iostream>
#include <cstring>//to use string functions of C
using namespace std;
const int MAX_ENTRIES = 50;  //maximum classes assumption is 50
const int MAX_MERGE_AUD = 6; //maximum merged classes assumption is 
struct Entry { //data from data-input-module
    char section[8];
    char subject[12];
    char faculty[8];
    char venue[8];
};
struct AdjNode {
    int to;          // index of neighbouring entry  
    AdjNode* next;
};
struct Graph {
    int n;                          
    AdjNode* head[MAX_ENTRIES];     
    bool visited[MAX_ENTRIES];      // used by dfs so we never visit a node twice 
};
void initGraph(Graph& g, int n) {
    g.n = n;
    for (int i = 0; i < n; i++) {
        g.head[i] = NULL;
        g.visited[i] = false;
    }
}
void addEdge(Graph& g, int a, int b) { //undirected edge;if A merges with B, B merges with A
    g.head[a] = new AdjNode{b, g.head[a]};
    g.head[b] = new AdjNode{a, g.head[b]};
}
void freeGraph(Graph& g) {
    for (int i = 0; i < g.n; i++) {
        AdjNode* p = g.head[i];
        while (p) {
            AdjNode* tmp = p;
            p = p->next;
            delete tmp;
        }
        g.head[i] = NULL;
    }
}
struct Group {   //merged group kept in our own linked list
    int members[MAX_ENTRIES];   
    int count;
    int totalStrength;
    bool valid;
    char reason[40];
    Group* next;
};
void freeGroups(Group* head) {
    while (head) {
        Group* tmp = head;
        head = head->next;
        delete tmp;
    }
}
struct SectionInfo { const char* id; int strength; };
SectionInfo sectionTable[] = { {"A", 60}, {"B", 55}, {"C", 62}, {"D", 58}, {"E", 50} };
struct VenueInfo { const char* id; int capacity; bool isAuditorium; };
VenueInfo venueTable[] = { {"CR5", 70, false}, {"LT1", 150, false}, {"LT2", 250, false}, {"AUD1", 600, true} };
int getStrength(const char* section) {
    int count = sizeof(sectionTable) / sizeof(sectionTable[0]);
    for (int i = 0; i < count; i++)
        if (strcmp(sectionTable[i].id, section) == 0) return sectionTable[i].strength;
    return 0;
}
const VenueInfo* findVenue(const char* venue) {
    int count = sizeof(venueTable) / sizeof(venueTable[0]);
    for (int i = 0; i < count; i++)
        if (strcmp(venueTable[i].id, venue) == 0) return &venueTable[i];
    return NULL;
}
bool sameClass(const Entry& x, const Entry& y) { //building graph for one slot
    return strcmp(x.subject, y.subject) == 0 &&
           strcmp(x.faculty, y.faculty) == 0 &&
           strcmp(x.venue, y.venue) == 0;
}
void buildGraph(Graph& g, const Entry e[], int n) {
    initGraph(g, n);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (sameClass(e[i], e[j]))
                addEdge(g, i, j);
}
void dfs(Graph& g, int u, int members[], int& count) {//dfs collects everything reachable from node u
    g.visited[u] = true;
    members[count++] = u;
    for (AdjNode* p = g.head[u]; p != NULL; p = p->next)
        if (!g.visited[p->to])
            dfs(g, p->to, members, count);
}
void validateGroup(Group* grp, const Entry e[]) {//checking rules for merged classes
    grp->totalStrength = 0;
    for (int i = 0; i < grp->count; i++)
        grp->totalStrength += getStrength(e[grp->members[i]].section);
    grp->valid = true;
    strcpy(grp->reason, "ok");
    const VenueInfo* v = findVenue(e[grp->members[0]].venue);
    if (v == NULL) {
        grp->valid = false;
        strcpy(grp->reason, "unknown venue");
    } else if (v->isAuditorium && grp->count > MAX_MERGE_AUD) {
        grp->valid = false;
        strcpy(grp->reason, "more than 6 sections in auditorium");
    } else if (grp->totalStrength > v->capacity) {
        grp->valid = false;
        strcpy(grp->reason, "venue capacity exceeded");
    }
}
Group* findGroups(Graph& g, const Entry e[]) {//checking for all merged groups
    Group* head = NULL;
    Group* tail = NULL;
    for (int i = 0; i < g.n; i++) {
        if (g.visited[i]) continue;
        Group* grp = new Group;
        grp->count = 0;
        grp->next = NULL;
        dfs(g, i, grp->members, grp->count);
        if (grp->count >= 2) {     //1 section alone=normal class, not merged     
            validateGroup(grp, e);
            if (head == NULL) head = grp;
            else tail->next = grp;
            tail = grp;
        } else {
            delete grp;
        }
    }
    return head;
}
void printGroups(const Group* head, const Entry e[]) {
    int num = 1;
    for (const Group* g = head; g != NULL; g = g->next, num++) {
        cout << "Merged group " << num << ": { ";
        for (int i = 0; i < g->count; i++)
            cout << e[g->members[i]].section << " ";
        cout << "}  subject=" << e[g->members[0]].subject
             << "  venue=" << e[g->members[0]].venue
             << "  strength=" << g->totalStrength
             << "  -> " << (g->valid ? "VALID" : "INVALID") << " (" << g->reason << ")\n";
    }
    if (head == NULL) cout << "No merged groups in this slot.\n";
}
int main() { //testing with one timeslot
    Entry slot[] = { {"A", "Maths", "F01", "LT1"},{"B", "Maths", "F01", "LT1"},
        {"C", "DSA",   "F02", "CR5"},{"D", "DSA",   "F02", "CR5"},
        {"E", "OOP",   "F03", "LT2"}
    };
    int n = sizeof(slot) / sizeof(slot[0]);
    cout << "Slot: MON 10:10-11:05\n";
    Graph g;
    buildGraph(g, slot, n);            
    Group* groups = findGroups(g, slot);   
    printGroups(groups, slot);
    //in the full program; repeat for every slot witha fresh graph
    freeGroups(groups);
    freeGraph(g);
    return 0;
}