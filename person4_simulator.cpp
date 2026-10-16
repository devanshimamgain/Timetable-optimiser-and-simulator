// Person 4: Timetable simulation + SFML GUI
// C++17, SFML 2.5/2.6
// CSV format (no quoted commas):
// day,period,duration,subject,section,faculty,venue,clash,recommendation
// Mon,1,1,Math,A,Dr Rao,R101,0,
// Mon,1,1,Physics,B,Dr Rao,R102,1,"Move Physics to period 3"
// Tue,2,2,Lab,A,Dr Sen,L201,0,
//
// Compile on Linux:
// g++ -std=c++17 person4_simulator.cpp -o simulator -lsfml-graphics -lsfml-window -lsfml-system
// Put a font named arial.ttf beside executable, or change FONT_PATH below.
// Run: ./simulator timetable.csv
//
// This module visualizes the clash flag/recommendation supplied by teammate 3.
// Connect loadCSV() to your team's actual data/detector for final integration.

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>

struct Entry {
    std::string day, subject, section, faculty, venue, recommendation;
    int period=1, duration=1;
    bool clash=false;
};

int dayNumber(const std::string& d) {
    const std::string days[]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    for(int i=0;i<7;++i) if(d==days[i]) return i;
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

bool loadCSV(const std::string& filename,TimetableQueue& q){
    std::ifstream file(filename);
    if(!file)return false;
    std::string line;
    while(std::getline(file,line)){
        if(line.empty())continue;
        std::stringstream ss(line);
        std::string f[9];
        for(int i=0;i<9;++i) if(!std::getline(ss,f[i],','))f[i]="";
        if(f[0]=="day"||f[0]=="Day")continue;
        try{
            Entry e;
            e.day=f[0];e.period=std::stoi(f[1]);e.duration=std::max(1,std::stoi(f[2]));
            e.subject=f[3];e.section=f[4];e.faculty=f[5];e.venue=f[6];
            e.clash=(std::stoi(f[7])!=0);e.recommendation=f[8];
            if(e.period>0)q.enqueue(e);
        }catch(...){std::cerr<<"Skipping malformed CSV row: "<<line<<"\n";}
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

sf::Text text(const std::string& s,const sf::Font& f,unsigned size,sf::Vector2f pos,sf::Color c=sf::Color(30,35,45)){
    sf::Text t(s,f,size);t.setPosition(pos);t.setFillColor(c);return t;
}
int main(int argc,char**argv){
    TimetableQueue q;
    std::string path=argc>1?argv[1]:"timetable.csv";
    if(!loadCSV(path,q)){std::cerr<<"Cannot open "<<path<<". Create CSV as shown in source header.\n";return 1;}
    Slot slots[500];int slotCount=makeSlots(q,slots,500);int current=-1;

    sf::RenderWindow win(sf::VideoMode(1100,760),"Timetable Optimiser | Slot Simulator");
    win.setFramerateLimit(60);
    sf::Font font;
    if(!font.loadFromFile("arial.ttf")&&!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")){
        std::cerr<<"Font missing: place arial.ttf beside executable.\n";return 1;
    }
    sf::RectangleShape prev({145,44}),next({145,44}),reset({120,44});
    prev.setPosition(650,690);next.setPosition(810,690);reset.setPosition(970,690);
    prev.setFillColor(sf::Color(80,90,110));next.setFillColor(sf::Color(40,105,190));reset.setFillColor(sf::Color(100,100,100));
    auto buttonText=[&](const char* s,float x){return text(s,font,17,{x,700},sf::Color::White);};
    sf::Text title=text("TIMETABLE SLOT SIMULATOR",font,26,{24,18});
    sf::Text help=text("Tap NEXT/PREVIOUS to step through the timetable; RESET returns to start.",font,15,{24,55},sf::Color(90,95,105));
    sf::Text slotTitle("",font,22,{24,100});
    sf::Text status("",font,19,{24,140});
    sf::RectangleShape tableBg({1050,465});tableBg.setPosition(24,185);tableBg.setFillColor(sf::Color(248,249,251));tableBg.setOutlineThickness(1);tableBg.setOutlineColor(sf::Color(190,195,205));
    sf::Text tableText("",font,17,{42,202});
    sf::Text recTitle("CLASH DETAILS / SUGGESTIONS",font,18,{24,610});
    sf::Text recText("",font,14,{24,638});
    while(win.isOpen()){
        sf::Event ev;
        while(win.pollEvent(ev)){
            if(ev.type==sf::Event::Closed)win.close();
            if(ev.type==sf::Event::MouseButtonPressed&&ev.mouseButton.button==sf::Mouse::Left){
                sf::Vector2f m((float)ev.mouseButton.x,(float)ev.mouseButton.y);
                if(next.getGlobalBounds().contains(m)){if(current<slotCount-1)++current;}
                if(prev.getGlobalBounds().contains(m)){if(current>=0)--current;}
                if(reset.getGlobalBounds().contains(m))current=-1;
            }
        }
        if(current<0){slotTitle.setString("Ready: "+std::to_string(q.size())+" timetable entries loaded");
            status.setString("No slot selected. Press NEXT SLOT to start.");status.setFillColor(sf::Color(50,90,160));
            tableText.setString("The selected slot's class allocations will appear here.");
            recText.setString("When the clash detector supplies clash details and proposed fixes, they will be shown here.");
        }else if(current>=slotCount){
            slotTitle.setString("Simulation complete");status.setString("Every distinct day/period batch has been displayed.");
            tableText.setString("End of timetable.");recText.setString("");
        }else{
            Slot& s=slots[current];
            slotTitle.setString("Slot "+std::to_string(current+1)+" / "+std::to_string(slotCount)+"   |   "+s.day+"   |   Period "+std::to_string(s.period));
            status.setString(s.clash?"CLASH FLAGGED — red entries need review":"VALID SLOT — no clash flag reported");
            status.setFillColor(s.clash?sf::Color(190,35,40):sf::Color(20,135,65));
            std::ostringstream rows,rec;
            rows<<"SUBJECT | SECTION | FACULTY | VENUE | LENGTH\n\n";
            for(int i=0;i<s.n;++i){
                Entry&e=s.rows[i];
                rows<<(e.clash?"[CLASH] ":"[OK] ")<<e.subject<<" | "<<e.section<<" | "<<e.faculty<<" | "<<e.venue<<" | "<<e.duration<<" period(s)\n";
                if(e.clash){
                    rec<<e.subject<<" ("<<e.section<<"): ";
                    if(!e.recommendation.empty())rec<<e.recommendation;
                    else rec<<"No specific fix supplied; check alternate free period, faculty, or room.";
                    rec<<"\n";
                }
            }
            tableText.setString(rows.str());recText.setString(rec.str().empty()?"No clash recommendations for this slot.":rec.str());
        }
        win.clear(sf::Color(232,236,242));win.draw(title);win.draw(help);win.draw(slotTitle);win.draw(status);win.draw(tableBg);win.draw(tableText);win.draw(recTitle);win.draw(recText);
        win.draw(prev);win.draw(next);win.draw(reset);win.draw(buttonText("PREVIOUS",666));win.draw(buttonText("NEXT SLOT",830));win.draw(buttonText("RESET",995));
        win.display();
    }
}
