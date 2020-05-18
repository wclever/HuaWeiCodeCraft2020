/*使用lower_bound,修改数组*/

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
#include <atomic> 
using namespace std;
#define THREAD_NUM 4
uint64_t vals[2000005];
uint32_t inputs[4000005];
uint32_t TMP[4000005];
uint32_t res_num[THREAD_NUM * 5];
uint32_t anslen[THREAD_NUM * 5];
uint32_t *ansSort[5][2000000];
uint32_t ansSortCount[5][2000000];
atomic_flag flag[2000000] = {ATOMIC_FLAG_INIT};

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

    uint32_t* ans13 = new uint32_t[3 * 20000000];
    uint32_t* ans14 = new uint32_t[4 * 20000000];
    uint32_t* ans15 = new uint32_t[5 * 20000000];
    uint32_t* ans16 = new uint32_t[6 * 20000000];
    uint32_t* ans17 = new uint32_t[7 * 20000000];

    uint32_t* ans23 = new uint32_t[3 * 20000000];
    uint32_t* ans24 = new uint32_t[4 * 20000000];
    uint32_t* ans25 = new uint32_t[5 * 20000000];
    uint32_t* ans26 = new uint32_t[6 * 20000000];
    uint32_t* ans27 = new uint32_t[7 * 20000000];

    uint32_t* ans33 = new uint32_t[3 * 20000000];
    uint32_t* ans34 = new uint32_t[4 * 20000000];
    uint32_t* ans35 = new uint32_t[5 * 20000000];
    uint32_t* ans36 = new uint32_t[6 * 20000000];
    uint32_t* ans37 = new uint32_t[7 * 20000000];

    uint32_t* ans43 = new uint32_t[3 * 20000000];
    uint32_t* ans44 = new uint32_t[4 * 20000000];
    uint32_t* ans45 = new uint32_t[5 * 20000000];
    uint32_t* ans46 = new uint32_t[6 * 20000000];
    uint32_t* ans47 = new uint32_t[7 * 20000000];

    uint32_t *ans[THREAD_NUM * 5] = {
                        ans13, ans23, ans33, ans43, 
                        ans14, ans24, ans34, ans44,
                        ans15, ans25, ans35, ans45,
                        ans16, ans26, ans36, ans46,
                        ans17, ans27, ans37, ans47};

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

    uint32_t* ansAll = (uint32_t*)malloc(20000000 * 2 * 7 * sizeof(uint32_t));
    uint32_t len = 0, sumNum = 0, depNum = 0, tmpSumNUm = 0;
    uint32_t lenNum[5] = {0};//3-7起始点位置
    for(uint32_t i = 0; i < 5; ++i){
        len = i + 3;
        lenNum[i] = sumNum;
        for(uint32_t j = 0; j < id_num; ++j){
            depNum = ansSortCount[i][j];//i个数j个key的len
            if(depNum == 0) continue;
            auto& addrNum = ansSort[i][j];
            for(uint32_t t1 = 0; t1 < depNum; ++t1){
                ansAll[tmpSumNUm++] = *(addrNum + t1);
            }
            sumNum += depNum;
        }
    }
    
    struct stat buffer;
    if(stat (resultFile, &buffer) != 0){
        remove(resultFile);
    }
    int fd = open(resultFile,O_CREAT|O_RDWR,0666);
    uint32_t size = 0;
    uint32_t result_num = 0;
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
    for(uint32_t j = 0; j < lenNum[1]; j+=3){
        s = midwrite[ansAll[j]];
        s += midwrite[ansAll[j+1]];
        s += endwrite[ansAll[j+2]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }
    for(uint32_t j = lenNum[1]; j < lenNum[2]; j+=4){
        s = midwrite[ansAll[j]];
        s += midwrite[ansAll[j+1]];
        s += midwrite[ansAll[j+2]];
        s += endwrite[ansAll[j+3]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }
    for(uint32_t j = lenNum[2]; j < lenNum[3]; j+=5){
        s = midwrite[ansAll[j]];
        s += midwrite[ansAll[j+1]];
        s += midwrite[ansAll[j+2]];
        s += midwrite[ansAll[j+3]];
        s += endwrite[ansAll[j+4]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }
    for(uint32_t j = lenNum[3]; j < lenNum[4]; j+=6){
        s = midwrite[ansAll[j]];
        s += midwrite[ansAll[j+1]];
        s += midwrite[ansAll[j+2]];
        s += midwrite[ansAll[j+3]];
        s += midwrite[ansAll[j+4]];
        s += endwrite[ansAll[j+5]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }
    for(uint32_t j = lenNum[4]; j < sumNum; j+=7){
        s = midwrite[ansAll[j]];
        s += midwrite[ansAll[j+1]];
        s += midwrite[ansAll[j+2]];
        s += midwrite[ansAll[j+3]];
        s += midwrite[ansAll[j+4]];
        s += midwrite[ansAll[j+5]];
        s += endwrite[ansAll[j+6]];
        strcpy(start + offerlen,s.c_str());
        offerlen += s.size();
    }

    // for(int i = 0; i < THREAD_NUM; ++i){
    //     uint32_t *p = ans[i];
    //     for(int j = 0; j < anslen[i]; j+=3){
    //         s = midwrite[*(p+j)];
    //         s += midwrite[*(p+j+1)];
    //         s += endwrite[*(p+j+2)];
    //         strcpy(start + offerlen,s.c_str());
    //         offerlen += s.size();
    //     }
    // }
    // for(int i = THREAD_NUM; i < THREAD_NUM*2; ++i){       
    //     uint32_t *p = ans[i];
    //     for(int j = 0; j < anslen[i]; j+=4){
    //         s = midwrite[*(p+j)];
    //         s += midwrite[*(p+j+1)];
    //         s += midwrite[*(p+j+2)];
    //         s += endwrite[*(p+j+3)];
    //         strcpy(start + offerlen,s.c_str());
    //         offerlen += s.size();
    //     }
    // }
    // for(int i = THREAD_NUM*2; i < THREAD_NUM*3; ++i){       
    //     uint32_t *p = ans[i];
    //     for(int j = 0; j < anslen[i]; j+=5){
    //         s = midwrite[*(p+j)];
    //         s += midwrite[*(p+j+1)];
    //         s += midwrite[*(p+j+2)];
    //         s += midwrite[*(p+j+3)];
    //         s += endwrite[*(p+j+4)];
    //         strcpy(start + offerlen,s.c_str());
    //         offerlen += s.size();
    //     }
    // }
    // for(int i = THREAD_NUM*3; i < THREAD_NUM*4; ++i){       
    //     uint32_t *p = ans[i];
    //     for(int j = 0; j < anslen[i]; j+=6){
    //         s = midwrite[*(p+j)];
    //         s += midwrite[*(p+j+1)];
    //         s += midwrite[*(p+j+2)];
    //         s += midwrite[*(p+j+3)];
    //         s += midwrite[*(p+j+4)];
    //         s += endwrite[*(p+j+5)];
    //         strcpy(start + offerlen,s.c_str());
    //         offerlen += s.size();
    //     }
    // }
    // for(int i = THREAD_NUM*4; i < THREAD_NUM*5; ++i){       
    //     uint32_t *p = ans[i];
    //     for(int j = 0; j < anslen[i]; j+=7){
    //         s = midwrite[*(p+j)];
    //         s += midwrite[*(p+j+1)];
    //         s += midwrite[*(p+j+2)];
    //         s += midwrite[*(p+j+3)];
    //         s += midwrite[*(p+j+4)];
    //         s += midwrite[*(p+j+5)];
    //         s += endwrite[*(p+j+6)];
    //         strcpy(start + offerlen,s.c_str());
    //         offerlen += s.size();
    //     }
    // }
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
        uint64_t val = vals[i >> 1];
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
                        uint32_t* cp = ans[id+THREAD_NUM];
                        int lent = anslen[id+THREAD_NUM];
                        *(cp + lent) = key;
                        *(cp + lent + 1) = pair1.first;
                        *(cp + lent + 2) = p.k1.first;
                        *(cp + lent + 3) = p.k2.first;
                        anslen[id+THREAD_NUM] += 4;                        
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
                                    uint32_t* cp = ans[id+THREAD_NUM*2];
                                    int lent = anslen[id+THREAD_NUM*2];
                                    *(cp + lent) = key;
                                    *(cp + lent + 1) = pair1.first;
                                    *(cp + lent + 2) = pair2.first;
                                    *(cp + lent + 3) = p.k1.first;
                                    *(cp + lent + 4) = p.k2.first;
                                    anslen[id+THREAD_NUM*2] += 5;
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
                                    uint32_t* cp = ans[id];
                                    int lent = anslen[id];
                                    *(cp + lent) = key;
                                    *(cp + lent + 1) = pair1.first;
                                    *(cp + lent + 2) = pair2.first;
                                    anslen[id] += 3;
                                    res_num[id]++;
                                }
                            }
                            if(pair3.first > key && !visited[pair3.first]){
                                visited[pair3.first] = true;
                                if(keyvec.find(pair3.first) != keyvec.end()){
                                    for(auto &p : keyvec[pair3.first]){
                                        if(pair3.second <= 5*p.k0.second && p.k0.second <= 3*pair3.second && p.k2.second <= 5*pair1.second && pair1.second <= 3*p.k2.second){
                                            if(!visited[p.k1.first] && !visited[p.k2.first]){
                                                uint32_t* cp = ans[id+THREAD_NUM*3];
                                                int lent = anslen[id+THREAD_NUM*3];
                                                *(cp + lent) = key;
                                                *(cp + lent + 1) = pair1.first;
                                                *(cp + lent + 2) = pair2.first;
                                                *(cp + lent + 3) = pair3.first;
                                                *(cp + lent + 4) = p.k1.first;
                                                *(cp + lent + 5) = p.k2.first;
                                                anslen[id+THREAD_NUM*3] += 6;                                                
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
                                                        uint32_t* cp = ans[id+THREAD_NUM*4];
                                                        int lent = anslen[id+THREAD_NUM*4];
                                                        *(cp + lent) = key;
                                                        *(cp + lent + 1) = pair1.first;
                                                        *(cp + lent + 2) = pair2.first;
                                                        *(cp + lent + 3) = pair3.first;
                                                        *(cp + lent + 4) = pair4.first;
                                                        *(cp + lent + 5) = p.k1.first;
                                                        *(cp + lent + 6) = p.k2.first;
                                                        anslen[id+THREAD_NUM*4] += 7;                                                        
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
    //标记存在的ID和访问路径
    vector<bool> visited(id_num,false);
    uint32_t anslen3,anslen4,anslen5,anslen6,anslen7;
    for(uint32_t i = start; i < end; ++i){
        if(flag[i].test_and_set()) continue;
        if(outDegrees[i] != 0){
            anslen3 = anslen[id];
            anslen4 = anslen[id+THREAD_NUM];
            anslen5 = anslen[id+THREAD_NUM*2];
            anslen6 = anslen[id+THREAD_NUM*3];
            anslen7 = anslen[id+THREAD_NUM*4];
            ansSort[0][i] = ans[id] + anslen3;
            ansSort[1][i] = ans[id+THREAD_NUM] + anslen4;
            ansSort[2][i] = ans[id+THREAD_NUM*2] + anslen5;
            ansSort[3][i] = ans[id+THREAD_NUM*3] + anslen6;
            ansSort[4][i] = ans[id+THREAD_NUM*4] + anslen7;
            dbfs(i, visited, id, keyvec);
            ansSortCount[0][i] = anslen[id] - anslen3;
            ansSortCount[1][i] = anslen[id+THREAD_NUM] - anslen4;
            ansSortCount[2][i] = anslen[id+THREAD_NUM*2] - anslen5;
            ansSortCount[3][i] = anslen[id+THREAD_NUM*3] - anslen6;
            ansSortCount[4][i] = anslen[id+THREAD_NUM*4] - anslen7;
        }
    }
    
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
    for(int i = 0; i < thread_num; ++i){
        th[i] = thread(&SolveGraph::solve, this, i, 0, id_num);
    }
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

/*
the number of id: 111314
the process time is: 1.546
the 3 res num is: 60
the 4 res num is: 256
the 5 res num is: 1202
the 6 res num is: 5557
the 7 res num is: 27634
the solve:3 time is: 1617
the 3 res num is: 281
the 4 res num is: 2214
the 5 res num is: 18401
the 6 res num is: 156899
the 7 res num is: 1362271
the solve:2 time is: 14205
the 3 res num is: 869
the 4 res num is: 7187
the 5 res num is: 70035
the 6 res num is: 771595
the 7 res num is: 9455205
the solve:0 time is: 30894
the 3 res num is: 709
the 4 res num is: 6375
the 5 res num is: 62125
the 6 res num is: 643576
the 7 res num is: 7037894
the solve:1 time is: 31790
the size of result is: 19630345
the mmap file cost is: 1.922
the all time is: 82.022
*/