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
#include "PrefixTree.hpp"

#define DIV 0
#define TRACE 1
#define ARG 2
#define ECOMPARE -3
#define ENOOPEN -2

int find_divergence(std::string file_path, std::string failureIndicator, std::string newLogIndicator);
int find_caller(std::string file_path, std::string failureIndicator, std::string caller_for);
int find_value(std::string file_path, std::string failureIndicator, std::string read_method);

int main (int argc, char *argv[]){    ////////////////////////////////////////////////////////////////////
    
    // std::string file_path = "logs/step1a2.log";
    std::string file_path = "prod_log/step1_99.log";
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
        return ENOOPEN;
    }

    
    // std::string failureIndicator = "BlockManager$ReplicationMonitor"; // using thread name for now
    
    int failure_id = 99;
    std::string failureIndicator = "ID=" + std::to_string(failure_id); 
    std::string newLogIndicator = "Method Entry";   // start new log
    std::string arg_value = "-1"; std::string caller_for = ""; std::string read_method = "";
    if(argc>=4){
        failureIndicator = argv[3];
        std::cout << "Indicator: " << failureIndicator << std::endl;
    } //"";
    if(argc>=5){
        newLogIndicator = argv[4];
        if(what_to_do==TRACE){
            caller_for = argv[4];
        }else if(what_to_do==ARG){
            read_method = argv[4];
        }
    }
    
     
    if(what_to_do == TRACE){    ////// TODO = 1 //// PRINT STACK TRACE ///////////////////////////
        
        return find_caller(file_path, failureIndicator, caller_for);
    
    }else if(what_to_do == DIV){

        return find_divergence(file_path, failureIndicator, newLogIndicator);
    
    }else if(what_to_do == ARG){ ///// TODO = 2 /////////// VARIABLE //////////////////////////////////

        return find_value(file_path, failureIndicator, read_method);

        std::cout << "searching for argument value of ID : " << read_method << std::endl;
        bool found = false;
        std::string arg_value = ""; 
        std::string::size_type temp_id;
        Log* log = new Log(); int idx = 0;
        std::string line;
        while(std::getline(file1, line)){
            std::string::size_type temp_id = line.find("Stack Trace");
            if(temp_id != std::string::npos){ continue; } // stack trace, ignore
            temp_id = line.find("BM");
            if(temp_id == std::string::npos){ continue; } // no BM, is stack trace, ignore
            log->to_parse.push_back(line);
            log->parseNextLine();
            temp_id = line.find(read_method);
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

}
 
int find_divergence(std::string file_path, std::string failureIndicator, std::string newLogIndicator){
    std::vector<Log*> succeeds; 
    std::vector<Log*> fails;
    std::unordered_map<std::string, Log*> threads; // newest log from that thread
    Log* log = nullptr;
    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return ENOOPEN;
    }
    std::string line;
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
                }else{
                    succeeds.push_back(log);
                }
            }
            
            // std::cout << "new log! " << "fail: " << fail << std::endl;
            log = new Log();
            // log->loopIds = loopIds; 
            // log->loopStartIds = loopStartIds; log->loopIds_count = loopStartIds.size() + 1; log->parentLoop = parentLoop;
            // log->init_contexts(loopStarts);
            threads[thread] = log; // update current log for that thread
        }else{
            // log = threads[thread]; // current log of that thread
        }
        
        if(log==nullptr){
            std::cout << "null" << std::endl;
            continue;
        }
        
        if(line.find(failureIndicator) != std::string::npos){ // failed run
            log->fail = true;
        }

        log->to_parse.push_back(line);
        // std::cout << line << ": thread # " << thread << " fail: " << fail << std::endl;
    }
    if(log != nullptr){ // push back the previous log
        if(log->fail){
            fails.push_back(log);
        }else{
            succeeds.push_back(log);
        }
    }
    // std::cout << "# logs " << logs.size() << std::endl;
    
    if(fails.size()==0){
        std::cout << "did not find log for failure runs" << std::endl;
        return ECOMPARE;
    }
    
    
    std::cout << "# of fails: " << fails.size() << std::endl;
    std::cout << "# of succs: " << succeeds.size() << std::endl;
    file1.close(); 
    
    std::cout << "/////////////////////////" << std::endl;
    
    Trie* fail = new Trie();
    Trie* succeed = new Trie();
    for(int i=0; i<fails.size(); i++){
        fails[i]->parseAll();
        fail->insertLog(fails[i], i);
    }

    for(int i=0; i<succeeds.size(); i++){
        succeeds[i]->parseAll();
        succeed->insertLog(succeeds[i], i);
    }
    std::cout << "inserted " << std::endl;
    std::cout << "fail: " << std::endl;
    fail->print_Trie(); std::cout << "/////" << std::endl;
    std::cout << "succeed: " << std::endl;
    succeed->print_Trie(); std::cout << "/////" << std::endl;
    

    TriePrefix result = compareTries(fail, succeed);
    std::cout << "div: " << result.div << std::endl;
    std::cout << "length: " << result.length << std::endl;
    std::cout << "vec (size= " << result.prefix.size() << "): " << std::endl;
    for(int i=0; i<result.prefix.size(); i++){
        std::cout << "ID=" << result.prefix[i] << " ";
    }
    std::cout << std::endl;
    if(result.div){
        std::cout << "div at " << result.prefix.back() << std::endl;
        return result.prefix.back();
    }
    else{
        std::cout << "no divergence" << std::endl;
        for (auto& child : fail->root->children) {
            std::cout << "continue at " << child.first << std::endl;
            return child.first;
        }
    }
    std::cout << std::endl;
    return ECOMPARE;
}

std::vector<int> instrumentationLineNumbers;

int find_caller(std::string file_path, std::string failureIndicator, std::string caller_for){

    std::cout << "use stack trace" << std::endl;
    std::cout << "failureIndicator " << failureIndicator << std::endl;
    std::string traceStartIndicator = "Start Stack Trace";   // start new log
    std::string traceEndIndicator = "End Stack Trace"; 

    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return ENOOPEN;
    }

    std::string::size_type temp_id;
    
    std::vector<Trace*> failed_traces;
    std::unordered_map<std::string, Trace*> thread_traces;
    Trace* current_trace;
    bool tracing = false;

    std::string line;
    while(std::getline(file1, line)){
        temp_id = line.find(traceStartIndicator);
        if(temp_id != std::string::npos){
            std::string thread = line.substr(temp_id+traceStartIndicator.length()+2);
            temp_id = thread.find("]");
            if(temp_id != std::string::npos){
                thread = thread.substr(0, temp_id);
            }
            current_trace = new Trace(thread); current_trace->fail = false;
            thread_traces[thread] = current_trace;
            tracing = true;
            continue;
        }

        temp_id = line.find(traceEndIndicator);
        if(temp_id != std::string::npos){
            std::string thread = line.substr(temp_id+traceEndIndicator.length()+2);
            temp_id = thread.find("]");
            if(temp_id != std::string::npos){
                thread = thread.substr(0, temp_id);
            }
            tracing = false;
        }

        temp_id = line.find(failureIndicator); // "ID=99"
        if(temp_id != std::string::npos){
            std::string thread = line.substr(5);
            temp_id = thread.find("]");
            if(temp_id != std::string::npos){
                thread = thread.substr(0, temp_id);
            }
            if(thread_traces.find(thread)!=thread_traces.end()){
                if(thread_traces[thread]->fail==false){
                    thread_traces[thread]->fail = true;
                    failed_traces.push_back(thread_traces[thread]);
                }
            }
        }

        if(tracing){
            current_trace->lines.push_back(line);
        }
    }
    std::cout << "# failed: " << failed_traces.size() << std::endl;
    if(failed_traces.size()==0){
        std::cout << "Did not find stack trace of " << failureIndicator << std::endl;
        return ECOMPARE;
    }
    
    for(int i=0; i<failed_traces.size(); i++){
        std::cout << "trace " << i << std::endl ;
        failed_traces[i]->print();
        std::cout << std::endl;
    }
    for(int i=0; i<failed_traces.size(); i++){
        std::cout << "trace " << i << std::endl ;
        Caller caller = failed_traces[i]->find_caller(caller_for);
        if(caller.valid){
            std::cout << "caller of " << caller_for << ": " << std::endl;
            std::cout << caller.function_name << ", line: " << caller.line_number << std::endl;
        }else{
            std::cout << "did not find caller of " << caller_for << std::endl;
        }
    }
    return ECOMPARE;
}

int find_value(std::string file_path, std::string failureIndicator, std::string read_method){
    std::cout << "searching for argument value of ID : " << read_method << std::endl;

    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return ENOOPEN;
    }

    bool found = false;
    std::string arg_value = ""; 
    std::string::size_type temp_id;
    // Log* log = new Log(); 
    std::string line; std::vector<std::string> lines;

    while(std::getline(file1, line)){
        std::string::size_type temp_id = line.find("Stack Trace");
        if(temp_id != std::string::npos){ continue; } // stack trace, ignore
        temp_id = line.find("BM");
        if(temp_id == std::string::npos){ continue; } // no BM, is stack trace
        lines.push_back(line);
    }
    file1.close();

    std::cout << "#lines: " << lines.size() << std::endl;
    if(lines.size()==0){
        std::cout << "no log" << std::endl;
        return ECOMPARE;
    }
    int ID=ECOMPARE; std::vector<int> IDs;
    std::unordered_set<std::string> threads_fails;
    std::unordered_map<std::string, int> value_lines; // value to line number

    
    for(int i=lines.size()-1; i>=0; i-=1){
        line = lines[i];
        // std::cout << i << ": " << line << std::endl;

        temp_id = line.find(failureIndicator);
        if(temp_id != std::string::npos){
            line = line.substr(5); // get rid of [BM][
            temp_id = line.find("]");
            if(temp_id != std::string::npos){ 
                std::string thread = line.substr(0, temp_id); // thread name
                threads_fails.insert(thread);
            }
        }

        temp_id = line.find(read_method);
        if(temp_id != std::string::npos){
            line = line.substr(5); // get rid of [BM][
            temp_id = line.find("]");
            if(temp_id != std::string::npos){ 
                std::string thread = line.substr(0, temp_id); // thread name
                if (threads_fails.find(thread) != threads_fails.end()){
                    found = true;
                    temp_id = line.find(","); 
                    if(temp_id != std::string::npos){ 
                        std::string value = line.substr(temp_id+1);
                        threads_fails.erase(thread);
                        value_lines.insert({value, i});
                        std::cout << "add target value " << value << std::endl;
                    }
                }
            }
            continue;
        }

        temp_id = line.find(","); 
        if(temp_id != std::string::npos){ 
            std::string value = line.substr(temp_id+1);
            if(value_lines.find(value)!=value_lines.end()){
                std::cout << "target value found: " << value << std::endl;
                line = line.substr(0, temp_id);
                temp_id = line.find("ID="); 
                if(temp_id != std::string::npos){ 
                    std::string id_str = line.substr(temp_id+3);
                    ID = std::stoi(id_str); IDs.push_back(ID);
                    std::cout << "ID=" << ID << ", " << value << std::endl;
                }
            }
        }
    }

    if(!found){
        std::cout << "______" << std::endl;
        std::cout << "did not find value as target" << std::endl;
        return ECOMPARE;
    }

    if(ID==ECOMPARE){
        std::cout << "did not find target value" << std::endl;
    }
    std::cout << "ID selected: " << ID << std::endl;
    return ID;
}