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
};