#include "Trace.hpp"
     // Copy constructor
Trace::Trace(std::string name) : thread(name) {
    fail = false;
}
Trace::~Trace() {
    lines.clear(); lines.resize(0);
}
void Trace::print(){
    std::cout << "thread " << thread << "stack trace: " << std::endl;
    for(std::string l : lines){
        std::cout << l << std::endl;
    }
}

Caller Trace::find_caller(std::string function_name, int target_line){
    std::string::size_type temp_id;
    Caller result;
    int i = 0; int index = i; bool found = false;
    for(; i<lines.size(); i++){
        temp_id = lines[i].find(function_name);
        if(temp_id != std::string::npos){
            std::cout << "i: " << i << std::endl;
            if(!found){
                index = i; found = true;
            }else{
                std::string line = lines[i]; temp_id = line.find(":"); 
                std::cout << "line: " << line << std::endl;
                if (temp_id != std::string::npos) {
                    line = line.substr(temp_id+1);
                }
                temp_id = line.find(")");
                if (temp_id != std::string::npos) {
                    line = line.substr(0,temp_id);
                }
                std::stringstream ss(line);
                int lineNum = -1; ss >> lineNum;

                std::cout << "lineNum " << lineNum << std::endl;
                if(lineNum == target_line){
                    index = i;
                }
            }
        }
    }
    i = index+1;
    for(; i<lines.size(); i++){
        std::string line = lines[i];
        temp_id = line.find("(");
        if(temp_id == std::string::npos){
            continue;
        }else{
            std::string name = line.substr(0, temp_id);
            std::string call = line.substr(temp_id+1);
            temp_id = name.rfind(".");
            if (temp_id != std::string::npos && temp_id != name.length()-1) {
                name = name.substr(temp_id + 1);
            }
            result.function_name = name;

            temp_id = call.find(":");
            if (temp_id != std::string::npos) {
                call = call.substr(temp_id+1);
            }
            temp_id = call.find(")");
            if (temp_id != std::string::npos) {
                call = call.substr(0,temp_id);
            }
            std::stringstream ss(call);
            int lineNum = -1; ss >> lineNum;
            result.line_number = lineNum;
            result.valid = true;
            return result;
        }
    }
    return result;
}

std::string compare_trace(Trace* t1, Trace* t2){
    int size1 = t1->lines.size();
    int size2 = t2->lines.size();
    int i=0;
    if(size1==0 || size2==0){
        return "";
    }
    for(; i<size1&&i<size2; i++){
        if(t1->lines[i] != t2->lines[i]){
            break;
        }
    }
    if(i>0){
        return t1->lines[i-1];
    }
    else{
        return t1->lines[0];
    }
}

// ================

    std::string function;
    int line;

    Call* caller;
    
Call::Call() {
    lineNum = -1;
    std::unordered_map<std::string, Call*> temp; callers = temp;
}

Call::Call(std::string f_name, int l_num): function_name(f_name), lineNum(l_num){
    std::unordered_map<std::string, Call*> temp; callers = temp;
}
Call::Call(std::string line){
    lineNum = -1; std::unordered_map<std::string, Call*> temp; callers = temp;
    std::string::size_type temp_id;
    temp_id = line.find("("); 
    if(temp_id != std::string::npos){ 
        std::string func = line.substr(0, temp_id);
        std::string call_site = line.substr(temp_id+1);
        temp_id = func.rfind(".");
        if (temp_id != std::string::npos && temp_id != func.length()-1) {
            function_name = func.substr(temp_id + 1);
            class_name = func.substr(0, temp_id);
        }else{
            function_name = func;
            class_name = "";
        }
        temp_id = call_site.find(")");
        call_site = call_site.substr(0, temp_id);
        temp_id = call_site.find(":");
        call_site = call_site.substr(temp_id+1);
        std::stringstream ss(call_site);
        int l = -1; ss >> l;
        if(l!=-1){
            lineNum = l;
            found = true;
        }
    }
}
void Call::print(){
    std::cout << function_name << "fail: " << fail << std::endl;
    // if(caller==nullptr){
    //     std::cout << "caller: nullptr" << std::endl;
    // }else{
    //     std::cout << "called by " << caller->function_name << " at line: " << caller->lineNum << std::endl;
    // }
}
Call::~Call(){}
bool Call::operator== (const Call& rhs) const{ // compare events across logs
    if(function_name != rhs.function_name){ // not same line number
        return false;
    }
    if(lineNum != rhs.lineNum){ // not same line number
        return false;
    }
    return true;
}
bool Call::operator!= (const Call& rhs) const{
    return !(this->operator==(rhs));
}

bool CallTree::addTrace(Trace* trace){
    if(trace==nullptr || trace->lines.size()==0){
        return false;
    }
    int i = 0; int size = trace->lines.size();
    std::string line = trace->lines[i];
    Call* current = new Call(line);
    if(fi==nullptr){
        fi = current;
    }else{
        while(current!=nullptr && (*fi != * current) && i<size){
            i++;
            line = trace->lines[i];
            free( current); current = nullptr;
            current = new Call(line);
        }
    }
    
    if(*fi != *current){ // fi is not in the new stack trace
        return false;
    }
    if(fi != current){
        free(current);
    }
    current = fi;
    while(i<size && current!=nullptr){
        if(trace->fail){
            current->fail = true;
        }
        i++;
        line = trace->lines[i]; 
        Call* next = new Call(line);
        if(next==nullptr){
            std::cout << "in addTrace(): next call is null" << std::endl;
            return false;
        }
        std::string next_func = next->function_name;
        if(next_func == ""){
            std::cout << "in addTrace(): next call is empty" << std::endl;
            return false;
        }
        if(current->callers.find(next_func)==current->callers.end()){
            // is a new caller (diverge)
            current->callers[next_func] = next; 
        }else{
            free(next); next = nullptr;
            next = current->callers[next_func];
        }
        current = next;
    }
    return true;
}

void CallTree::print(){
    if(fi == nullptr){
        std::cout << "fi is null" << std::endl;
        return;
    }
    Call* current = fi;
    std::cout << "fi:" << std::endl;
    fi->print();
}

