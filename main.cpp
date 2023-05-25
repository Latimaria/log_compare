#include <iostream>
#include <sstream> 
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>

#include "Event.hpp"
#include "Log.hpp"

std::string chooseRandom(int init, const std::vector<bool> &cond){ // L1
    std::stringstream log; log << std::boolalpha;
    int val = init; // L2
    log << "L2 " << "val=" << val << " "; // L2 instrument: val
    int loop = cond.size(); // number of loops
    
    for(int i=0; i<loop; i++){ // L3: loop condition
        log << "L3 " << (i<loop) << " "; // L3 instrument: loop condition
        log << "L4 " << cond[i] << " "; // L4 instrument: if condition
        if(cond[i]){ // L4: if condition
            val--; // L5 
            log << "L5 " << "val=" << val << " "; // L5 instrument: val change
        }
    }
    log << "L6 " << (val > 0) << " "; // L6 instrument: if condition
    if(val > 0){ // L6: if condition
        log << "L7 " << "fail "; // L7 instrument: throw Exception
    }
    return log.str();
}


    
std::pair<std::vector<Event>, std::vector<Event>> logCompare(std::string failed_str, std::vector<std::string>& succeed_str){
    std::vector<Event> prefix; // longest common prefix
    std::vector<Event> longest;
    if(succeed_str.size()==0) {return std::make_pair(prefix, longest);}
    int max_length = 0; // longest prefix length
    int max_idx = -1;

    Log failed(failed_str); 
    for(int i=0; i<succeed_str.size(); i++){
        Log succeed(succeed_str[i]);
        int idx = 0; int length = 0; 
        while(failed.getEvent(idx)!=nullptr && succeed.getEvent(idx)!=nullptr){
            Event* ef = failed.getEvent(idx); // parse as we proceed
            Event* es = succeed.getEvent(idx); // might be able to store this some where
            if(*es == *ef){ // compare lineNum
                length++;
                if(length > max_length){
                    max_length = length; // update max length
                    max_idx = i; // the run index in vector woth longest common prefix
                }
            }else{
                break; // diverge
            }
            idx++;
        }
    }
    std::cout << std::endl;
    int i=0; 
    for (; i < max_length; i++) {
        Event* ef = failed.getEvent(i);
        prefix.push_back(*ef); // reconstruct common prefix from failed
    }
    // tried to reuse the part already parsed, but get a bug. will figure that out later
    i = 0; Log lon(succeed_str[max_idx]); 
    while(i<lon.current || !lon.parsedAll()){
        Event* es = lon.getEvent(i);
        if(es == nullptr){
            break;
        }else{        
            longest.push_back(*es);
        }
        i++;
    }
    return std::make_pair(prefix, longest);
} 


int main (){
    int init = 3;
    std::vector<bool> cond = {false, false, false};
    std::cout << std::boolalpha;
    std::string failed_str = chooseRandom(init, cond);
    std::cout << "Failed Run: " << std::endl << failed_str << std::endl;

    std::cout << "Successful Runs: " << std::endl;
    std::vector<std::string> succeeds;
    init = 3; cond = {true, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    init = 3; cond = {true, false, false, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    init = 4; cond = {false, true, true, true, true};
    succeeds.push_back(chooseRandom(init, cond));
    for(int i=0; i<succeeds.size(); i++){
        std::cout << "Run " << i << ": " << succeeds[i] << std::endl;
    }

    auto result = logCompare(failed_str, succeeds);
    std::cout << "Prefix: " << std::endl;
    for(int i=0; i<result.first.size(); i++){
        std::cout << "L" << result.first[i].lineNum << " ";
    } std::cout << std::endl;
    std::cout << "Longest: " << std::endl;
    for(int i=0; i<result.second.size(); i++){
        std::cout << "L" << result.second[i].lineNum << " ";
    } std::cout << std::endl;
    return 0;
}

/* // current output:
Failed Run: 
L2 val=3 L3 true L4 false L3 true L4 false L3 true L4 false L6 true L7 fail 
Successful Runs: 
Run 0: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 1: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 false L3 true L4 false L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 2: L2 val=4 L3 true L4 false L3 true L4 true L5 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 

//// if we consider CE with different value (True / False) as different log:
Prefix: 
L2 L3 L4 L3 
Longest: 
L2 L3 L4 L3 L4 L5 L3 L4 L5 L3 L4 L5 L3 L4 L5 L6 
// diverge at L3

//// otherwise:
Prefix: 
L2 L3 L4 L3 L4 
Longest: 
L2 L3 L4 L3 L4 L5 L3 L4 L5 L3 L4 L5 L3 L4 L5 L6 
// diverge at L4
*/