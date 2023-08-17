#include <iostream>
#include <sstream> 
#include <fstream>
#include <string> 
#include <cstdio>

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#define DIV 0
#define TRACE 1
#define ARG 2

bool process_normal(std::string file_path, std::string out_path, std::string succeed_marker, int fail_id);
bool process_trace(std::string file_path, std::string out_path, std::string succeed_marker, int fail_id);

int main (int argc, char *argv[]){    ////////////////////////////////////////////////////////////////////

    std::string step = "1";
    if(argc>=2){         // ./process {step}
        step = argv[1];    
    } 
    std::string todo = "";
    if(argc>=3){         // ./process {step}
        todo = argv[2];    
    } 

    std::string file_path = "prod_log/step" + step + "_100.log";
    std::string out_path = "prod_log/step" + step + "_99.log";

    std::string succeed_marker = "ID=100,0"; 
    int fail_id = 99;
    
    if(todo=="trace"){
        process_trace(file_path, out_path, succeed_marker, fail_id);
    }else{
        process_normal(file_path, out_path, succeed_marker, fail_id);
    }
    return 0;

}

bool process_normal(std::string file_path, std::string out_path, std::string succeed_marker, int fail_id){
    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return false;
    }
    std::ofstream file2(out_path);
    std::cout << "output path: " << file_path << std::endl;
    if (!file2.is_open()) {
        std::cout << "Failed to create " << out_path << std::endl;
        return false;
    }

    // std::string failureIndicator = "ID=100,0"; 
    std::string newLogIndicator = "Method Entry";   // start new log

    int num_logs = 0;
    int num_succeed = 0;;

    std::string line; 
    int k = 0; bool succeed = false; bool newLog = false;
    std::string prev = "";
    std::string thread = ""; std::unordered_set<std::string> threads;
    bool start = true;
    while(std::getline(file1, line)){
        // std::cout << line << std::endl; 
        std::string::size_type temp_id = line.find("Stack Trace");
        if(temp_id != std::string::npos){ continue; } // stack trace, ignore
        temp_id = line.find("BM");
        if(temp_id == std::string::npos){ continue; } // no BM, is stack trace, ignore
        
        temp_id = line.substr(5).find("]");
        if(temp_id != std::string::npos){ 
            thread = line.substr(5, temp_id); 
            if(start){
                prev = thread;
            }
        }
        
        if(threads.find(thread) == threads.end()){ // new thread
            newLog = true;
            std::cout << "new log " << k << ": " << line << std::endl;
            threads.insert(thread);
        }
        
        temp_id = line.find(newLogIndicator);
        if(temp_id != std::string::npos){ // new Log
            newLog = true;
            std::cout << "new log " << k << ": " << line << std::endl;
            num_logs++;
        }

        temp_id = line.find(succeed_marker);
        bool is_succeed_marker = (temp_id != std::string::npos);
        if(is_succeed_marker){ // succeed
            succeed = true;
            num_succeed ++;
        }

        if(newLog){
            k++;
            if(!start && !succeed){
                file2 << "[BM][" << prev << "]ID=" << fail_id << ",\n";
            }
            prev = thread;
            succeed = false;
            newLog = false; 
            start = false;
        }else{
            // log = threads[thread]; // current log of that thread
        }
        
        if( !is_succeed_marker){
            file2 << line << "\n";
        }
    }
    if(!succeed){
        file2 << "[BM][" << prev << "]ID=" << fail_id << ",\n";
    }
    
    
    file1.close(); 
    file2.close(); 
    std::cout << "num succeed: " << num_succeed << std::endl;
    std::cout << "num fail: " << (num_logs-num_succeed) << std::endl;
    std::cout << "num logs: " << num_logs << std::endl;
    return true;
}

bool process_trace(std::string file_path, std::string out_path, std::string succeed_marker, int fail_id){
    std::ifstream file1(file_path);
    std::cout << "file path: " << file_path << std::endl;
    if (!file1.is_open()) {
        std::cout << "Failed to open logs." << std::endl;
        return false;
    }
    std::ofstream file2(out_path);
    std::cout << "output path: " << file_path << std::endl;
    if (!file2.is_open()) {
        std::cout << "Failed to create " << out_path << std::endl;
        return false;
    }

    std::string traceStartIndicator = "Start Stack Trace";   // start new log
    std::string traceEndIndicator = "End Stack Trace"; 

    int num_logs = 0;
    int num_succeed = 0;;
    int num_fails = 0; 

    std::string line; 
    int k = 0; bool succeed = false; 
    std::string prev = "";
    std::string thread = ""; 
    std::unordered_map<std::string, bool> threads_succeed;

    bool start = true; 
    bool after = false; bool newTrace = false;
    
    while(std::getline(file1, line)){
        // std::cout << line << std::endl; 
        std::string::size_type temp_id = line.find(traceStartIndicator);
        newTrace = false;
        if(temp_id != std::string::npos){
            std::string thread = line.substr(temp_id+traceStartIndicator.length()+2);
            temp_id = thread.find("]");
            if(temp_id != std::string::npos){
                thread = thread.substr(0, temp_id);
            }
            after = false; 
            newTrace = true; num_logs++;
            if(threads_succeed.find(thread)!=threads_succeed.end()){
                if(threads_succeed[thread] == false){
                    file2 << "[BM][" << thread << "]ID=" << fail_id << "\n";
                    num_fails ++;
                }
                threads_succeed.erase(thread);
            }
        }

        temp_id = line.find(traceEndIndicator);
        if(temp_id != std::string::npos){
            std::string thread = line.substr(temp_id+traceEndIndicator.length()+2);
            temp_id = thread.find("]");
            if(temp_id != std::string::npos){
                thread = thread.substr(0, temp_id);
            }
            after = true;
            prev = thread;
            threads_succeed[thread] = false;
        }

        temp_id = line.find(succeed_marker);
        bool is_succeed_marker = (temp_id != std::string::npos);
        if(is_succeed_marker){ // succeed
            temp_id = line.substr(5).find("]");
            if(temp_id != std::string::npos){ 
                thread = line.substr(5, temp_id); 
                if(threads_succeed.find(thread)!=threads_succeed.end()){
                    if(threads_succeed[thread] == false){
                        threads_succeed[thread] = true;
                        num_succeed ++;
                    }
                }
            }
        }

        // if( !is_succeed_marker){
            file2 << line << "\n";
        //}
    }
    for(auto it : threads_succeed){
        if(it.second == false){
            file2 << "[BM][" << it.first << "]ID=" << fail_id << "\n";
            num_fails++;
        }
    }
    
    file1.close(); 
    file2.close(); 
    num_logs = (int)(num_logs/2);
    std::cout << "num succeed: " << num_succeed << std::endl;
    std::cout << "num fail: " << num_fails << std::endl;
    std::cout << "num logs: " << num_logs << std::endl;
    if ((num_fails+num_succeed)!=num_logs){
        std::cout << "not match" << std::endl;
        return false;
    }
    return true;
}
