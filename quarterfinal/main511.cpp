/*
* vec+数组
*/
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
#define THREAD_NUM 4
uint64_t vals[2000005];
uint32_t inputs[4000005];
int res_num[THREAD_NUM*5];

bool cmp(const pair<uint32_t,uint64_t> a, const pair<uint32_t ,uint64_t> b) {
    return a.first < b.first;
}

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
    uint32_t id_max;
    vector<vector<pair<uint32_t, uint64_t>>> map;
    vector<vector<pair<uint32_t, uint64_t>>> invmap;
    vector<uint32_t> ans3[THREAD_NUM];
    vector<uint32_t> ans4[THREAD_NUM];
    vector<uint32_t> ans5[THREAD_NUM];
    vector<uint32_t> ans6[THREAD_NUM];
    vector<uint32_t> ans7[THREAD_NUM];

    void backtrack(uint32_t *list, uint32_t *price, bool *visited, long *visited1, int depth, uint32_t key, int id, uint32_t head);
    void dfsmap(uint32_t p_0, bool *visited, long *visited1);
    void dfsinvmap(uint32_t p_0, bool *visited, long *visited1);
    void solve(int id, uint32_t start, uint32_t end);

};

SolveGraph::SolveGraph():id_num(0),edge_num(0),ver_num(0),outdegreesum(0),id_max(0){
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
            id_max = id_max > tmp ? id_max : tmp;
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
    int fd = open(resultFile,O_CREAT|O_RDWR,0666);
    unsigned int size = 0;
    unsigned int result_num = 0;
    for(int j = 0; j < THREAD_NUM*5; ++j) size += res_num[j];
    if(size == 0) result_num = 1;
    else result_num = size;
    //TODO：注意文件大小的判断，可能要改
    ftruncate(fd, result_num * 24 * 7);
    char* start = (char*)mmap(nullptr,result_num * 24 * 7,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
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
    munmap(start, result_num * 24 * 7);
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
    uint32_t *tmp = new uint32_t[ver_num];
    for(uint32_t i = 0; i < ver_num; ++i){
        tmp[i] = inputs[i];
    }
    sort(tmp, tmp + ver_num);
    id_num = unique(tmp, tmp + ver_num) - tmp;
    // int tmp[id_max+1];
    // int tmp1[id_max+1];
    // memset(tmp, 0, sizeof(tmp));
    // memset(tmp, 0, sizeof(tmp1));
    // for(int i = 0; i < ver_num; ++i){
    //     if(inputs[i] <= 50000) tmp[inputs[i]] = 1;
    // }
    // for(int i = 0; i <= id_max; ++i){
    //     if(tmp[i]) tmp1[id_num++] = i;
    // }
    midwrite.resize(id_num);
    endwrite.resize(id_num);
    uint32_t key;
    for(uint32_t i = 0; i < id_num; ++i){
        key = tmp[i];
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
        map[n1].push_back(make_pair(n2, val));
        invmap[n2].push_back(make_pair(n1, val));
        outDegrees[n1]++;
        inDegrees[n2]++;
    }
    for(uint32_t i = 0; i < id_num; ++i){
        if(outDegrees[i] >= 1){
            outdegreesum += outDegrees[i];
            sort(map[i].begin(), map[i].end(), cmp);
            sort(invmap[i].begin(), invmap[i].end(), cmp);
        }
    }
    delete []tmp;
#ifdef TEST
    printf("the number of id: %d\n", id_num);
    printf("the process time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::backtrack(uint32_t *list, uint32_t *price, bool *visited, long *visited1, int depth, uint32_t key, int id, uint32_t head){
    list[depth-1] = key;
    visited[key] = true;
    for(uint32_t i = 0; i < outDegrees[key]; ++i){
        uint32_t mapkey = map[key][i].first;
        uint64_t pricee = map[key][i].second;
        // if(depth == 3 && list[0] == hash_key[6000] && list[1] == hash_key[6001] 
        //             && list[2] == hash_key[6003]) printf("yes:%d %d\n",mapkey, hash_key[6002]);
        if(mapkey < head || pricee == 0) continue;
        price[depth-1] = pricee;
        if(depth > 1){
            uint64_t tmp1 = price[depth-2];
            if(tmp1 > 5 * pricee || pricee > 3 * tmp1) continue;
        }
        if(depth >= 2 && visited1[mapkey] == -2 && !visited[mapkey]){
            list[depth] = mapkey;
            uint64_t prices = 0;
            for(auto paira : invmap[list[0]]){
                if(paira.first == mapkey){
                    prices = paira.second;
                    break;
                }
            }
            //uint64_t tmp2 = price[depth-1]; && pricee == 1699212293 && prices == 1239116434
            
            uint64_t tmp3 = price[0];
            if(pricee <= 5 * prices && prices <= 3 * pricee && prices <= 5 * tmp3 && tmp3 <= 3 * prices){
                vector<uint32_t> *p;
                switch(depth){
                    case 2:{ p = &ans3[id]; break; }
                    case 3:{ p = &ans4[id]; break; }
                    case 4:{ p = &ans5[id]; break; }
                    case 5:{ p = &ans6[id]; break; }
                    case 6:{ p = &ans7[id]; break; }
                    default:{ printf("error\n"); break; }
                }
                for(int m = 0; m <= depth; ++m){
                    p->emplace_back(list[m]);
                }
                res_num[id+THREAD_NUM*(depth-2)]++;
            }
        }
        // if(depth == 3 && list[0] == hash_key[6000] && list[1] == hash_key[6001] 
        //      && mapkey == hash_key[6003]) printf("yes:%d %d\n",mapkey, hash_key[6002]);
        if(!visited[mapkey] && (visited1[mapkey] == head || visited1[mapkey] == -2) && depth != 6 && mapkey != head)
            backtrack(list, price, visited, visited1, depth + 1, mapkey, id, head);
    }
    visited[key] = false;
}

void SolveGraph::dfsmap(uint32_t p_0, bool *visited, long *visited1){
    *(visited+p_0) = true;
    for(int i = 0; i < outDegrees[p_0]; ++i){
        int tmpkey = map[p_0][i].first;
        if(tmpkey < p_0) continue;
        if(!visited[tmpkey]){
            visited1[tmpkey] = p_0;
            *(visited+tmpkey) = true;
            for(int i1 = 0; i1 < outDegrees[tmpkey]; ++i1){
                int tmpkey1 = map[tmpkey][i1].first;
                if(tmpkey1 < p_0) continue;
                if(!visited[tmpkey1]){
                    visited1[tmpkey1] = p_0;
                    *(visited+tmpkey1) = true;
                    for(int i2 = 0; i2 < outDegrees[tmpkey1]; ++i2){
                        int tmpkey2 = map[tmpkey1][i2].first;
                        if(tmpkey2 < p_0) continue;
                        if(!visited[tmpkey2]){
                            visited1[tmpkey2] = p_0;
                        }
                    }
                    *(visited+tmpkey1) = false;
                }
            }
            *(visited+tmpkey) = false;
        }    
    }
    *(visited+p_0) = false;
}

void SolveGraph::dfsinvmap(uint32_t p_0, bool *visited, long *visited1){
    *(visited+p_0) = true;
    for(int i = 0; i < inDegrees[p_0]; ++i){
        int tmpkey = invmap[p_0][i].first;
        if(tmpkey < p_0) continue;
        if(!visited[tmpkey]){
            visited1[tmpkey] = p_0;
            *(visited+tmpkey) = true;
            for(int i1 = 0; i1 < inDegrees[tmpkey]; ++i1){
                int tmpkey1 = invmap[tmpkey][i1].first;
                if(tmpkey1 < p_0) continue;
                if(!visited[tmpkey1]){
                    visited1[tmpkey1] = p_0;
                    *(visited+tmpkey1) = true;
                    for(int i2 = 0; i2 < inDegrees[tmpkey1]; ++i2){
                        int tmpkey2 = invmap[tmpkey1][i2].first;
                        if(tmpkey2 < p_0) continue;
                        if(!visited[tmpkey2]){
                            visited1[tmpkey2] = p_0;
                        }
                    }
                    *(visited+tmpkey1) = false;
                }
            }
            *(visited+tmpkey) = false;
        }    
    }
    *(visited+p_0) = false;
}

void SolveGraph::solve(int id, uint32_t start, uint32_t end){
#ifdef TEST
    auto back = std::chrono::steady_clock::now();
#endif
    ans3[id].reserve(10000);
    ans4[id].reserve(50000);
    ans5[id].reserve(1000000);
    ans6[id].reserve(10000000);
    ans7[id].reserve(100000000);
    uint32_t path[7] = {0};
    uint32_t price[7] = {0};

    //标记存在的ID和访问路径
    bool *visited = new bool[id_num];
    fill(visited, visited + id_num, false);
    long *visited1 = new long[id_num];
    fill(visited1, visited1 + id_num, -1);

    for(uint32_t i = start; i < end; ++i){
        if(outDegrees[i] != 0){
            dfsmap(i, visited, visited1);
            dfsinvmap(i, visited, visited1);
            for(int z = 0; z < inDegrees[i]; z++){
                visited1[invmap[i][z].first] = -2;
            }
            backtrack(path, price, visited, visited1, 1, i, id, i);
            for(int z = 0; z < inDegrees[i]; z++){
                visited1[invmap[i][z].first] = i;
            }
        }
    }
    delete []visited;
    delete []visited1;
    
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