/*
version：0.2.0
name : alviney wong
方法 dfs，使用vector判断
visited标记填入值
set作为结果的集合，减少判断排序
每个顺序的下一个节点不能超过首结点，避免排序
加入入度判断，输出用fpringf
使用拓扑排序删除入度出度0的key
加入映射成连续的值
min time: 1.177
*/

#define TEST

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
//#include <cmath>
//#include <cstdlib>
#include <thread>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
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

typedef vector<int> List;
unordered_map<int, List> map;
unordered_map<int, unordered_map<int, vector<int>>> covmap;
vector<int> vals;
vector<int> inDegrees;
vector<int> outDegrees;
vector<int> degreeList;
set<vector<int>, comp> res;
set<string> str;
int DEPTH = 7;
int MAX_ID = 0;

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
            degreeList.push_back(tmp);
        	MAX_ID = MAX_ID > tmp ? MAX_ID : tmp;//最大ID值
        	col++;
        	tmp = 0;
            continue;
        }
        if(*p == '\r'){//cout << arg[0] << arg[1] << tmp << endl;
        	map[arg[0]].push_back(arg[1]);
        	vals.push_back(tmp);
            //degreeList.push_back(arg[1]);
        	tmp = 0;
        	col = 0;
        	p++;
            count++;
        	continue;
        }
        if(*p == '\n'){//cout << arg[0] << arg[1] << tmp << endl;
        	map[arg[0]].push_back(arg[1]);
        	vals.push_back(tmp);
            //degreeList.push_back(arg[1]);
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

// void writeResultFile_ofstream(string resultFile){
//     int writefile = clock();

//     ofstream fout(resultFile.c_str());
//     if (!fout.is_open()) {
//         cout << "打开预测结果文件失败" << endl;
//     }

//     fout << res.size() << endl;
//     for(auto it = res.begin(); it != res.end(); it++){
//         int j;
//         for(j = 0; j < (*it).size() - 1; j++){
//             fout << (*it)[j] << ",";
//         }
//         fout << (*it)[j] << endl;
//     }
//     fout.close();

// #ifdef TEST
//     printf("the size of result is: %lu\n",res.size());
//     printf("the write file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
// #endif
// }

void writeResultFile_fprintf(string resultFile){
    int writefile = clock();

    FILE *fd = fopen(resultFile.c_str(), "w");
    fprintf(fd, "%lu\n", res.size());
    for(auto it = res.begin(); it != res.end(); it++){
        int j;
        for(j = 0; j < (*it).size() - 1; j++){
            fprintf(fd, "%d,", (*it)[j]);
        }
        fprintf(fd, "%d\n", (*it)[j]);
    }
    fclose(fd);
#ifdef TEST
    printf("the size of result is: %lu\n",res.size());
    printf("the write file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
#endif
}

// void writeResultFile_fwrite(string resultFile){
//     int writefile = clock();

//     FILE *fd = fopen(resultFile.c_str(), "wb");
//     unsigned int res_size = res.size();
//     fwrite(&res_size,sizeof(unsigned int),1,fd);
//     fwrite("\n",1,1,fd);
//     for(auto it = res.begin(); it != res.end(); it++){
//         int j;
//         for(j = 0; j < (*it).size() - 1; j++){
//             fwrite(&(*it)[j],sizeof(int),1,fd);
//             fwrite(",",1,1,fd);
//         }
//         fwrite(&(*it)[j],sizeof(int),1,fd);
//         fwrite("\n",1,1,fd);
//     }
//     fclose(fd);
// #ifdef TEST
//     printf("the size of result is: %lu\n",res.size());
//     printf("the write file cost is: %.3f\n", float(clock() - writefile) / CLOCKS_PER_SEC);
// #endif
// }

// vector<int> minHeadVector(vector<int>& vec){
// 	int minidx = 0;
// 	int minNum = vec[0];
// 	for(int i = 1; i < vec.size(); i++){
// 		if(vec[i] < minNum){
// 			minNum = vec[i];
// 			minidx = i;
// 		}
// 	}
// 	vector<int> tmp(vec);
// 	for(int i = 0; i < vec.size(); i++){
// 		tmp[i] = vec[(i + minidx) % vec.size()];
// 	}
// 	return tmp;
// }

// bool isHasList(vector<int>& vec){
// 	stringstream ss;
// 	//1#13#24#6# --- 1-13-24-6
// 	for(int i = 0; i < vec.size(); i++){
// 		ss << vec[i] << "#";
// 	}
// 	string s = ss.str();//cout << s << endl;
// 	if(str.find(s) != str.end()) return true;
// 	else str.insert(s);
// 	return false;
// }

// bool isSameVector(vector<int>& a, vector<int>& b){
//     if(a.size() != b.size()) return false;
//     for(int i = 0; i < a.size(); i++){
//         if(a[i] != b[i]) return false;
//     }
//     return true;
// }

void backtrack(vector<int> &list, vector<int> &visited, int depth, int key){
	if(depth == DEPTH) return;

	if(map.find(key) == map.end()) return;
	//key的list提取
	vector<int> keylist = map[key];
    if(keylist.size() == 0) return;
	for(int i = 0; i < keylist.size(); i++){
		//判断新加入的ID和第一个ID相同
		if(keylist[i] == list[0]){
			//判断深度大于3
			if(depth < 2) continue;
			//满足条件重新排序
			//vector<int> minheadvec(minHeadVector(list));
			//添加字符串查找是否存在，存在返回，不存在添加进set，vector
			res.insert(list);
		}//cout << keylist[i] << endl;
		//存在重复元素且不在第一位，直接返回
		//if(singlenum.find(keylist[i]) != singlenum.end()) continue;
		if(visited[keylist[i]] == 1) continue;
        if(keylist[i] < list[0]) continue;

		list.push_back(keylist[i]);
		visited[keylist[i]] = 1;
		//singlenum.insert(keylist[i]);
		backtrack(list, visited, depth + 1, keylist[i]);
		list.pop_back();
		visited[keylist[i]] = 0;
		//singlenum.erase(keylist[i]);
	}
	
}

bool cmp(vector<int> a, vector<int> b){
	if(a.size() == b.size()){
		for(int i = 0; i < a.size(); i++){
			if(a[i] == b[i]) continue;
			return a[i] < b[i];
		}
	}
	else return a.size() < b.size();
	return false;
}

void degreeSort(vector<int> &degrees, bool isTrue){
    queue<int> q;
    for(int i = 0;i < degrees.size(); i++){
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
    for(int i = 0; i < degrees.size(); i++){
        if(degrees[i] == 0){
            //map.erase(i);
            map[i].clear();
            cnt++;
        }
        else if(degrees[i] == -1) continue;
        else if(isTrue){
            sort(map[i].begin(),map[i].end());
        }
    }
#ifdef TEST
    printf("the number of Nodes remove: %d\n", cnt);
#endif
}

// void covbuild(){
//     for(const auto& ids : map){
//         auto &mi = ids.second;
//         for(int k : mi){
//             auto &mk = map[k];
//             for(int j : mk){
//                 if(ids.first != j){
//                     covmap[j][ids.first].push_back(k);
//                 }
//             }
//         }
//     }
//     for(const auto& idc : covmap){
//         for(const auto& ida : idc.second){
//             if(ida.second.size() > 1){
//                 sort(ida.second.begin(), ida.second.end());
//             }
//         }
//     }
// }

int main(int argc, char *argv[]){
	int start = clock();
#ifdef TEST
    string testFile = "/Users/alviney/test/data/testdata/26571/test_data.txt";
    string resultFile = "/Users/alviney/test/data/testdata/26571/result.txt";
#endif
#ifndef TEST
    string testFile = "/data/test_data.txt";
    string resultFile = "/projects/student/result.txt";
#endif
    loadTestData(testFile);

    int runtime = clock();
    vector<int> path;
    //标记存在的ID和访问路径
    vector<int> visited(MAX_ID + 1,-1);
    for(int i = 0; i < degreeList.size(); i++){
        visited[degreeList[i]] = 0;
    }
    //计算出度入度
    inDegrees = vector<int>(visited);
    outDegrees = vector<int>(visited);
    for(int i = 0; i < degreeList.size(); i += 2){
        outDegrees[degreeList[i]]++;
        inDegrees[degreeList[i+1]]++;
    }
    degreeSort(inDegrees, false);
    degreeSort(outDegrees, true);

    //covbuild();
    for(const auto& id : map){//cout << id.second.size() << endl;
            if(id.second.size() != 0){
            path.push_back(id.first);
            visited[id.first] = 1;
            //cout << "1: " << id.first << endl;
            backtrack(path, visited, 0, id.first);
            path.pop_back();
            visited[id.first] = 0;
        }

    }
#ifdef TEST     
    printf("the calculate time is: %.3f\n", float(clock() - runtime) / CLOCKS_PER_SEC);
#endif
    writeResultFile_fprintf(resultFile);
#ifdef TEST   
    printf("the all time is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
#endif
    return 0;
}