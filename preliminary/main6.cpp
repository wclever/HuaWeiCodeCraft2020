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
using namespace std;

const int ROW = 280000;
const int COL1 = 50;
const int COL2 = 255;
int map[ROW][COL1];
int invmap[ROW][COL2];
int maplen[ROW];
int invmaplen[ROW];
int vals[300000];
int inputs[600000];
int res_num[8];

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
    void loadTestData(const string& testFile);
    void writeResultFile_mmap(const string& resultFile);
    void preprocess();
    void solve();

private:
    unordered_map<int, int> hash_key;
    set<vector<int>, comp> res;
    vector<string> midwrite;
    vector<string> endwrite;
    int id_num;
    int edge_num;
    int ver_num;

    void backtrack(vector<int> &list, bool *visited, int *visited1, int depth, int key);
    void degreeSort(int *degrees, int *outdegrees);
    void dfsmap(int (&tmap)[ROW][COL1], int *tmaplen, int p_0, int key, bool *visited, int *visited1, int depth);
    void dfsinvmap(int (&tmap)[ROW][COL2], int *tmaplen, int p_0, int key, bool *visited, int *visited1, int depth);

};

SolveGraph::SolveGraph(){
    int init = clock();
    id_num = 0;
    edge_num = 0;
    ver_num = 0;
#ifdef TEST
    printf("initialize\n");
    printf("the init cost is: %.3f\n", float(clock() - init) / CLOCKS_PER_SEC);
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
        for(int v = 0; v < maplen[key]; v++) {
            if(--indegrees[map[key][v]] == 0)
                q.push(map[key][v]);
        }
    }
    while(!p.empty()){
        int key = p.front();
        p.pop();
        for(int v = 0; v < invmaplen[key]; v++) {
            if(--outdegrees[invmap[key][v]] == 0)
                p.push(invmap[key][v]);
        }
    }

    int cnt = 0;
    for(int i = 0; i < id_num; i++){
        if(indegrees[i] == 0 || outdegrees[i] == 0){
            maplen[i] = 0;
            invmaplen[i] = 0;
            cnt++;
        }
        else{
            if(outdegrees[i] >= 1){
                sort(map[i],map[i]+maplen[i]);
                sort(invmap[i],invmap[i]+invmaplen[i]);
            }
        }
    }
#ifdef TEST
    printf("the number of deleNodes: %d\n", cnt);
#endif
}

void SolveGraph::preprocess(){
    int runtime = clock();
    //映射ID
    int tmp[ver_num];
    for(int i = 0; i < ver_num; i++){
        tmp[i] = inputs[i];
    }
    sort(tmp, tmp + ver_num);
    id_num = unique(tmp, tmp + ver_num) - tmp;
    midwrite.resize(id_num);
    endwrite.resize(id_num);
    for(int i = 0; i < id_num; i++){
        int key = tmp[i];
        hash_key[key] = i;
        midwrite[i] = to_string(key) + ',';
        endwrite[i] = to_string(key) + '\n';
    }
#ifdef TEST 
    printf("the number of id: %d\n", id_num);
#endif

    // map.resize(id_num);
    // invmap.resize(id_num);
    //计算出度入度
    int *inDegrees = new int[id_num]();
    int *outDegrees = new int[id_num]();
    for(int i = 0; i < ver_num; i += 2){
        int n1 = hash_key[inputs[i]];
        int n2 = hash_key[inputs[i + 1]];
        int cnt = 0;
        for(int i = 0; i < maplen[n1]; i++){
            if(map[n1][i] == n2) cnt++;
        }
        if(cnt == 0){
            map[n1][maplen[n1]++] = n2;
            invmap[n2][invmaplen[n2]++] = n1;
            outDegrees[n1]++;
            inDegrees[n2]++;        
        }
    }
    degreeSort(inDegrees, outDegrees);

    //covbuild();
#ifdef TEST  
    printf("the process time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
}

void SolveGraph::backtrack(vector<int> &list, bool *visited, int *visited1, int depth, int key){
    list.push_back(key);
    visited[key] = true;

    //key的list提取
    for(int i = 0; i < maplen[key]; i++){
        if(map[key][i] < list[0]) continue;
        if(visited1[map[key][i]] == -2 && !visited[map[key][i]]){
            if(depth >= 2){
                list.push_back(map[key][i]);
                res.insert(list);
                res_num[depth+1]++;
                list.pop_back();
            }
        }
        if(!visited[map[key][i]] && (visited1[map[key][i]] == list[0] || visited1[map[key][i]] == -2) && depth != 6 && map[key][i] != list[0])
            backtrack(list, visited, visited1, depth + 1, map[key][i]);
    }

    list.pop_back();
    visited[key] = false;
}

void SolveGraph::dfsmap(int (&tmap)[ROW][COL1], int *tmaplen, int p_0, int key, bool *visited, int *visited1, int depth){
    visited[key] = true;
    for(int i = 0; i < tmaplen[key]; i++){
        if(tmap[key][i] < p_0) continue;
        if(!visited[tmap[key][i]]){
            visited1[tmap[key][i]] = p_0;
            if(depth != 3) dfsmap(tmap, tmaplen, p_0, tmap[key][i], visited, visited1, depth+1);
        }    
    }
    visited[key] = false;
}

void SolveGraph::dfsinvmap(int (&tmap)[ROW][COL2], int *tmaplen, int p_0, int key, bool *visited, int *visited1, int depth){
    visited[key] = true;
    for(int i = 0; i < tmaplen[key]; i++){
        if(tmap[key][i] < p_0) continue;
        if(!visited[tmap[key][i]]){
            visited1[tmap[key][i]] = p_0;
            if(depth != 3) dfsinvmap(tmap, tmaplen, p_0, tmap[key][i], visited, visited1, depth+1);
        }    
    }
    visited[key] = false;
}

void SolveGraph::solve(){
    int back = clock();
    vector<int> path;

    //标记存在的ID和访问路径
    bool visited[id_num];
    memset(visited, false, sizeof(visited));
    int visited1[id_num];
    memset(visited1, -1, sizeof(visited1));

    for(int i = 0; i < id_num; ++i){//cout << id.second.size() << endl;
        if(maplen[i] != 0){
            dfsmap(map, maplen, i, i, visited, visited1, 1);
            dfsinvmap(invmap, invmaplen, i, i, visited, visited1, 1);
            for(int z = 0; z < invmaplen[i]; z++){
                visited1[invmap[i][z]] = -2;
            }
            backtrack(path, visited, visited1, 1, i);
            for(int z = 0; z < invmaplen[i]; z++){
                visited1[invmap[i][z]] = i;
            }
        }
    }
#ifdef TEST
    for(int i = 3; i < 8; i++){
        printf("the %d res num is: %d\n", i, res_num[i]);
    }
    printf("the solve time is: %.3f\n", float(clock() - back) / CLOCKS_PER_SEC);
#endif
}

int main(int argc, char *argv[]){
    int start = clock();
#ifdef TEST
    string testFile = "/Users/alviney/test/data/testdata/2755223/test_data.txt";
    string resultFile = "/Users/alviney/test/data/testdata/2755223/result.txt";
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
*/