/*
version：0.2.0
name : alviney wong
方法 dfs，使用vector判断
加入mmap写入
做ID映射 ：速度变慢了
用pair读取数据并排序
能用数组就用数组，效率比vector快的多
vector比unordermap快很多
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
#include <string.h>
#include <unordered_map>
#include <set>
#include <string>
#include <queue>
#include <algorithm>
using namespace std;

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


//unordered_map<int, vector<int>> map;
vector<vector<int>> map;
vector<unordered_map<int, vector<int>>> covmap;
unordered_map<int, int> hash_key;
//vector<int> inverse_hash;
vector<int> vals;

//vector<int> inputVal;
vector<pair<int, int>> inputs;
set<vector<int>, comp> res;
//vector<vector<int>> res;
vector<string> midwrite;
vector<string> endwrite;
int *reachable;
int MAX_ID = 0;
int id_num = 0;

void loadTestData(string testFile){
	int start = clock();
    struct stat sb;
    char *buf;
    int fd = open(testFile.c_str(), O_RDONLY);
    fstat(fd, &sb);
    buf = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    int col = 0;
    int tmp = 0;
    int arg[2] = {0, 0};
    int count = 0;
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        if(*p == ','){
        	arg[col] = tmp;
            //inputVal.push_back(tmp);
        	MAX_ID = MAX_ID > tmp ? MAX_ID : tmp;//最大ID值
        	col++;
        	tmp = 0;
            continue;
        }
        if(*p == '\r'){//cout << arg[0] << arg[1] << tmp << endl;
        	//map[arg[0]].push_back(arg[1]);
            inputs.push_back(make_pair(arg[0],arg[1]));
        	vals.push_back(tmp);
        	tmp = 0;
        	col = 0;
        	p++;
            count++;
        	continue;
        }
        if(*p == '\n'){//cout << arg[0] << arg[1] << tmp << endl;
        	//map[arg[0]].push_back(arg[1]);
            inputs.push_back(make_pair(arg[0],arg[1]));
        	vals.push_back(tmp);
        	tmp = 0;
        	col = 0;
            count++;
        	continue;
        }
        tmp = tmp * 10 + *p - '0';
    }
    
    close(fd);
    munmap(buf, sb.st_size);

#ifdef TEST
    printf("the number of edge is: %d\n",count);
    printf("the load file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
}

void writeResultFile_fprintf(string resultFile){
    int writefile = clock();

    FILE *fd = fopen(resultFile.c_str(), "w");
    fprintf(fd, "%lu\n", res.size());
    for(auto it = res.begin(); it != res.end(); it++){
        int j;
        for(j = 0; j < (*it).size() - 1; j++){
            fprintf(fd, "%s", midwrite[(*it)[j]].c_str());
        }
        fprintf(fd, "%s", endwrite[(*it)[j]].c_str());
    }
    fclose(fd);
#ifdef TEST
    printf("the size of result is: %lu\n",res.size());
    printf("the fprintf file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
#endif
}

void writeResultFile_mmap(string resultFile){
    int writefile = clock();
    int fd = open(resultFile.c_str(),O_CREAT|O_RDWR,0666);
    //TODO：注意文件大小的判断，可能要改
    ftruncate(fd, res.size() * 20 * 7);
    char* start = (char*)mmap(NULL,res.size() * 20 * 7,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    
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

void backtrack(vector<int> &list, bool *visited, int depth, int key){
    list.push_back(key);
    visited[key] = true;
	//if(depth > 7) return;

	//if(map.find(key) == map.end()) return;
	//key的list提取
	vector<int> &keylist = map[key];
    auto it = lower_bound(keylist.begin(),keylist.end(),list[0]);
    //if(keylist.size() == 0) return;
    if(it != keylist.end() && *it == list[0] && depth >=3 && depth < 7){
        res.insert(list);
    }
    if(depth < 6){
        for(;it != keylist.end(); ++it){
            if(!visited[*it]) backtrack(list, visited, depth + 1, *it);
        }
    }
    else if(reachable[key] > -1 && depth == 6){
        auto ks = covmap[list[0]][key];
        for(int index = reachable[key]; index < ks.size(); ++index){
            int k = ks[index];
            if(visited[k]) continue;
            vector<int> tmp(list);
            tmp.push_back(k);
            res.insert(tmp);
        }
    }
	// for(int i = 0; i < keylist.size(); i++){
	// 	//判断新加入的ID和第一个ID相同
	// 	if(keylist[i] == list[0]){
	// 		//判断深度大于3
	// 		if(depth < 3) continue;
	// 		//满足条件重新排序
	// 		//vector<int> minheadvec(minHeadVector(list));
	// 		//添加字符串查找是否存在，存在返回，不存在添加进set，vector
	// 		res.insert(list);
	// 	}//cout << keylist[i] << endl;
	// 	//存在重复元素且不在第一位，直接返回
	// 	//if(singlenum.find(keylist[i]) != singlenum.end()) continue;
	// 	if(visited[keylist[i]] == 1) continue;
 //        if(keylist[i] < list[0]) continue;

	// 	list.push_back(keylist[i]);
	// 	visited[keylist[i]] = 1;
	// 	//singlenum.insert(keylist[i]);
	// 	backtrack(list, visited, depth + 1, keylist[i]);
	// 	list.pop_back();
	// 	visited[keylist[i]] = 0;
	// 	//singlenum.erase(keylist[i]);
	// }

    list.pop_back();
    visited[key] = false;
	
}

void degreeSort(int *degrees){
    queue<int> q;
    for(int i = 0;i < id_num; i++){
        if(degrees[i] == 0)
            q.push(i);
    }
    while(!q.empty()){
        int key = q.front();
        q.pop();
        for(int v : map[key]) {
            if(--degrees[v] == 0)
                q.push(v);
        }
    }

    int cnt = 0;
    for(int i = 0; i < id_num; i++){
        if(degrees[i] == 0){
            //map.erase(i);
            map[i].clear();
            cnt++;
        }
        // else if(isTrue){
        //     sort(map[i].begin(),map[i].end());
        // }
    }
#ifdef TEST
    printf("the number of deleNodes: %d\n", cnt);
#endif
}

void covbuild(){
    covmap = vector<unordered_map<int, vector<int>>>(id_num,unordered_map<int, vector<int>>());
    for(int i = 0; i < id_num; i++){
        auto mi = map[i];
        for(int k : mi){
            auto mk = map[k];
            for(int j : mk){
                if(i != j){
                    covmap[j][i].push_back(k);
                }
            }
        }
    }
    for(int i = 0; i < id_num; i++){
        for(auto &ida : covmap[i]){
            if(ida.second.size() > 1){
                sort(ida.second.begin(), ida.second.end());
            }
        }
    }
}

bool cmp(const pair<int, int> a, const pair<int, int> b){
    if(a.first == b.first) return a.second < b.second;
    else return a.first < b.first;
}

bool cmmp(const vector<int> a, const vector<int> b){
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
    string testFile = "/Users/alviney/test/data/testdata/3512444/test_data.txt";
    string resultFile = "/Users/alviney/test/data/testdata/3512444/result.txt";
#endif
#ifndef TEST
    string testFile = "/data/test_data.txt";
    string resultFile = "/projects/student/result.txt";
#endif
    //读取数据
    loadTestData(testFile);

    int runtime = clock();
    //映射ID
    // vector<int> idvec = inputVal;
    // sort(idvec.begin(), idvec.end());
    // idvec.erase(unique(idvec.begin(), idvec.end()), idvec.end());
    vector<int> idvectmp(MAX_ID + 1, 0);
    sort(inputs.begin(), inputs.end(), cmp);
    for(auto par : inputs){
        idvectmp[par.first] = 1;
        idvectmp[par.second] = 1;
    }
    vector<int> idvec;
    for(int i = 0; i < idvectmp.size(); i++){
        if(idvectmp[i]) idvec.push_back(i);
    }
    idvectmp.shrink_to_fit();
    id_num = idvec.size();
    int hash_index = 0;
    for(int key : idvec){
        hash_key[key] = hash_index++;
        //inverse_hash.push_back(key);
        midwrite.push_back(to_string(key) + ',');
        endwrite.push_back(to_string(key) + '\n');
    }
    idvec.shrink_to_fit();
#ifdef TEST 
    printf("the number of id: %d\n", id_num);
#endif

    map = vector<vector<int>>(id_num);
    //计算出度入度
    int inDegrees[id_num];
    int outDegrees[id_num];
    memset(inDegrees, 0, sizeof(inDegrees));
    memset(outDegrees, 0, sizeof(outDegrees));
    // inDegrees = vector<int>(id_num, 0);
    // outDegrees = vector<int>(id_num, 0);
    for(int i = 0; i < inputs.size(); i++){
        int n1 = hash_key[inputs[i].first];
        int n2 = hash_key[inputs[i].second];
        map[n1].push_back(n2);
        outDegrees[n1]++;
        inDegrees[n2]++;
    }
    inputs.shrink_to_fit();;
    degreeSort(inDegrees);
    degreeSort(outDegrees);

    covbuild();
    int back = clock();
    vector<int> path;
    int reach[id_num];
    memset(reach, -1, sizeof(reach));
    reachable = reach;
    vector<int> currentIs(id_num);
    //标记存在的ID和访问路径
    bool visited[id_num];
    memset(visited, false, sizeof(visited));
    //vector<int> visited(id_num,0);
    for(int i = 0; i < id_num; ++i){//cout << id.second.size() << endl;
        if(!map[i].empty()){
            for(auto &is : covmap[i]){
                int j = is.first;
                if(j > i){
                    auto &val = is.second;
                    int first_index = lower_bound(val.begin(), val.end(), i) - val.begin();
                    if(first_index < val.size()) reachable[j] = first_index;
                    currentIs.push_back(j);
                }
            }
            
            //cout << "1: " << id.first << endl;
            backtrack(path, visited, 1, i);
            for(int j : currentIs) reachable[j] = -1;
            currentIs.clear();
            
        }

    }
    
#ifdef TEST
    printf("the back time is: %.3f\n", float(clock() - back) / CLOCKS_PER_SEC);   
    printf("the calculate time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
    //sort(res.begin(), res.end(), cmmp);
    //writeResultFile_fprintf(resultFile);
    writeResultFile_mmap(resultFile);
#ifdef TEST   
    printf("the all time is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
    return 0;
}