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

Caller Trace::find_caller(std::string function_name){
    std::string::size_type temp_id;
    Caller result;
    int i = 0;
    for(; i<lines.size(); i++){
        temp_id = lines[i].find(function_name);
        if(temp_id != std::string::npos){
            i++;
            break;
        }
    }
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

