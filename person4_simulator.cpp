// Person 4: Timetable simulator (console, no graphics library)
//
// Uses Person 1's loader (Person1_CSVHandler_FIXED.cpp) and Person 3's clash check (gi.cpp).
// The classes are put into a queue and shown slot by slot.
//
// Compile: g++ -std=c++17 -Wall -Wextra person4_simulator.cpp -o simulator.exe
// Run (in the folder that has the CSV files): ./simulator.exe
//
// Commands:  n = next slot,  p = previous slot,  r = reset,  a = show all slots,  q = quit

#define GI_NO_MAIN            // gi.cpp has its own main(); this file has its own
#include "gi.cpp"

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>

struct Entry {
    std::string day, subject, section, faculty, venue, recommendation;
    int period=1, duration=1;
    bool clash=false;
};

// "Monday", "MON" and "mon" all give 0 (Monday). Unknown day gives 7 (last).
int dayNumber(const std::string& d){
    const std::string days[]={"mon","tue","wed","thu","fri","sat","sun"};
    std::string key;
    for(size_t i=0;i<3&&i<d.size();++i) key+=(char)std::tolower((unsigned char)d[i]);
    for(int i=0;i<7;++i) if(key==days[i]) return i;
    return 7;
}
bool earlier(const Entry& a,const Entry& b) {
    if(dayNumber(a.day)!=dayNumber(b.day)) return dayNumber(a.day)<dayNumber(b.day);
    return a.period<b.period;
}

// Custom singly linked list used as the timetable queue.
class TimetableQueue {
    struct Node { Entry item; Node* next; explicit Node(const Entry& e):item(e),next(nullptr){} };
    Node* first=nullptr;
    Node* last=nullptr;
    int count=0;
public:
    ~TimetableQueue(){clear();}
    TimetableQueue()=default;
    TimetableQueue(const TimetableQueue&)=delete;
    TimetableQueue& operator=(const TimetableQueue&)=delete;
    void enqueue(const Entry& e){
        Node* n=new Node(e);
        if(!last) first=last=n;
        else {last->next=n;last=n;}
        ++count;
    }
    bool empty() const{return first==nullptr;}
    int size() const{return count;}
    Entry at(int index) const {
        if(index<0||index>=count) throw std::out_of_range("queue index");
        Node* n=first; for(int i=0;i<index;++i)n=n->next;
        return n->item;
    }
    void clear(){
        while(first){Node* n=first;first=first->next;delete n;}
        last=nullptr;count=0;
    }
    // Stable insertion sort by weekday then period, without STL containers.
    void sortChronologically(){
        Node* sorted=nullptr;
        while(first){
            Node* n=first; first=first->next;
            if(!sorted||earlier(n->item,sorted->item)){
                n->next=sorted;sorted=n;
            } else {
                Node* p=sorted;
                while(p->next&&!earlier(n->item,p->next->item))p=p->next;
                n->next=p->next;p->next=n;
            }
        }
        first=sorted;last=first;
        if(last)while(last->next)last=last->next;
    }
};

// Text of every clash that involves this class (from Person 3's check).
std::string clashTextFor(const TimetableEntry* t){
    std::string r;
    for(int i=0;i<clashCount;++i)
        if(clashes[i].a==t||clashes[i].b==t){
            if(!r.empty()) r+="; ";
            r+=clashes[i].kind+": "+clashes[i].text;
        }
    return r;
}

// Loads the timetable with Person 1's loader, checks it for clashes (Person 3),
// and puts every class into the queue.
bool loadCSV(TimetableQueue& q){
    loadAllCSVFiles();
    if(timetableHead==nullptr) return false;
    runClashCheck();
    for(TimetableEntry* t=timetableHead;t!=nullptr;t=t->next){
        Entry e;
        e.day=t->day;
        e.period=t->startSlot;
        e.duration=t->duration;
        e.subject=t->subject;
        e.section=t->sectionOrGroup;
        Faculty* f=searchFacultyById(t->facultyId);
        e.faculty=f?f->name:t->facultyId;
        e.venue=t->venueId;
        e.clash=hasClash(t);
        e.recommendation=clashTextFor(t);
        q.enqueue(e);
    }
    q.sortChronologically();
    return true;
}

struct Slot {
    std::string day;
    int period=0;
    Entry rows[100];
    int n=0;
    bool clash=false;
};

// Groups the sorted queue into slots: one Slot per (day, period).
int makeSlots(const TimetableQueue& q,Slot slots[],int cap){
    int total=0;
    for(int i=0;i<q.size();++i){
        Entry e=q.at(i);
        if(total==0||slots[total-1].day!=e.day||slots[total-1].period!=e.period){
            if(total>=cap)break;
            slots[total].day=e.day;slots[total].period=e.period;slots[total].n=0;slots[total].clash=false;
            ++total;
        }
        Slot& s=slots[total-1];
        if(s.n<100){s.rows[s.n++]=e;s.clash=s.clash||e.clash;}
    }
    return total;
}

// Prints one slot: its classes, and the clash reason for each class that has one.
void showSlot(const Slot& s,int number,int total){
    std::cout << "\n===== SLOT " << number << " OF " << total << " | " << s.day << " | PERIOD " << s.period << " =====\n";
    std::cout << (s.clash ? "STATUS: CLASH FLAGGED - check the entries marked [CLASH]\n"
                          : "STATUS: no clash reported for this slot\n");
    for(int i=0;i<s.n;++i){
        const Entry& e=s.rows[i];
        std::cout << (e.clash ? "[CLASH] " : "[OK]    ")
                  << e.subject << " | " << e.section << " | " << e.faculty << " | " << e.venue
                  << " | " << e.duration << " period(s)\n";
        if(e.clash) std::cout << "         reason: " << e.recommendation << "\n";
    }
}

int main(){
    TimetableQueue q;
    if(!loadCSV(q)){
        std::cerr<<"Could not load timetable.csv from this folder. Run the program where the CSV files are.\n";
        return 1;
    }
    static Slot slots[500];
    int slotCount=makeSlots(q,slots,500);
    int current=-1;                       // -1 = not started yet

    std::cout<<"\nSimulator ready: "<<slotCount<<" time slots.\n"
             <<"Commands: n = next, p = previous, r = reset, a = show all, q = quit\n";

    std::string cmd;
    while(true){
        std::cout<<"\n> ";
        if(!std::getline(std::cin,cmd)) break;
        char c=cmd.empty()?' ':(char)std::tolower((unsigned char)cmd[0]);

        if(c=='n'){
            if(current<slotCount-1){ ++current; showSlot(slots[current],current+1,slotCount); }
            else std::cout<<"Simulation complete. Press r to start again.\n";
        } else if(c=='p'){
            if(current>0){ --current; showSlot(slots[current],current+1,slotCount); }
            else std::cout<<"Already at the first slot.\n";
        } else if(c=='r'){
            current=-1;
            std::cout<<"Reset. Press n to start.\n";
        } else if(c=='a'){
            for(int i=0;i<slotCount;++i) showSlot(slots[i],i+1,slotCount);
        } else if(c=='q'){
            break;
        } else {
            std::cout<<"Use n, p, r, a or q.\n";
        }
    }

    freeAllMemory();
    std::cout<<"Goodbye.\n";
    return 0;
}
