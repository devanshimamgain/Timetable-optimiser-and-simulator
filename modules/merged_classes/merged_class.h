
#ifndef MERGED_CLASS_H
#define MERGED_CLASS_H

#include <iostream>
#include <string>

#include "../file_handling/person1_file_handling.h"

using namespace std;

constexpr int MAX_ENTRIES = 500;

// One section participating in a class.
struct SlotEntry {
    string section;
    string subject;
    string faculty;
    string venue;
    int duration;
    TimetableEntry* row;
};

// Information about an invalid merged group.
struct MergeProblem {
    TimetableEntry* row;
    string reason;
};

// --------------------------------------------------
// GRAPH
// --------------------------------------------------

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
    explicit Graph(int nodes);
    ~Graph();

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    void addEdge(int a, int b);
    bool isVisited(int u) const;
    void dfs(int u, int members[], int& count);
};

// --------------------------------------------------
// DETECTED GROUP
// --------------------------------------------------

class DetectedGroup {
private:
    int members[MAX_ENTRIES];
    int count;
    int totalStrength;
    bool valid;
    string reason;
    DetectedGroup* next;

public:
    DetectedGroup();

    void addMember(int entryIndex);
    int getCount() const;
    int getMember(int i) const;

    void setStrength(int s);
    int getStrength() const;

    void setInvalid(const string& why);
    bool isValid() const;
    const string& getReason() const;

    DetectedGroup* getNext() const;
    void setNext(DetectedGroup* p);
};

// --------------------------------------------------
// GROUP LINKED LIST
// --------------------------------------------------

class GroupList {
private:
    DetectedGroup* head;
    DetectedGroup* tail;
    int size;

public:
    GroupList();
    ~GroupList();

    GroupList(const GroupList&) = delete;
    GroupList& operator=(const GroupList&) = delete;

    void add(DetectedGroup* g);
    DetectedGroup* getHead() const;
    int getSize() const;
};

// --------------------------------------------------
// MERGED CLASS DETECTOR
// --------------------------------------------------

class MergeDetector {
private:
    const SlotEntry* entries;
    int n;

    Graph graph;
    GroupList groups;

    static bool sameClass(
        const SlotEntry& x,
        const SlotEntry& y
    );

    void buildGraph();
    void validateGroup(DetectedGroup* g) const;

public:
    MergeDetector(const SlotEntry e[], int count);

    void detect();

    int getGroupCount() const;
    const DetectedGroup* getFirstGroup() const;
    const SlotEntry& getEntry(int i) const;

    void print() const;
};

// --------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------

int collectSlotEntries(
    const string& day,
    int slot,
    SlotEntry out[],
    int maxCount
);

int checkMergedClassesInSlot(
    const string& day,
    int slot,
    MergeProblem problems[],
    int maxProblems,
    int& problemCount
);

void showMergedClasses();

#endif
