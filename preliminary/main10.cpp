/*
version：0.2.0
name : alviney wong
6+1 用3领域
map invmap改静态数组
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
#define THREAD_NUM 8

const int ROW = 50010;
const int COL1 = 50;
const int COL2 = 50;
int vals[300000];
int inputs[600000];
int map[ROW][COL1];
int invmap[ROW][COL2];
int inDegrees[ROW];
int outDegrees[ROW];
int ans3[THREAD_NUM][3*500000];
int ans4[THREAD_NUM][4*500000];
int ans5[THREAD_NUM][5*1000000];
int ans6[THREAD_NUM][6*2000000];
int ans7[THREAD_NUM][7*3000000];
int ans3len[THREAD_NUM];
int ans4len[THREAD_NUM];
int ans5len[THREAD_NUM];
int ans6len[THREAD_NUM];
int ans7len[THREAD_NUM];
int res_num[THREAD_NUM*5];

class SolveGraph{
public:
    SolveGraph();
    void loadTestData(const char* testFile);
    void writeResultFile_mmap(const char* resultFile);
    void preprocess();
    void runthread(int thread_num);

private:
    unordered_map<int, int> hash_key;
    vector<string> midwrite;
    vector<string> endwrite;
    int id_num;
    int edge_num;
    int ver_num;
    int outdegreesum;
    int id_max;

    void backtrack(int *list, bool *visited, int *visited1, int depth, int key, int id, int head);
    void dfsmap(int p_0, bool *visited, int *visited1);
    void dfsinvmap(int p_0, bool *visited, int *visited1);
    void solve(int id, int start, int end);

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
    
    int tmp = 0;
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        if(*p >= '0' && *p <= '9'){
            tmp = tmp * 10 + *p - '0';
        }
        else if(*p == ','){
            inputs[ver_num] = tmp;
            id_max = id_max > tmp ? id_max : tmp;
            ver_num++;
            tmp = 0;
        }
        else if(*p == '\r'){
            vals[edge_num] = tmp;
            tmp = 0;
            p++;
            edge_num++;
        }
        else if(*p == '\n'){
            vals[edge_num] = tmp;
            tmp = 0;
            edge_num++;
        }
        //else tmp = tmp * 10 + *p - '0';
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
    ftruncate(fd, result_num * 20 * 7);
    char* start = (char*)mmap(nullptr,result_num * 20 * 7,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
    long long offerlen = 0;
    string s = to_string(size) + '\n';
    strcpy(start,s.c_str());
    offerlen = s.size();
    int len3 = 0;
    for(int i = 0; i < THREAD_NUM; ++i){       
        len3 = ans3len[i];
        int *p = ans3[i];
        for(int j = 0; j < len3; j+=3){
            s = midwrite[*(p+j)];
            s += midwrite[*(p+j+1)];
            s += endwrite[*(p+j+2)];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    int len4 = 0;
    for(int i = 0; i < THREAD_NUM; ++i){       
        len4 = ans4len[i];
        int *p = ans4[i];
        for(int j = 0; j < len4; j+=4){
            s = midwrite[*(p+j)];
            s += midwrite[*(p+j+1)];
            s += midwrite[*(p+j+2)];
            s += endwrite[*(p+j+3)];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    int len5 = 0;
    for(int i = 0; i < THREAD_NUM; ++i){       
        len5 = ans5len[i];
        int *p = ans5[i];
        for(int j = 0; j < len5; j+=5){
            s = midwrite[*(p+j)];
            s += midwrite[*(p+j+1)];
            s += midwrite[*(p+j+2)];
            s += midwrite[*(p+j+3)];
            s += endwrite[*(p+j+4)];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    int len6 = 0;
    for(int i = 0; i < THREAD_NUM; ++i){       
        len6 = ans6len[i];
        int *p = ans6[i];
        for(int j = 0; j < len6; j+=6){
            s = midwrite[*(p+j)];
            s += midwrite[*(p+j+1)];
            s += midwrite[*(p+j+2)];
            s += midwrite[*(p+j+3)];
            s += midwrite[*(p+j+4)];
            s += endwrite[*(p+j+5)];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    int len7 = 0;
    for(int i = 0; i < THREAD_NUM; ++i){       
        len7 = ans7len[i];
        int *p = ans7[i];
        for(int j = 0; j < len7; j+=7){
            s = midwrite[*(p+j)];
            s += midwrite[*(p+j+1)];
            s += midwrite[*(p+j+2)];
            s += midwrite[*(p+j+3)];
            s += midwrite[*(p+j+4)];
            s += midwrite[*(p+j+5)];
            s += endwrite[*(p+j+6)];
            strcpy(start + offerlen,s.c_str());
            offerlen += s.size();
        }
    }
    ftruncate(fd, offerlen);
    close(fd);
    munmap(start, result_num * 20 * 7);
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
    // int tmp[ver_num];
    // for(int i = 0; i < ver_num; ++i){
    //     tmp[i] = inputs[i];
    // }
    // sort(tmp, tmp + ver_num);
    // id_num = unique(tmp, tmp + ver_num) - tmp;
    int tmp[id_max+1];
    int tmp1[id_max+1];
    memset(tmp, 0, sizeof(tmp));
    memset(tmp, 0, sizeof(tmp1));
    for(int i = 0; i < ver_num; ++i){
        if(inputs[i] <= 50000) tmp[inputs[i]] = 1;
    }
    for(int i = 0; i <= id_max; ++i){
        if(tmp[i]) tmp1[id_num++] = i;
    }
    midwrite.resize(id_num);
    endwrite.resize(id_num);
    int key;
    for(int i = 0; i < id_num; ++i){
        key = tmp1[i];
        hash_key[key] = i;
        midwrite[i] = to_string(key) + ',';
        endwrite[i] = to_string(key) + '\n';
    }
    //计算出度入度
    int n1,n2,cnt;
    for(int i = 0; i < ver_num; i += 2){
        if(inputs[i] > 50000 || inputs[i + 1] > 50000) continue;
        n1 = hash_key[inputs[i]];
        n2 = hash_key[inputs[i + 1]];
        cnt = 0;
        for(int j = 0; j < outDegrees[n1]; ++j){
            if(map[n1][j] == n2){
                ++cnt;
                break;
            }
        }
        if(cnt == 0){
            map[n1][outDegrees[n1]++] = n2;
            invmap[n2][inDegrees[n2]++] = n1;
        }
    }
    for(int i = 0; i < id_num; ++i){
        if(outDegrees[i] >= 1){
            outdegreesum += outDegrees[i];
            sort(map[i],map[i]+outDegrees[i]);
            sort(invmap[i],invmap[i]+inDegrees[i]);
        }
    }
#ifdef TEST
    printf("the number of id: %d\n", id_num);
    printf("the process time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::backtrack(int *list, bool *visited, int *visited1, int depth, int key, int id, int head){
    list[depth-1] = key;
    *(visited+key) = true;
    for(int i = 0; i < outDegrees[key]; ++i){
        int mapkey = map[key][i];
        if(mapkey < head) continue;
        if(visited1[mapkey] == -2 && !visited[mapkey]){
            if(depth >= 2){
                list[depth] = mapkey;                              
                switch(depth){
                    case 2:{
                        int lent = ans3len[id];
                        int *cp = ans3[id];
                        *(cp + lent) = list[0];
                        *(cp + lent + 1) = list[1];
                        *(cp + lent + 2) = list[2];
                        ans3len[id] += 3;
                        break;
                    }
                    case 3:{
                        int lent = ans4len[id];
                        int *cp = ans4[id];
                        *(cp + lent) = list[0];
                        *(cp + lent + 1) = list[1];
                        *(cp + lent + 2) = list[2];
                        *(cp + lent + 3) = list[3];
                        ans4len[id] += 4;
                        break;
                    }
                    case 4:{
                        int lent = ans5len[id];
                        int *cp = ans5[id];
                        *(cp + lent) = list[0];
                        *(cp + lent + 1) = list[1];
                        *(cp + lent + 2) = list[2];
                        *(cp + lent + 3) = list[3];
                        *(cp + lent + 4) = list[4];
                        ans5len[id] += 5;
                        break;                        
                    }
                    case 5:{
                        int lent = ans6len[id];
                        int *cp = ans6[id];
                        *(cp + lent) = list[0];
                        *(cp + lent + 1) = list[1];
                        *(cp + lent + 2) = list[2];
                        *(cp + lent + 3) = list[3];
                        *(cp + lent + 4) = list[4];
                        *(cp + lent + 5) = list[5];
                        ans6len[id] += 6;
                        break;                         
                    }
                    case 6:{
                        int lent = ans7len[id];
                        int *cp = ans7[id];
                        *(cp + lent) = list[0];
                        *(cp + lent + 1) = list[1];
                        *(cp + lent + 2) = list[2];
                        *(cp + lent + 3) = list[3];
                        *(cp + lent + 4) = list[4];
                        *(cp + lent + 5) = list[5];
                        *(cp + lent + 6) = list[6];
                        ans7len[id] += 7;
                        break;                        
                    }
                    default:{
                        printf("error\n");
                        break;                        
                    }
                }
                res_num[id+THREAD_NUM*(depth-2)]++;
            }
        }
        if(!visited[mapkey] && (visited1[mapkey] == head || visited1[mapkey] == -2) && depth != 6 && mapkey != head)
            backtrack(list, visited, visited1, depth + 1, mapkey, id, head);
    }
    *(visited+key) = false;
}

void SolveGraph::dfsmap(int p_0, bool *visited, int *visited1){
    *(visited+p_0) = true;
    for(int i = 0; i < outDegrees[p_0]; ++i){
        int tmpkey = map[p_0][i];
        if(tmpkey < p_0) continue;
        if(!visited[tmpkey]){
            visited1[tmpkey] = p_0;
            *(visited+tmpkey) = true;
            for(int i1 = 0; i1 < outDegrees[tmpkey]; ++i1){
                int tmpkey1 = map[tmpkey][i1];
                if(tmpkey1 < p_0) continue;
                if(!visited[tmpkey1]){
                    visited1[tmpkey1] = p_0;
                    *(visited+tmpkey1) = true;
                    for(int i2 = 0; i2 < outDegrees[tmpkey1]; ++i2){
                        int tmpkey2 = map[tmpkey1][i2];
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

void SolveGraph::dfsinvmap(int p_0, bool *visited, int *visited1){
    *(visited+p_0) = true;
    for(int i = 0; i < inDegrees[p_0]; ++i){
        int tmpkey = invmap[p_0][i];
        if(tmpkey < p_0) continue;
        if(!visited[tmpkey]){
            visited1[tmpkey] = p_0;
            *(visited+tmpkey) = true;
            for(int i1 = 0; i1 < inDegrees[tmpkey]; ++i1){
                int tmpkey1 = invmap[tmpkey][i1];
                if(tmpkey1 < p_0) continue;
                if(!visited[tmpkey1]){
                    visited1[tmpkey1] = p_0;
                    *(visited+tmpkey1) = true;
                    for(int i2 = 0; i2 < inDegrees[tmpkey1]; ++i2){
                        int tmpkey2 = invmap[tmpkey1][i2];
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

void SolveGraph::solve(int id, int start, int end){
#ifdef TEST
    auto back = std::chrono::steady_clock::now();
#endif
    int path[7] = {0};

    //标记存在的ID和访问路径
    bool visited[id_num];
    memset(visited, false, sizeof(visited));
    int visited1[id_num];
    memset(visited1, -1, sizeof(visited1));

    for(int i = start; i < end; ++i){
        if(outDegrees[i] != 0){
            dfsmap(i, visited, visited1);
            dfsinvmap(i, visited, visited1);
            for(int z = 0; z < inDegrees[i]; z++){
                visited1[invmap[i][z]] = -2;
            }
            backtrack(path, visited, visited1, 1, i, id, i);
            for(int z = 0; z < inDegrees[i]; z++){
                visited1[invmap[i][z]] = i;
            }
            //outDegrees[i] = 0;
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
    int length = id_num / thread_num;
    int allthnum = 0;
    for(int i = 1; i <= thread_num; ++i){
        allthnum += i;
    }
    // test 1
    int numde = outdegreesum / allthnum;
    int j = 0,cur = 0, sumde = 0;
    for(int i = 0; i < thread_num - 1; ++i){
        int sum = 0;
        sumde = (i+1) * numde;
        while(sum < sumde) sum += outDegrees[j++];
        th[i] = thread(&SolveGraph::solve, this, i, cur, j);
        cur = j;
        // cpu_set_t cpuset;
        // CPU_ZERO(&cpuset);
        // CPU_SET(i, &cpuset);
        // int rc = pthread_setaffinity_np(th[i].native_handle(),sizeof(cpu_set_t), &cpuset);
    }
    th[thread_num - 1] = thread(&SolveGraph::solve, this, thread_num-1, cur, id_num);
    // test 2
    // int verm = id_num / allthnum;
    // int cur = 0;
    // for(int i = 0; i < thread_num - 1; i++){
    //     th[i] = thread(&SolveGraph::solve, this, i, cur, cur + (i+1)*verm);
    //     cur = cur + (i+1)*verm;
    // }
    // th[thread_num - 1] = thread(&SolveGraph::solve, this, thread_num-1, cur, id_num);
    for(int i = 0; i < thread_num; ++i){
        th[i].join();
    }
}

int main(int argc, char *argv[]){
#ifdef TEST
    int start = clock();
    const char* testFile = "/Users/alviney/test/data/testdata/1004812/test_data.txt";
    const char* resultFile = "/Users/alviney/test/data/testdata/1004812/result.txt";
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
使用3邻域
initialize
the number of vert is: 560000
the number of edge is: 280000
the load file cost is: 0.019
the number of id: 25000
the number of deleNodes: 0
the process time is: 0.103
the 3 res num is: 484
the 4 res num is: 3884
the 5 res num is: 35272
the 6 res num is: 328023
the 7 res num is: 3144781
the solve time is: 4.233
the size of result is: 3512444
the mmap file cost is: 0.465
the all time is: 4.823

1.344
1.279
*/