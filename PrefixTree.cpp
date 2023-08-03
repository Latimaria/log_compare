#include "PrefixTree.hpp"

bool TrieNode::operator== (const TrieNode& rhs) const{
    return (lineNum == rhs.lineNum);
}
bool TrieNode::operator!= (const TrieNode& rhs) const{
    return !(this->operator==(rhs));
}
    
Trie::Trie(){
    root = nullptr;
}
Trie::Trie(std::string name) : entry(name){
    root = nullptr;
}
Trie::~Trie(){
}
void Trie::print_Trie(){
    if(root==nullptr){
        std::cout << "root is null" << std::endl;
        return;
    }
    print_subTrie(root);
}
void Trie::print_subTrie(TrieNode* node){
    if(node==nullptr){
        return;
    }
    std::cout << node->lineNum << "; children: ";
    for (auto& child : node->children) {
        std::cout << child.first << " ";
    }
    std::cout << std::endl;
    for (auto& child : node->children) {
        print_subTrie(child.second);
    }
}
bool Trie::insertLog(Log* log, int idx) {
    if(log==nullptr || log->parsed.size()==0 || log->parsed[0]==nullptr){
        return false;
    }
    if(root==nullptr){
        root = new TrieNode(log->parsed[0]->lineNum);
        entry = log->entry;
        std::cout << "entry: " << entry << std::endl;
    }
    loopStartIds.insert(log->loopStartIds.begin(), log->loopStartIds.end());
    parentLoop.insert(log->parentLoop.begin(), log->parentLoop.end());
    
    TrieNode *current = root;
    if(current==nullptr){
        std::cout << "null root" << std::endl;
    }
    
    bool start_loop = false;
    for (int i = 1; i < log->parsed.size(); i++) {
        int lineNum = -1;
        Event* e = log->getEvent(i);
        if(e==nullptr){
            std::cout << "null Event in log, idx: " << i << std::endl;
            current->isEndOfLog = true;
            return false;
        }else{
            lineNum = e->lineNum;
        }
        
        if(e->loopId>-1 && )
        if (current->children.find(lineNum) == current->children.end()){
            current->children[lineNum] = new TrieNode(lineNum);
        }
        
        current = current->children[lineNum];
        
        if(loopStartIds.find(lineNum) != loopStartIds.end()){
            start_loop = true;
        }else{
            start_loop = false;
        }
    }

    current->isEndOfLog = true;
    current->endLogIdx = idx;
    return true;
}
//typedef struct return_prefix {
//    Log* failed;
//    bool div;
//    int length;
//    std::vector<Event> prefix;
//} Prefix;

TriePrefix compareTries(Trie* fail, Trie* succeed){
    TriePrefix current = TriePrefix(); 
    if(fail==nullptr || succeed==nullptr || fail->entry!=succeed->entry){
        return current; 
    }
    std::queue<TriePair> q;
    q.push({fail->root, succeed->root, 1});
    // int length = 0; // current longest 
    // // std::vector<int> current; // lineNums
    while(!q.empty()){
        TriePair node = q.front();
        q.pop();
        if(node.eventA==nullptr || node.eventB==nullptr){
            continue;
        }
        if (*(node.eventA) == *(node.eventB)) {
            if (node.depth > current.length) {
                current.length = node.depth;
                current.prefix.push_back(node.eventA->lineNum);
                if(node.eventA->isEndOfLog && node.eventB->isEndOfLog){
                    current.div = false;     
                }else{
                    current.div = true;
                }
            }

            for (auto& childA : node.eventA->children) {
                for (auto& childB : node.eventB->children) {
                    if (childA.first == childB.first) {
                        q.push({childA.second, childB.second, node.depth+1});
                    }
                }
            } // enque
        }
    }
    return current;
}
