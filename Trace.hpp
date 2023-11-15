#ifndef TRACE_HPP
#define TRACE_HPP

#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <regex>
#include <stack>
#include <queue>

class Caller{
    public:
    std::string function_name;
    int line_number;
    bool valid;
    Caller(){
        line_number = -1;
        valid = false;
    }
};
class Trace {
public:
    std::vector<std::string> lines;
    std::string thread;

    bool fail;
    
    Trace() {}
    Trace(std::string name);
    ~Trace();

    void print();
    Caller find_caller(std::string function_name, int target_line);
};
std::string compare_trace(Trace* t1, Trace* t2);

class Call {
public:
    std::string function_name;
    std::string class_name;
    int lineNum;
    bool found;
    bool fail; // is part of fail run stack trace

    std::unordered_map<std::string, Call*> callers; // function name to caller*
    
    Call();
    Call(std::string f_name, int l_num);
    Call(std::string line);
    ~Call();

    void print();
    bool operator== (const Call& rhs) const;
    bool operator!= (const Call& rhs) const;
};
class CallTree {
public:
    Call* fi; // function of interest
    
    CallTree();
    ~CallTree();

    void print();

    bool addTrace(Trace* trace);
};

#endif // TRACE_HPP
