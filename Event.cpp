#include "Event.hpp"
Event::Event(const int num) : lineNum(num) {
    type = EventType::undefined; 
    context = nullptr; 
    var = "";
}
Event::Event(const int num, EventType t) : lineNum(num), type(t) {
    context = nullptr; 
    var = "";
}

bool Event::operator== (const Event& rhs) const{
    if(lineNum != rhs.lineNum || type != rhs.type ){ // not same line number
        return false;
    }
    if(type == EventType::Invocation){
        // if invocational, call should be same
        if(value != rhs.value){
            return false;
        }
    }
    if(type == EventType::Condition){
        // if conditional, value should be same
        if(value != rhs.value){
             // return false;
        }
    }
    // should also compare context here
    return true;
}