/*使用lower_bound，unorder_map,修改数组*/

#define TEST
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
#include <chrono>
using namespace std;
#define THREAD_NUM 6
uint64_t vals[2000005];
uint32_t inputs[4000005];
uint32_t TMP[4000005];
int res_num[THREAD_NUM*5];
bool cmp(const pair<uint32_t,uint64_t> a, const pair<uint32_t ,uint64_t> b) {
    return a.first < b.first;
}

struct Index{
    pair<uint32_t, uint64_t> k0;//ikey3
    pair<uint32_t, uint64_t> k1;//ikey2
    pair<uint32_t, uint64_t> k2;//ikey1
    Index(pair<uint32_t, uint64_t> key0, pair<uint32_t, uint64_t> key1, pair<uint32_t, uint64_t> key2) : k0(key0), k1(key1), k2(key2) {};
    bool operator<(const Index&idx1)const{
        if(k1.first == idx1.k1.first){
            return k2.first < idx1.k2.first;
        }
        else return k1.first < idx1.k1.first;
    }
};

class SolveGraph{
public:
    SolveGraph();
    void loadTestData(const char* testFile);
    void writeResultFile_mmap(const char* resultFile);
    void preprocess();
    void runthread(int thread_num);

private:
    uint32_t *outDegrees;
    uint32_t *inDegrees;
    unordered_map<uint32_t, uint32_t> hash_key;
    vector<string> midwrite;
    vector<string> endwrite;
    uint32_t id_num;
    uint32_t edge_num;
    uint32_t ver_num;
    uint32_t outdegreesum;
    vector<vector<pair<uint32_t, uint64_t>>> map;
    vector<vector<pair<uint32_t, uint64_t>>> invmap;
    //vector<vector<Index>> keyvec[THREAD_NUM];
    
    vector<uint32_t> ans3[THREAD_NUM];
    vector<uint32_t> ans4[THREAD_NUM];
    vector<uint32_t> ans5[THREAD_NUM];
    vector<uint32_t> ans6[THREAD_NUM];
    vector<uint32_t> ans7[THREAD_NUM];

    void dbfs(uint32_t key, vector<bool> &visited, int id, unordered_map<uint32_t, vector<Index>> &keyvec);
    void solve(int id, uint32_t start, uint32_t end);
};

SolveGraph::SolveGraph():id_num(0),edge_num(0),ver_num(0),outdegreesum(0){
#ifdef TEST
    int init = clock();
    printf("initialize\n");
    printf("the init cost is: %.3f\n", float(clock() - init) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::loadTestData(const char* testFile){
#ifdef TEST
    int start = clock();
#endif
    struct stat sb;
    char *buf;
    int fd = open(testFile, O_RDONLY);
    fstat(fd, &sb);
    buf = (char*)mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    uint64_t tmp = 0;
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        if(*p >= '0' && *p <= '9'){
            tmp = tmp * 10 + *p - '0';
        }
        else if(*p == ','){
            inputs[ver_num] = tmp;
            TMP[ver_num] = tmp;
            //id_max = id_max > tmp ? id_max : tmp;
            ++ver_num;
            tmp = 0;
        }
        else if(*p == '\r'){
            vals[edge_num] = tmp;
            tmp = 0;
            p++;
            ++edge_num;
        }
        else if(*p == '\n'){
            vals[edge_num] = tmp;
            tmp = 0;
            ++edge_num;
        }
    }
    
    close(fd);
    munmap(buf, sb.st_size);

#ifdef TEST
    printf("the number of vert is: %d\n",ver_num);
    printf("the number of edge is: %d\n",edge_num);
    printf("the load file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::writeResultFile_mmap(const char* resultFile){
#ifdef TEST
    int writefile = clock();
#endif
    struct stat buffer;
    if(stat (resultFile, &buffer) != 0){
        remove(resultFile);
    }
    int fd = open(resultFile,O_CREAT|O_RDWR,0666);
    unsigned int size = 0;
    unsigned int result_num = 0;
    for(int j = 0; j < THREAD_NUM*5; ++j) size += res_num[j];
    if(size == 0) result_num = 1;
    else result_num = size;
    //TODO：注意文件大小的判断，可能要改
    ftruncate(fd, result_num * 12 * 7);
    char* start = (char*)mmap(nullptr,result_num * 12 * 7,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
    long long offerlen = 0;
    string s = to_string(size) + '\n';
    strcpy(start,s.c_str());
    offerlen = s.size();
    for(int i = 0; i < THREAD_NUM; ++i){       
        for(int j = 0; j < ans3[i].size(); j+=3){
            s = midwrite[ans3[i][j]];
            s += midwrite[ans3[i][j+1]];
            s += endwrite[ans3[i][j+2]];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    for(int i = 0; i < THREAD_NUM; ++i){       
        for(int j = 0; j < ans4[i].size(); j+=4){
            s = midwrite[ans4[i][j]];
            s += midwrite[ans4[i][j+1]];
            s += midwrite[ans4[i][j+2]];
            s += endwrite[ans4[i][j+3]];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    for(int i = 0; i < THREAD_NUM; ++i){       
        for(int j = 0; j < ans5[i].size(); j+=5){
            s = midwrite[ans5[i][j]];
            s += midwrite[ans5[i][j+1]];
            s += midwrite[ans5[i][j+2]];
            s += midwrite[ans5[i][j+3]];
            s += endwrite[ans5[i][j+4]];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    for(int i = 0; i < THREAD_NUM; ++i){       
        for(int j = 0; j < ans6[i].size(); j+=6){
            s = midwrite[ans6[i][j]];
            s += midwrite[ans6[i][j+1]];
            s += midwrite[ans6[i][j+2]];
            s += midwrite[ans6[i][j+3]];
            s += midwrite[ans6[i][j+4]];
            s += endwrite[ans6[i][j+5]];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    for(int i = 0; i < THREAD_NUM; ++i){       
        for(int j = 0; j < ans7[i].size(); j+=7){
            s = midwrite[ans7[i][j]];
            s += midwrite[ans7[i][j+1]];
            s += midwrite[ans7[i][j+2]];
            s += midwrite[ans7[i][j+3]];
            s += midwrite[ans7[i][j+4]];
            s += midwrite[ans7[i][j+5]];
            s += endwrite[ans7[i][j+6]];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    ftruncate(fd, offerlen);
    close(fd);
    munmap(start, result_num * 12 * 7);
#ifdef TEST
    printf("the size of result is: %u\n",size);
    printf("the mmap file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::preprocess(){
#ifdef TEST
    int runtime = clock();
#endif
    //映射ID
    // uint32_t *tmp = new uint32_t[ver_num];
    // for(uint32_t i = 0; i < ver_num; ++i){
    //     tmp[i] = inputs[i];
    // }
    sort(TMP, TMP + ver_num);
    id_num = unique(TMP, TMP + ver_num) - TMP;
    midwrite.resize(id_num);
    endwrite.resize(id_num);
    uint32_t key;
    for(uint32_t i = 0; i < id_num; ++i){
        key = TMP[i];
        hash_key[key] = i;
        midwrite[i] = to_string(key) + ',';
        endwrite[i] = to_string(key) + '\n';
    }
    //计算出度入度
    inDegrees = new uint32_t[id_num]();
    outDegrees = new uint32_t[id_num]();
    map.resize(id_num);
    invmap.resize(id_num);
    uint32_t n1,n2,cnt;
    for(uint32_t i = 0; i < ver_num; i += 2){
        n1 = hash_key[inputs[i]];
        n2 = hash_key[inputs[i + 1]];
        //去重否？
        uint64_t val = vals[i / 2];
        if(val != 0){
            map[n1].emplace_back(make_pair(n2, val));
            invmap[n2].emplace_back(make_pair(n1, val));
            outDegrees[n1]++;
            inDegrees[n2]++;
        }
    }
    for(uint32_t i = 0; i < id_num; ++i){
        if(outDegrees[i] >= 1){
            outdegreesum += outDegrees[i];
            sort(map[i].begin(), map[i].end(), cmp);
            sort(invmap[i].begin(), invmap[i].end(), cmp);
        }
    }
    //delete []tmp;
#ifdef TEST
    printf("the number of id: %d\n", id_num);
    printf("the process time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::dbfs(uint32_t key, vector<bool> &visited, int id, unordered_map<uint32_t, vector<Index>> &keyvec){
    int count3 = 0;
   // keyvec.resize(id_num);
    //反向
    uint32_t ik1 = 0;
    while(ik1 < inDegrees[key] && invmap[key][ik1].first <= key) ++ik1;
    for(; ik1 < inDegrees[key]; ++ik1){
        pair<uint32_t, uint64_t> ipair1 = invmap[key][ik1];
        if(ipair1.first > key){
            visited[ipair1.first] = true;
            uint32_t ik2 = 0;
            while(ik2 < inDegrees[ipair1.first] && invmap[ipair1.first][ik2].first <= key) ++ik2;
            for(; ik2 < inDegrees[ipair1.first]; ++ik2){
                pair<uint32_t, uint64_t> ipair2 = invmap[ipair1.first][ik2];
                if(ipair2.first > key && !visited[ipair2.first] && ipair1.second <= 3*ipair2.second && ipair2.second <= 5*ipair1.second){
                    visited[ipair2.first] = true;
                    uint32_t ik3 = 0;
                    while(ik3 < inDegrees[ipair2.first] && invmap[ipair2.first][ik3].first < key) ++ik3;
                    for(; ik3 < inDegrees[ipair2.first]; ++ik3){
                        pair<uint32_t, uint64_t> ipair3 = invmap[ipair2.first][ik3];
                        if(ipair3.first == key) count3++;//是否超过3
                        if(ipair3.first > key && !visited[ipair3.first] && ipair2.second <= 3*ipair3.second && ipair3.second <= 5*ipair2.second){
                            if(keyvec.find(ipair3.first) == keyvec.end()){
                                keyvec.emplace(ipair3.first, vector<Index>{Index(ipair3,ipair2,ipair1)});
                            }
                            else{
                                keyvec[ipair3.first].push_back(Index(ipair3,ipair2,ipair1));
                            }
                            //keyvec[id][ipair3.first].emplace_back(Index(ipair3,ipair2,ipair1));
                        }
                    }
                    visited[ipair2.first] = false;
                }
            }
            visited[ipair1.first] = false;
        }
    }
    //sort
    // for(int i = 0; i < id_num; ++i){
    //     if(!keyvec[id][i].empty()){
    //         sort(keyvec[id][i].begin(),keyvec[id][i].end());
    //     }
    // }
    for(auto &it : keyvec){
        sort(it.second.begin(), it.second.end());
    }
    //pinjie
    //if(count3 == 0) return;
    uint32_t k1 = 0;
    while(k1 < outDegrees[key] && map[key][k1].first <= key) ++k1;
    for(; k1 < outDegrees[key]; ++k1){
        pair<uint32_t, uint64_t> pair1 = map[key][k1];
        if(pair1.first > key && !visited[pair1.first]){
            visited[pair1.first] = true;
            if(keyvec.find(pair1.first) != keyvec.end()){
                for(auto &p : keyvec[pair1.first]){
                    if(pair1.second <= 5*p.k0.second && p.k0.second <= 3*pair1.second && p.k2.second <= 5*pair1.second && pair1.second <= 3*p.k2.second){
                        ans4[id].emplace_back(key);
                        ans4[id].emplace_back(pair1.first);
                        ans4[id].emplace_back(p.k1.first);
                        ans4[id].emplace_back(p.k2.first);
                        res_num[id+THREAD_NUM]++;
                    }
                }
            }
            uint32_t k2 = 0;
            while(k2 < outDegrees[pair1.first] && map[pair1.first][k2].first <= key) ++k2;
            for(; k2 < outDegrees[pair1.first]; ++k2){
                pair<uint32_t, uint64_t> pair2 = map[pair1.first][k2];
                if(pair2.first > key && !visited[pair2.first] && pair1.second <= 5*pair2.second && pair2.second <= 3*pair1.second){
                    visited[pair2.first] = true;
                    if(keyvec.find(pair2.first) != keyvec.end()){
                        for(auto &p : keyvec[pair2.first]){
                            if(pair2.second <= 5*p.k0.second && p.k0.second <= 3*pair2.second && p.k2.second <= 5*pair1.second && pair1.second <= 3*p.k2.second){
                                if(!visited[p.k1.first] && !visited[p.k2.first]){
                                    ans5[id].emplace_back(key);
                                    ans5[id].emplace_back(pair1.first);
                                    ans5[id].emplace_back(pair2.first);
                                    ans5[id].emplace_back(p.k1.first);
                                    ans5[id].emplace_back(p.k2.first);
                                    res_num[id+THREAD_NUM*2]++;
                                }
                            }
                        }
                    }
                    uint32_t k3 = 0;
                    while(k3 < outDegrees[pair2.first] && map[pair2.first][k3].first < key) ++k3;
                    for(; k3 < outDegrees[pair2.first]; ++k3){
                        pair<uint32_t, uint64_t> pair3 = map[pair2.first][k3];
                        if(pair2.second <= 5*pair3.second && pair3.second <= 3*pair2.second){
                            if(pair3.first == key){
                                if(pair3.second <= 5*pair1.second && pair1.second <= 3*pair3.second){
                                    ans3[id].emplace_back(key);
                                    ans3[id].emplace_back(pair1.first);
                                    ans3[id].emplace_back(pair2.first);
                                    res_num[id]++;
                                }
                            }
                            if(pair3.first > key && !visited[pair3.first]){
                                visited[pair3.first] = true;
                                if(keyvec.find(pair3.first) != keyvec.end()){
                                    for(auto &p : keyvec[pair3.first]){
                                        if(pair3.second <= 5*p.k0.second && p.k0.second <= 3*pair3.second && p.k2.second <= 5*pair1.second && pair1.second <= 3*p.k2.second){
                                            if(!visited[p.k1.first] && !visited[p.k2.first]){
                                                ans6[id].emplace_back(key);
                                                ans6[id].emplace_back(pair1.first);
                                                ans6[id].emplace_back(pair2.first);
                                                ans6[id].emplace_back(pair3.first);
                                                ans6[id].emplace_back(p.k1.first);
                                                ans6[id].emplace_back(p.k2.first);
                                                res_num[id+THREAD_NUM*3]++;
                                            }
                                        }
                                    }
                                }
                                uint32_t k4 = 0;
                                while(k4 < outDegrees[pair3.first] && map[pair3.first][k4].first <= key) ++k4;
                                for(; k4 < outDegrees[pair3.first]; ++k4){
                                    pair<uint32_t, uint64_t> pair4 = map[pair3.first][k4];
                                    if(pair4.first > key && !visited[pair4.first] && pair3.second <= 5*pair4.second && pair4.second <= 3*pair3.second){
                                        visited[pair4.first] = true;
                                        if(keyvec.find(pair4.first) != keyvec.end()){
                                            for(auto &p : keyvec[pair4.first]){
                                                if(pair4.second <= 5*p.k0.second && p.k0.second <= 3*pair4.second && p.k2.second <= 5*pair1.second && pair1.second <= 3*p.k2.second){
                                                    if(!visited[p.k1.first] && !visited[p.k2.first]){
                                                        ans7[id].emplace_back(key);
                                                        ans7[id].emplace_back(pair1.first);
                                                        ans7[id].emplace_back(pair2.first);
                                                        ans7[id].emplace_back(pair3.first);
                                                        ans7[id].emplace_back(pair4.first);
                                                        ans7[id].emplace_back(p.k1.first);
                                                        ans7[id].emplace_back(p.k2.first);
                                                        res_num[id+THREAD_NUM*4]++;
                                                    }
                                                }
                                            }
                                        }
                                        visited[pair4.first] = false;
                                    }
                                }
                                visited[pair3.first] = false;
                            }
                        }
                    }
                    visited[pair2.first] = false;
                }
            }
            visited[pair1.first] = false;
        }
    }
    keyvec.clear();
}

void SolveGraph::solve(int id, uint32_t start, uint32_t end){
#ifdef TEST
    auto back = std::chrono::steady_clock::now();
#endif
    unordered_map<uint32_t, vector<Index>> keyvec;
    ans3[id].reserve(10000);
    ans4[id].reserve(50000);
    ans5[id].reserve(1000000);
    ans6[id].reserve(10000000);
    ans7[id].reserve(100000000);
    //keyvec[id].resize(id_num);
    // uint32_t path[7] = {0};
    // uint32_t price[7] = {0};

    //标记存在的ID和访问路径
    vector<bool> visited(id_num,false);

    for(uint32_t i = start; i < end; ++i){
        if(outDegrees[i] != 0){
            dbfs(i, visited, id, keyvec);
        }
    }
    // delete []visited;
    
#ifdef TEST
    auto endtime = std::chrono::steady_clock::now();
    for(int i = 0; i < 5; ++i){
        printf("the %d res num is: %d\n", i+3, res_num[id+THREAD_NUM*i]);
    }
    printf("the solve:%d time is: %lld\n",id, std::chrono::duration_cast<std::chrono::milliseconds>(endtime - back).count());
#endif
}

void SolveGraph::runthread(int thread_num){
    thread th[thread_num];
    uint32_t length = id_num / thread_num;
    uint32_t allthnum = 0;
    for(int i = 1; i <= thread_num; ++i){
        allthnum += i;
    }
    // test 1
    int numde = outdegreesum / allthnum;
    uint32_t j = 0,cur = 0, sumde = 0;
    for(int i = 0; i < thread_num - 1; ++i){
        uint32_t sum = 0;
        sumde = (i+1) * numde;
        while(sum < sumde) sum += outDegrees[j++];
        th[i] = thread(&SolveGraph::solve, this, i, cur, j);
        cur = j;
    }
    th[thread_num - 1] = thread(&SolveGraph::solve, this, thread_num-1, cur, id_num);
    for(int i = 0; i < thread_num; ++i){
        th[i].join();
    }
    delete []inDegrees;
    delete []outDegrees;
}

int main(int argc, char *argv[]){
#ifdef TEST
    int start = clock();
    const char* testFile = "/Users/alviney/test/data/testdata/19630345/test_data.txt";
    const char* resultFile = "/Users/alviney/test/data/testdata/19630345/result.txt";
#endif
#ifndef TEST
    const char* testFile = "/data/test_data.txt";
    const char* resultFile = "/projects/student/result.txt";
#endif
    SolveGraph solveGraph;
    //读取数据
    solveGraph.loadTestData(testFile); 
    solveGraph.preprocess();
    solveGraph.runthread(THREAD_NUM);   
    solveGraph.writeResultFile_mmap(resultFile);
#ifdef TEST   
    printf("the all time is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
    exit(0);
    return 0;
}