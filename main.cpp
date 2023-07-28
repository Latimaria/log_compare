#include <iostream>
#include <sstream> 
#include <fstream>
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_map>
#include <regex>

#include "Event.hpp"
#include "Log.hpp"
#include "globals.hpp"
#include "Trace.hpp"

#define DIV 0
#define TRACE 1
#define ARG 2

std::pair<Log*, std::vector<Event>> logCompare(Log* failed, std::vector<Log*> succeeds);                                                                                                              
int main (int argc, char *argv[]){    ////////////////////////////////////////////////////////////////////
    std::string file_path = "logs/production_log_round1.txt";
    std::string base_path = "/home/ubuntu/hadoop/hadoop-hdfs-project/hadoop-hdfs/src/main/java/";
    int what_to_do = DIV; 
    
    if(argc>=2){         // ./compare {file name} {to do} {failureIndicator} {newLogIndicator}
        file_path = argv[1];    
    } //"";
    if(argc>=3){
        std::stringstream ss (argv[2]);         // 0: find divergence
        ss >> what_to_do;                   // 1: print stack trace
        std::cout << "to do: " << what_to_do << std::endl; // 2: find argument value
    }
    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return 1;
    }

    
    // std::string failureIndicator = "BlockManager$ReplicationMonitor"; // using thread name for now
    std::string failureIndicator = "BlockManager$ReplicationMonitor"; 
    std::string newLogIndicator = "Method Entry";   // start new log
    std::string arg_value = "-1";
    if(argc>=4){
        failureIndicator = argv[3];
        std::cout << "Indicator: " << failureIndicator << std::endl;
    } //"";
    if(argc>=5){
        newLogIndicator = argv[4];
    }
    
    std::string line; 
    if(what_to_do == TRACE){    ////// TODO = 1 //// PRINT STACK TRACE ///////////////////////////
        std::cout << "______" << std::endl;
        std::cout << "stack trace of thread " << failureIndicator << ": " << std::endl;
        bool next = false; bool found = false;
        std::string::size_type temp_id;
        
        Trace failedtrace = Trace();
        while(std::getline(file1, line)){
            temp_id = line.find("Start Stack Trace");
            if(temp_id != std::string::npos){
                line = line.substr(20);
                temp_id = line.find(failureIndicator);
                if(temp_id != std::string::npos){
                    found = true; failedtrace.fail = true;  // found failed stack trace
                    temp_id = line.find("]");
                    if(temp_id != std::string::npos){
                        line = line.substr(0, temp_id);
                        failedtrace.thread = line;
                    }
                }
            }
            if(found){
                temp_id = line.find("Stack trace for thread ");
                if(temp_id != std::string::npos){
                    continue;
                }
                temp_id = line.find("End Stack Trace");
                if(temp_id != std::string::npos){
                    break;
                }
                failedtrace.lines.push_back(line);
            }
        }
        if(!found){
            std::cout << "Did not find stack trace of " << failureIndicator << std::endl;
        }else{
            failedtrace.print();
            if(failedtrace.lines.size()>2){
                std::cout << "caller: " << std::endl;   
                std::cout << failedtrace.lines[2] << std::endl;
            }
        }
        
        return 0;
    }
    else if(what_to_do == ARG){ ///// TODO = 2 /////////// VARIABLE //////////////////////////////////
        std::cout << "searching for argument value " << arg_value << std::endl;
        bool found = false;
        std::string arg_value = ""; 
        std::string::size_type temp_id;
        Log* log = new Log(); int idx = 0;
        while(std::getline(file1, line)){
            std::string::size_type temp_id = line.find("Stack Trace");
            if(temp_id != std::string::npos){ continue; } // stack trace, ignore
            temp_id = line.find("BM");
            if(temp_id == std::string::npos){ continue; } // no BM, is stack trace, ignore
            log->to_parse.push_back(line);
            log->parseNextLine();
            temp_id = line.find("Target");
            if(temp_id != std::string::npos){
                temp_id = line.find(failureIndicator);
                if(temp_id != std::string::npos){
                    Event* e = log->getEvent(idx);
                    if(e != nullptr) {
                        arg_value = e->value;
                        e->loopId = 1;
                        found = true;
                    }
                }
                
            }
            idx++;
        }
        if(!found){
            std::cout << "______" << std::endl;
            std::cout << "did not find value as target" << std::endl;
            return 0;
        }
        found = false;
        std::cout << "______" << std::endl;
        for(Event* e : log->parsed){
            if(e != nullptr && e->value==arg_value && e->loopId!=1){
                std::cout << "first log with value " << arg_value << ": " << std::endl;
                e->print(); 
                std::cout << std::endl;
                found = true;
                break;
            }
        }
        if(!found){
            std::cout << "Did not find logs with argument value " << arg_value << std::endl;
        }
        return 0;
    }
    
    ////////////////////////////// TEMP
    std::ofstream file2("logs/production_ID7.txt");
    if (!file2.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return 1;
    }
    
    std::vector<Log*> succeeds; // /////////// FIND DIVERGENCE //////////////////////////////////
    std::vector<Log*> fails;
    // std::cout << "HERE" << std::endl;
    std::unordered_map<std::string, Log*> threads; // newest log from that thread
    // int num_fails = 0;
    int num_threads = 0; bool fail_encountered = false;
    Log* log = nullptr;
    
    std::string prev_thread = "org.apache.hadoop.hdfs.server.blockmanagement.BlockManager$ReplicationMonitor@72bc6553";
    while(std::getline(file1, line)){
        bool newLog = false; std::string thread = ""; 
        // std::cout << line << std::endl; 
        std::string::size_type temp_id = line.find("Stack Trace");
        if(temp_id != std::string::npos){ continue; } // stack trace, ignore
        temp_id = line.find("BM");
        if(temp_id == std::string::npos){ continue; } // no BM, is stack trace, ignore
        
        line = line.substr(5); // get rid of [BM][
        
        temp_id = line.find("]");
        if(temp_id != std::string::npos){ 
            thread = line.substr(0, temp_id); // thread name
        }
        
        if(threads.find(thread) == threads.end()){ // new thread
            // threads[thread] = num_threads; num_threads++;
            newLog = true;
            std::cout << "new thread: " << thread << std::endl;
        }
        
        temp_id = line.find(newLogIndicator);
        if(temp_id != std::string::npos){ // new Log
            newLog = true;
            // if(fail) {num_fails++;}
        }
        
        if(newLog){
            if(log != nullptr){ // push back the previous log
                if(log->fail){
                    fails.push_back(log);
                    file2 << "[BM][" << prev_thread << "]ID=7,\n"; ///////// TEMP
                    prev_thread = thread;
                }else{
                    succeeds.push_back(log);
                }
            }
            
            // std::cout << "new log! " << "fail: " << fail << std::endl;
            log = new Log();
            log->fail = true; /////// TEMP
            // log->loopIds = loopIds; 
            // log->loopStartIds = loopStartIds; log->loopIds_count = loopStartIds.size() + 1; log->parentLoop = parentLoop;
            // log->init_contexts(loopStarts);
            threads[thread] = log; // update current log for that thread
            num_threads++;
        }else{
            // log = threads[thread]; // current log of that thread
        }
        
        if(log==nullptr){
            std::cout << "null" << std::endl;
            continue;
        }
        
        if(line.find(failureIndicator) != std::string::npos){ // failed run
            // fail_encountered = true;
            //log->fail = true;
        }
        
        ///////////////////// TEMP
        if(line.find("ID=6,0") != std::string::npos){
            log->fail = false;
        }
        ////////////////////// TEMP
        
        log->to_parse.push_back(line);
        file2 << "[BM][" << line << "\n"; /////////////////// TEMP
        // std::cout << line << ": thread # " << thread << " fail: " << fail << std::endl;
    }
    if(log != nullptr){ // push back the previous log
        if(log->fail){
            fails.push_back(log);
            file2 << "[BM][" << prev_thread << "]ID=7,\n"; ///////// TEMP
        }else{
            succeeds.push_back(log);
        }
    }
    // std::cout << "# logs " << logs.size() << std::endl;
    
    if(fails.size()==0){
        std::cout << "did not find log for failure runs" << std::endl;
        return 1;
    }
    
    
    std::cout << "# of fails: " << fails.size() << std::endl;
    std::cout << "# of succs: " << succeeds.size() << std::endl;
    file1.close(); file2.close(); return 0;
    
    
    int k = 0;
//    DONE collecting the log, start comparing 
    std::cout << std::endl;
    if(what_to_do == DIV){ 
        std::cout << "comparing logs" << std::endl;
        auto result = logCompare(fails[k], succeeds); //// COMPARE ////////////////
        int length = result.second.size();
        std::cout << "length: " << (length) << ". ";
        std::cout << "______" << std::endl;
            
        if( length==fails[k]->parsed.size() && length==result.first->parsed.size() ){
            std::cout << "prefix: " << std::endl;
            for(int i=0; i<result.second.size(); i++){
                std::cout << result.second[i].idx << ":ID=" << result.second[i].lineNum << " ";
            }std::cout << std::endl << std::endl;
              std::cout << "No divergence" << std::endl;
              
        }else{

            std::cout << "Diverge at: " ; 
            if(length==0){ // div at the beginning
                if(fails[k]->parsed.size()>1 && fails[k]->getEvent(0)->lineNum==-1){
                  std::cout << "HERE" << std::endl;
                  fails[k]->getEvent(1)->print();
                }else{
                    fails[k]->getEvent(0)->print();
                }
            }else if(length==1){
                if(fails[k]->parsed.size()>1 && result.second.back().lineNum==-1){ // the first one is entry
                    fails[k]->getEvent(1)->print(); // print the first one after the entry
                }else{
                    result.second.back().print();
                }
            }else {
                result.second.back().print();
            }
            std::cout << std::endl;
        }
    }
    return 0;
}

/* seenIds
Failed Run: 
L2 val=3 L3 true L4 false L3 true L4 false L3 true L4 false L6 true L7 fail 
Successful Runs: 
Run 0: L2 val=3 L3 true L4 true L5 val=2 L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
Run 1: L2 val=3 L3 true L4 trueseenIds L5 val=2 L3 true L4 false L3 true L4 false L3 true L4 true L5 val=1 L3 true L4 true L5 val=0 L6 false 
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

std::pair<Log*, std::vector<Event>> logCompare(Log* failed, std::vector<Log*> succeeds){
    std::vector<Event> prefix; // longest common prefix
    if(succeeds.size()==0) {return std::make_pair(nullptr, prefix);}
    int max_length = 0; // longest prefix length
    int max_total = 0;
    int max_idx = 0;

    std::pair<Log*, std::vector<Event>> out;
    
    for(int i=0; i<succeeds.size(); i++){
        // int length = compare_one_log(failed, succeeds[i]);
        // std::cout << "comapring " << i << std::endl;
        // auto result = compare_log_contexts(failed, succeeds[i]);
        auto result = compare_log_maploops(failed, succeeds[i]);
        int length = result.first;
        if(length > max_length || (length == max_length && succeeds[i]->parsed.size() > max_total) ){
            out.first = succeeds[i];
            out.second = result.second;
            max_length = length; // update max length
            max_idx = i; // the run index in vector woth longest common prefix
            max_total = succeeds[i]->parsed.size();
        }
    }
    
    // int i = 0; Log lon(succeed_str[max_idx]); 
    // while(i<lon.current || !lon.parsedAll()){
    //     Event* es = lon.getEvent(i);
    //     if(es == nullptr){
    //         break;
    //     }else{        
    //         longest.push_back(*es);
    //     }
    //     i++;
    // }
    return out; //std::make_pair(max_length, succeeds[max_idx]);
} 
 
