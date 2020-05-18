/*
version：0.2.0
name : alviney wong
方法 dfs，使用vector判断
加入mmap写入
做ID映射 ：速度变慢了
用pair读取数据并排序
能用数组就用数组，效率比vector快的多
vector比unordermap快很多
*/

#define TEST
//#include<bits/stdc++.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <thread>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <unordered_map>
#include <set>
#include <string>
#include <queue>
#include <algorithm>
using namespace std;
int maxdeg;
struct comp{
    bool operator () (const vector<int>& a,const vector<int>& b)const{
        if(a.size() == b.size()){
            for(int i = 0; i < a.size(); i++){
                if(a[i] == b[i]) continue;
                return a[i] < b[i];
            }
        }
        else return a.size() < b.size();
        return false;
    }
};
class SolveGraph{

public:
    SolveGraph();
    void loadTestData(const string&  testFile);
    void writeResultFile_fprintf(const string&  resultFile);
    void writeResultFile_mmap(const string&  resultFile);
    void preprocess();
    void solve();
    static bool cmp(const pair<int, int> &a, const pair<int, int> &b);
    static bool cmmp(const vector<int> &a, const vector<int> &b);


private:
    //unordered_map<int, vector<int>> map;
    vector<vector<int>> map;
    vector<vector<int>> invmap;
    vector<unordered_map<int, vector<int>>> covmap;
    vector<unordered_map<int, vector<int>>> covmap1;
    unordered_map<int, int> hash_key;
    //vector<int> inverse_hash;
    int vals[300000];

    //vector<int> inputVal;
    //vector<pair<int, int>> inputs;
    int inputs[600000];
    int res_num[8];
    set<vector<int>, comp> res;
    //vector<vector<int>> res;
    vector<string> midwrite;
    vector<string> endwrite;
    int *sevenIndex;
    int *sixIndex;
    //int MAX_ID;
    int id_num;
    int edge_num;
    int ver_num;

    void backtrack(vector<int> &list, bool *visited, int depth, int key);
    void degreeSort(int *degrees, int *outdegrees);
    void covbuild();

};

SolveGraph::SolveGraph(){
    
    //MAX_ID = 0;
    id_num = 0;
    edge_num = 0;
    ver_num = 0;
    memset(inputs, 0, sizeof(inputs));
    memset(vals, 0, sizeof(vals));
    memset(res_num, 0, sizeof(res_num));
    //vals.reserve(200000);
#ifdef TEST
    printf("initialize\n");
#endif
}


void SolveGraph::loadTestData(const string& testFile){
    int start = clock();
    //inputs.reserve(200000);
    struct stat sb;
    char *buf;
    int fd = open(testFile.c_str(), O_RDONLY);
    fstat(fd, &sb);
    buf = (char*)mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    int col = 0;
    int tmp = 0;
    //int vert = 0;
    int arg[2] = {0, 0};
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        if(*p == ','){
            arg[col] = tmp;
            //inputVal.push_back(tmp);
            inputs[ver_num] = tmp;
            ver_num++;
            //MAX_ID = MAX_ID > tmp ? MAX_ID : tmp;//最大ID值
            col++;
            tmp = 0;
            continue;
        }
        if(*p == '\r'){//cout << arg[0] << arg[1] << tmp << endl;
            //map[arg[0]].push_back(arg[1]);
            //inputs.push_back(make_pair(arg[0],arg[1]));
            vals[edge_num] = tmp;
            tmp = 0;
            col = 0;
            p++;
            edge_num++;
            continue;
        }
        if(*p == '\n'){//cout << arg[0] << arg[1] << tmp << endl;
            //map[arg[0]].push_back(arg[1]);
            //inputs.push_back(make_pair(arg[0],arg[1]));
            vals[edge_num] = tmp;
            tmp = 0;
            col = 0;
            edge_num++;
            continue;
        }
        tmp = tmp * 10 + *p - '0';
    }
    
    close(fd);
    munmap(buf, sb.st_size);

#ifdef TEST
    printf("the number of vert is: %d\n",ver_num);
    printf("the number of edge is: %d\n",edge_num);
    printf("the load file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::writeResultFile_fprintf(const string& resultFile){
    int writefile = clock();

    FILE *fd = fopen(resultFile.c_str(), "w");
    fprintf(fd, "%lu\n", res.size());
    for(const auto & re : res){
        int j;
        for(j = 0; j < re.size() - 1; j++){
            fprintf(fd, "%s", midwrite[re[j]].c_str());
        }
        fprintf(fd, "%s", endwrite[re[j]].c_str());
    }
    fclose(fd);
#ifdef TEST
    printf("the size of result is: %lu\n",res.size());
    printf("the fprintf file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::writeResultFile_mmap(const string& resultFile){
    int writefile = clock();
    int fd = open(resultFile.c_str(),O_CREAT|O_RDWR,0666);
    //TODO：注意文件大小的判断，可能要改
    //long long pagesize = res.size() * 20 * 7 / 4096 + 1;
    ftruncate(fd, res.size() * 20 * 7);
    char* start = (char*)mmap(nullptr,res.size() * 20 * 7,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
    long long offerlen = 0;
    string s = to_string(res.size()) + '\n';
    strcpy(start,s.c_str());
    offerlen = s.size();
    for(auto it = res.begin(); it != res.end(); it++){
        int j;
        s.clear();
        for(j = 0; j < (*it).size() - 1; j++){
            s += midwrite[(*it)[j]];
        }
        s += endwrite[(*it)[j]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }
    ftruncate(fd, offerlen);
    close(fd);
    munmap(start, res.size() * 20 * 7);
#ifdef TEST
    printf("the size of result is: %lu\n",res.size());
    printf("the mmap file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::preprocess(){
    int runtime = clock();
    //映射ID
    // vector<int> idvec = inputVal;
    // sort(idvec.begin(), idvec.end());
    // idvec.erase(unique(idvec.begin(), idvec.end()), idvec.end());
    int tmp[ver_num];
    for(int i = 0; i < ver_num; i++){
        tmp[i] = inputs[i];
    }
    sort(tmp, tmp + ver_num);
    id_num = unique(tmp, tmp + ver_num) - tmp;
    // vector<int> idvectmp(MAX_ID + 1, 0);
    // sort(inputs.begin(), inputs.end(), cmp);
    // for(auto par : inputs){
    //     idvectmp[par.first] = 1;
    //     idvectmp[par.second] = 1;
    // }
    // vector<int> idvec;
    // for(int i = 0; i < idvectmp.size(); i++){
    //     if(idvectmp[i]) idvec.push_back(i);
    // }
    // idvectmp.clear();
    // idvectmp.shrink_to_fit();
    // id_num = idvec.size();
    //int hash_index = 0;
    midwrite.resize(id_num);
    endwrite.resize(id_num);
    for(int i = 0; i < id_num; i++){
        int key = tmp[i];
        hash_key[key] = i;
        //inverse_hash.push_back(key);
        midwrite[i] = to_string(key) + ',';
        endwrite[i] = to_string(key) + '\n';
    }
    // idvec.clear();
    // idvec.shrink_to_fit();
#ifdef TEST 
    printf("the number of id: %d\n", id_num);
#endif

    map.resize(id_num);
    invmap.resize(id_num);
    //计算出度入度
    // int inDegrees[id_num];
    // int outDegrees[id_num];
    // memset(inDegrees, 0, sizeof(inDegrees));
    // memset(outDegrees, 0, sizeof(outDegrees));
    // inDegrees = vector<int>(id_num, 0);
    // outDegrees = vector<int>(id_num, 0);
    int *inDegrees = new int[id_num]();
    int *outDegrees = new int[id_num]();
    for(int i = 0; i < ver_num; i += 2){
        int n1 = hash_key[inputs[i]];
        int n2 = hash_key[inputs[i + 1]];
        map[n1].push_back(n2);
        invmap[n2].push_back(n1);
        outDegrees[n1]++;
        inDegrees[n2]++;
    }
    // inputs.clear();
    // inputs.shrink_to_fit();
    //degreeSort(inDegrees);
    degreeSort(inDegrees, outDegrees);

    covbuild();
#ifdef TEST  
    printf("the process time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::solve(){
    int back = clock();
    vector<int> path;
    int reach[id_num];
    memset(reach, -1, sizeof(reach));
    sevenIndex = reach;
    int reach1[id_num];
    memset(reach1, -1, sizeof(reach1));
    sixIndex = reach1;
    vector<int> sixfloor(1000);
    vector<int> fivefloor(1000);
    //标记存在的ID和访问路径
    bool visited[id_num];
    memset(visited, false, sizeof(visited));
    //vector<int> visited(id_num,0);
    for(int i = 0; i < id_num; ++i){//cout << id.second.size() << endl;
        if(!map[i].empty()){
            for(auto &is : covmap[i]){
                int j = is.first;
                if(!is.second.empty()){
                    sevenIndex[j] = 1;
                    sixfloor.push_back(j);
                }
            }
            for(auto &ms : covmap1[i]){
                int m = ms.first;
                if(!ms.second.empty()){
                    sixIndex[m] = 1;
                    fivefloor.push_back(m);
                }
            }
            //cout << "1: " << id.first << endl;
            backtrack(path, visited, 1, i);
            for(int j : sixfloor) sevenIndex[j] = -1;
            sixfloor.clear();
            for(int j : fivefloor) sixIndex[j] = -1;
            fivefloor.clear();        
        }
    }
#ifdef TEST
    for(int i = 3; i < 8; i++){
        printf("the %d res num is: %d\n", i, res_num[i]);
    }
    printf("the solve time is: %.3f\n", float(clock() - back) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::backtrack(vector<int> &list, bool *visited, int depth, int key){
    list.push_back(key);
    visited[key] = true;

    //key的list提取
    vector<int> &keylist = map[key];
    auto it = lower_bound(keylist.begin(),keylist.end(),list[0]);
    //if(keylist.size() == 0) return;
    if(it != keylist.end() && *it == list[0] && depth >=3 && depth < 6){
        res.insert(list);
        res_num[depth]++;
    }
    if(depth < 5){
        for(;it != keylist.end(); ++it){
            if(!visited[*it]) backtrack(list, visited, depth + 1, *it);
        }
    }
    else if(depth == 5){
        if(sevenIndex[key] != -1){
            auto ks = covmap[list[0]][key];
            for(int k : ks){
                if(visited[k]) continue;
                //vector<int> tmp(list);
                list.push_back(k);
                res.insert(list);
                res_num[6]++;
                list.pop_back();
            }      
        }
        if(sixIndex[key] != -1){
            auto hs = covmap1[list[0]][key];
            for(int idex = 0; idex < hs.size(); idex+=2){
                int h1 = hs[idex];
                int h2 = hs[idex+1];
                if(h1 == h2) continue;
                if(visited[h1] || visited[h2]) continue;
                //vector<int> tmp1(list);
                list.push_back(h1);
                list.push_back(h2);
                res.insert(list);
                res_num[7]++;
                list.pop_back();
                list.pop_back();
            }
        }

    }
    list.pop_back();
    visited[key] = false;
}

void SolveGraph::degreeSort(int *indegrees, int *outdegrees){
    queue<int> q,p;
    for(int i = 0;i < id_num; i++){
        if(indegrees[i] == 0)
            q.push(i);
        if(outdegrees[i] == 0)
            p.push(i);
    }
    while(!q.empty()){
        int key = q.front();
        q.pop();
        for(int v : map[key]) {
            if(--indegrees[v] == 0)
                q.push(v);
        }
    }
    while(!p.empty()){
        int key = p.front();
        p.pop();
        for(int v : invmap[key]) {
            if(--outdegrees[v] == 0)
                p.push(v);
        }
    }

    int cnt = 0;
    for(int i = 0; i < id_num; i++){
        if(indegrees[i] == 0 || outdegrees[i] == 0){
            map[i].clear();
            cnt++;
        }
        else{maxdeg = maxdeg > indegrees[i] ? maxdeg : indegrees[i];
            if(outdegrees[i] >= 1){
                sort(map[i].begin(),map[i].end());
            }
        }
    }
#ifdef TEST
    printf("the max deg: %d\n", maxdeg);
    printf("the number of deleNodes: %d\n", cnt);
#endif
}

void SolveGraph::covbuild(){
    covmap.resize(id_num);
    covmap1.resize(id_num);
    for(int i = 0; i < id_num; i++){
        auto mi = map[i];
        for(int k : mi){
            auto mk = map[k];
            for(int j : mk){
                if(i > j && k > j){
                    covmap[j][i].push_back(k);
                }
                auto mj = map[j];
                for(int h : mj){
                    if(i > h && j > h && k > h){
                        covmap1[h][i].push_back(k);
                        covmap1[h][i].push_back(j);
                    }
                }               
            }
        }
    }
}

bool SolveGraph::cmp(const pair<int, int> &a, const pair<int, int> &b){
    if(a.first == b.first) return a.second < b.second;
    else return a.first < b.first;
}

bool SolveGraph::cmmp(const vector<int> &a, const vector<int> &b){
    if(a.size() == b.size()){
        for(int i = 0; i < a.size(); i++){
            if(a[i] != b[i]) return a[i] < b[i];
        }
    }
    return a.size() < b.size();
}


int main(int argc, char *argv[]){
    int start = clock();
#ifdef TEST
    string testFile = "/Users/alviney/test/data/testdata/54/test_data.txt";
    string resultFile = "/Users/alviney/test/data/testdata/54/result.txt";
#endif
#ifndef TEST
    string testFile = "/data/test_data.txt";
    string resultFile = "/projects/student/result.txt";
#endif
    SolveGraph solveGraph;
    //读取数据
    solveGraph.loadTestData(testFile); 
    solveGraph.preprocess();    
    solveGraph.solve();
    //writeResultFile_fprintf(resultFile);
    solveGraph.writeResultFile_mmap(resultFile);
#ifdef TEST   
    printf("the all time is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
    exit(0);
    return 0;
}

/*
原始
the number of edge is: 271100
the load file cost is: 0.016
the number of id: 99546
the number of deleNodes: 7837
the number of deleNodes: 6175
the calculate time is: 1.185
the size of result is: 26571
the mmap file cost is: 0.006
the all time is: 1.207

剪枝
the number of edge is: 271100
the load file cost is: 0.025
the number of id: 99546
the number of deleNodes: 7837
the number of deleNodes: 6175
the back time is: 0.168
the calculate time is: 0.464
the size of result is: 26571
the mmap file cost is: 0.008
the all time is: 0.497
vector改数组
the number of vert is: 560000
the number of edge is: 280000
the load file cost is: 0.018
the number of id: 25000
the number of deleNodes: 0
the process time is: 3.020
the solve time is: 28.382
the size of result is: 3512444
the mmap file cost is: 0.506
the all time is: 31.928
使用5+2
the number of vert is: 560000
the number of edge is: 280000
the load file cost is: 0.014
the number of id: 25000
the number of deleNodes: 0
the process time is: 10.197
the solve time is: 7.057
the size of result is: 3512444
the mmap file cost is: 0.493
the all time is: 17.763
*/