#define main person1_main
#include "../Person1_CSVHandler_FIXED.cpp"
#undef main
const int MAX_ENTRIES = 50;       
const int MAX_MERGE_AUD = 6;      
struct SlotEntry {
    string section;
    string subject;
    string faculty;
    string venue;
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