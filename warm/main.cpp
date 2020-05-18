#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

//#define TEST

const int MAXLEN = 10 * 1024 * 1024 * 10;//(0.234,)(1001)(1000)(10)10000nums
char buffer[MAXLEN];

struct Data {
    vector<double> features;
    int label;
    Data(vector<double> f, int l) : features(f), label(l)
    {}
};
struct Param {
    vector<double> wtSet;
};


class LR {
public:
    void train();
    void predict();
    int loadModel();
    int storeModel();
    LR(string trainFile, string testFile, string predictOutFile);

private:
    vector<Data> trainDataSet;
    vector<Data> testDataSet;
    vector<int> predictVec;
    Param param;
    string trainFile;
    string testFile;
    string predictOutFile;
    string weightParamFile = "modelweight.txt";

private:
    bool init();
    void meanData();
    bool loadTrainData();
    bool loadTestData();
    int storePredict(vector<int> &predict);
    void initParam();
    int sgn(double num);
    double wxbCalc(const Data &data);
    double sigmoidCalc(const double wxb);
    double lossCal();
    double gradientSlope(const vector<Data> &dataSet, int index, int j, const vector<double> &sigmoidVec);
    static bool loadData(int sub_len,char *sub_p, vector<Data> *trainset);
    bool loadTrainDataMulti();

private:
    int featuresNum;
    const double wtInitV = 0.5;
    double stepSize = 0.04;//0.02 0.824 0.04 0.8205 0.05
    const int maxIterTimes = 4;
    const double predictTrueThresh = 0.5;
    const int train_show_step = 1;
    //const int decay = 1;
    double lamda = 0.003;
    const int minDataSize = 10;
    static const int minReadFile = 8000;
};

LR::LR(string trainF, string testF, string predictOutF)
{
    trainFile = trainF;
    testFile = testF;
    predictOutFile = predictOutF;
    featuresNum = 0;
    init();
}

bool LR::loadTrainData()
{
    int start = clock();
    struct stat sb;
    char *buf;
    int fd = open(trainFile.c_str(), O_RDONLY);
    fstat(fd, &sb);
    buf = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    vector<double> feature;
    feature.reserve(1001);
    int col = 0;
    int line = 0;
    trainDataSet.reserve(minReadFile);
    
    bool nag = false;
    float tmp = 0.0f;
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        nag = false;
        if(*p == ',' || *p == '\n'){
            continue;
        }
        if(*p == '-'){
            nag = true;
            p++;
        }
        col++;
        if(col == 1001){
            trainDataSet.push_back(Data(feature, *p - '0'));
            line++;
            
            col = 0;
            feature.clear();
            if(line == minReadFile) break;
            continue;
        }
        //0.123
        tmp = *p - '0';
        p++;
        p++;
        tmp = tmp * 10 + *p - '0';
        p++;
        tmp = tmp * 10 + *p - '0';
        p++;
        tmp = tmp * 10 + *p - '0';
        
        if(nag) feature.push_back(-tmp / 1000.0);
        else feature.push_back(tmp / 1000.0);
    }
    
    close(fd);
    munmap(buf, sb.st_size);
    printf("the train file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    //printf(" train size %lu\n", trainDataSet.size());
    

    
    
    
    
//    int start = clock();
//    FILE *fp = fopen(trainFile.c_str(), "r");
//    int len = fread(buffer, 1, MAXLEN, fp);
//
//    vector<double> feature;
//    feature.reserve(1001);
//    int col = 0;
//    int line = 0;
//    trainDataSet.reserve(minReadFile);
//
//    bool nag = false;
//    float tmp = 0.0f;
//    for(char* p = buffer; *p && p - buffer <= len; p++){
//        nag = false;
//        if(*p == ',' || *p == '\n'){
//            continue;
//        }
//        if(*p == '-'){
//            nag = true;
//            p++;
//        }
//        col++;
//        if(col == 1001){
//            trainDataSet.push_back(Data(feature, *p - '0'));
//            line++;
//
//            col = 0;
//            feature.clear();
//            if(line == minReadFile) break;
//            continue;
//        }
//        //0.123
//        tmp = *p - '0';
//        p++;
//        p++;
//        tmp = tmp * 10 + *p - '0';
//        p++;
//        tmp = tmp * 10 + *p - '0';
//        p++;
//        tmp = tmp * 10 + *p - '0';
//
//        if(nag) feature.push_back(-tmp / 1000.0);
//        else feature.push_back(tmp / 1000.0);
//    }
//    fclose(fp);
//    printf("the train file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    //printf(" train size %lu\n", trainDataSet.size());
    //free(buffer);
    return true;
    
//    int start = clock();
//    ifstream infile(trainFile.c_str());
//    string line;
//
//    if (!infile) {
//        cout << "打开训练文件失败" << endl;
//        exit(0);
//    }
//
//    while (infile) {
//        getline(infile, line);
//        if (line.size() > featuresNum) {
//            stringstream sin(line);
//            char ch;
//            double dataV;
//            int i;
//            vector<double> feature;
//            i = 0;
//
//            while (sin) {
//                char c = sin.peek();
//                if (int(c) != -1) {
//                    sin >> dataV;
//                    feature.push_back(dataV);
//                    sin >> ch;
//                    i++;
//                } else {
//                    cout << "训练文件数据格式不正确，出错行为" << (trainDataSet.size() + 1) << "行" << endl;
//                    return false;
//                }
//            }
//            int ftf;
//            ftf = (int)feature.back();
//            feature.pop_back();
//            trainDataSet.push_back(Data(feature, ftf));
//        }
//    }
//    infile.close();
//    printf("the cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
//    return true;
}
bool LR::loadData(int sub_len,char *sub_p, vector<Data> *trainset){
    vector<double> feature;
    feature.reserve(1001);
    int col = 0;
    int line = 0;
          
    bool nag = false;
    float tmp = 0.0f;
    
    char* point = sub_p;
    while(*point!='\n'){
        point++;
    }//until next line start
   for(char* p = point; *p && p - sub_p <= sub_len; p++){
       nag = false;
       if(*p == ',' || *p == '\n'){
           continue;
       }
       if(*p == '-'){
           nag = true;
           p++;
       }
       col++;
       if(col == 1001){
           (*trainset).push_back(Data(feature, *p - '0'));
           line++;
           
           col = 0;
           feature.clear();
           if(line == minReadFile / 4) break;
           continue;
       }
       //0.123
       tmp = *p - '0';
       p++;
       p++;
       tmp = tmp * 10 + *p - '0';
       p++;
       tmp = tmp * 10 + *p - '0';
       p++;
       tmp = tmp * 10 + *p - '0';
       
       if(nag) feature.push_back(-tmp / 1000.0);
       else feature.push_back(tmp / 1000.0);
   }
   //free(buffer);
   return true;
}

bool LR::loadTrainDataMulti(){
    vector<Data> trainDataSet1;
    vector<Data> trainDataSet2;
    vector<Data> trainDataSet3;
    
    int start = clock();
    FILE *fp = fopen(trainFile.c_str(), "r");
    int len = fread(buffer, 1, MAXLEN, fp);
    trainDataSet.reserve(minReadFile);
    
    int buf_len = len / 4;
    char *p1 = buffer;
    char *p2 = p1 + buf_len;
    char *p3 = p2 + buf_len;
    char *p4 = p3 + buf_len;
    
    thread t1(loadData, buf_len, p1, &trainDataSet);
    thread t2(loadData, buf_len, p2, &trainDataSet1);
    thread t3(loadData, buf_len, p3, &trainDataSet2);
    thread t4(loadData, buf_len, p4, &trainDataSet3);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    trainDataSet.insert(trainDataSet.end(),trainDataSet1.begin(),trainDataSet1.end());
    trainDataSet.insert(trainDataSet.end(),trainDataSet2.begin(),trainDataSet2.end());
    trainDataSet.insert(trainDataSet.end(),trainDataSet3.begin(),trainDataSet3.end());
    
    printf("the train file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);printf(" train size %lu\n", trainDataSet.size());
    
    return true;
}

void LR::initParam()
{
    int i;
    for (i = 0; i < featuresNum; i++) {
        double randes = rand() % 100 / (double)101;
        param.wtSet.push_back(randes);
    }
}

void LR::meanData(){
    //mean
    vector<double> paramSum(featuresNum);
    for(int i = 0; i < trainDataSet.size(); i++){
        for(int j = 0; j < featuresNum; j++){
            paramSum[j] += trainDataSet[i].features[j];
        }
    }
    for(int j = 0; j < featuresNum; j++){
        paramSum[j] = paramSum[j] / trainDataSet.size();
    }
    //var
    vector<double> paramVar(featuresNum);
    for(int i = 0; i < trainDataSet.size(); i++){
        for(int j = 0; j < featuresNum; j++){
            paramVar[j] += pow(trainDataSet[i].features[j] - paramSum[j], 2.0);
        }
    }
    for(int j = 0; j < featuresNum; j++){
        paramVar[j] = paramVar[j] / (trainDataSet.size() - 1);
    }
    //normalization
    for(int i = 0; i < trainDataSet.size(); i++){
        for(int j = 0; j < featuresNum; j++){
            trainDataSet[i].features[j] = (trainDataSet[i].features[j] - paramSum[j]) / paramVar[j];
        }
    }
}

bool LR::init()
{
    trainDataSet.clear();
    bool status = loadTrainData();
    if (status != true) {
        return false;
    }
    featuresNum = trainDataSet[0].features.size();
    param.wtSet.clear();
    initParam();
    //meanData();
    return true;
}


double LR::wxbCalc(const Data &data)
{
    double mulSum = 0.0L;
    int i;
    double wtv, feav;
    for (i = 0; i < param.wtSet.size(); i++) {
        wtv = param.wtSet[i];
        feav = data.features[i];
        mulSum += wtv * feav;
    }

    return mulSum;
}

inline double LR::sigmoidCalc(const double wxb)
{
    double expv = exp(-1 * wxb);
    double expvInv = 1 / (1 + expv);
    return expvInv;
}


double LR::lossCal()
{
    double lossV = 0.0L;
    int i;

    for (i = 0; i < trainDataSet.size(); i++) {
        lossV -= trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet[i])));
        lossV -= (1 - trainDataSet[i].label) * log(1 - sigmoidCalc(wxbCalc(trainDataSet[i])));
    }
    lossV /= trainDataSet.size();
    return lossV;
}


// double LR::gradientSlope(const vector<Data> &dataSet, int index, const vector<double> &sigmoidVec)
// {
//     double gsV = 0.0L;
//     int i;
//     double sigv, label;
//     for (i = 0; i < dataSet.size(); i++) {
//         sigv = sigmoidVec[i];
//         label = dataSet[i].label;
//         gsV += (label - sigv) * (dataSet[i].features[index]);
//     }

//     gsV = gsV / dataSet.size();
//     return gsV;
// }

double LR::gradientSlope(const vector<Data> &dataSet, int index, int j, const vector<double> &sigmoidVec)
{
    double gsV = 0.0L;
    for (int i = j - minDataSize; i < j; i++) {
        gsV += (dataSet[i].label - sigmoidVec[i]) * (dataSet[i].features[index]);
    }

    gsV = gsV / minDataSize;
    return gsV;
}

int LR::sgn(double num){
    if(num > 0) return 1;
    else if(num == 0) return 0;
    else return -1;
}

void LR::train()
{
    int start = clock();
    double sigmoidVal;
    double wxbVal;
    int i, j;

    for (i = 0; i < maxIterTimes; i++) {
        vector<double> sigmoidVec;

        for (j = 0; j < trainDataSet.size(); j++) {
            wxbVal = wxbCalc(trainDataSet[j]);
            sigmoidVal = sigmoidCalc(wxbVal);
            sigmoidVec.push_back(sigmoidVal);
            if(j % minDataSize == 0 && j != 0) {
                for (int k = 0; k < param.wtSet.size(); k++) {
                    param.wtSet[k] += stepSize * gradientSlope(trainDataSet, k, j, sigmoidVec) - lamda * param.wtSet[k] / minDataSize;
                }
            }
        }

        // if(stepSize >= 0.05)
        //     stepSize = stepSize / (1 + decay * i);

//        if (i % train_show_step == 0) {
//            // cout << "iter " << i << ". updated weight value is : ";
//            // for (j = 0; j < param.wtSet.size(); j++) {
//            //     cout << param.wtSet[j] << "  ";
//            // }
//            // cout << endl;
//            double sigVal;
//            int predictVal;
//            int count = 0;
//            double accurate = 0;
//            for (int j = 0; j < trainDataSet.size(); j++) {
//                sigVal = sigmoidCalc(wxbCalc(trainDataSet[j]));
//                predictVal = sigVal >= predictTrueThresh ? 1 : 0;
//                if(predictVal == trainDataSet[j].label) count++;
//            }
//            accurate = ((double)count) / trainDataSet.size();
//            cout << "iter " << i << "the prediction accuracy is " << accurate << endl;
//        }
    }
    //printf("the train cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
}

void LR::predict()
{
    double sigVal;
    int predictVal;

    loadTestData();
    for (int j = 0; j < testDataSet.size(); j++) {
        sigVal = sigmoidCalc(wxbCalc(testDataSet[j]));
        predictVal = sigVal >= predictTrueThresh ? 1 : 0;
        predictVec.push_back(predictVal);
    }

    storePredict(predictVec);
}

int LR::loadModel()
{
    string line;
    int i;
    vector<double> wtTmp;
    double dbt;

    ifstream fin(weightParamFile.c_str());
    if (!fin) {
        cout << "打开模型参数文件失败" << endl;
        exit(0);
    }

    getline(fin, line);
    stringstream sin(line);
    for (i = 0; i < featuresNum; i++) {
        char c = sin.peek();
        if (c == -1) {
            cout << "模型参数数量少于特征数量，退出" << endl;
            return -1;
        }
        sin >> dbt;
        wtTmp.push_back(dbt);
    }
    param.wtSet.swap(wtTmp);
    fin.close();
    return 0;
}

int LR::storeModel()
{
    string line;
    int i;

    ofstream fout(weightParamFile.c_str());
    if (!fout.is_open()) {
        cout << "打开模型参数文件失败" << endl;
    }
    if (param.wtSet.size() < featuresNum) {
        cout << "wtSet size is " << param.wtSet.size() << endl;
    }
    for (i = 0; i < featuresNum; i++) {
        fout << param.wtSet[i] << " ";
    }
    fout.close();
    return 0;
}


bool LR::loadTestData()
{
    int start = clock();
    struct stat sb;
    char *buf;
    int fd = open(testFile.c_str(), O_RDONLY);
    fstat(fd, &sb);
    buf = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    vector<double> feature;
    feature.reserve(1000);
    int col = 0;
    int line = 0;
    testDataSet.reserve(20000);
    
    bool nag = false;
    float tmp = 0.0f;
    for(char* p = buf; *p && p - buf <= sb.st_size; p++){
        nag = false;
        if(*p == ',' || *p == '\n'){
            continue;
        }
        if(*p == '-'){
            nag = true;
            p++;
        }
        col++;
        if(col == 1001){
            testDataSet.push_back(Data(feature, 0));
            line++;
            
            col = 0;
            feature.clear();
            //if(line == minReadFile) break;
            continue;
        }
        //0.123
        tmp = *p - '0';
        p++;
        p++;
        tmp = tmp * 10 + *p - '0';
        p++;
        tmp = tmp * 10 + *p - '0';
        p++;
        tmp = tmp * 10 + *p - '0';
        
        if(nag) feature.push_back(-tmp / 1000.0);
        else feature.push_back(tmp / 1000.0);
    }
    
    close(fd);
    munmap(buf, sb.st_size);
    printf("the test file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    //printf(" test size %lu\n", testDataSet.size());
    
    
    
//    int start = clock();
//    FILE *fp = fopen(testFile.c_str(), "r");
//    int len = fread(buffer, 1, MAXLEN, fp);
//
//    vector<double> feature;
//    feature.reserve(1000);
//    int col = 0;
//    int line = 0;
//    testDataSet.reserve(20000);
//
//    bool nag = false;
//    float tmp = 0.0f;
//    for(char* p = buffer; *p && p - buffer <= len; p++){
//        nag = false;
//        if(*p == ',' || *p == '\n'){
//            continue;
//        }
//        if(*p == '-'){
//            nag = true;
//            p++;
//        }
//        col++;
//        if(col == 1001){
//            testDataSet.push_back(Data(feature, 0));
//            line++;
//
//            col = 0;
//            feature.clear();
//            //if(line == minReadFile) break;
//            continue;
//        }
//        //0.123
//        tmp = *p - '0';
//        p++;
//        p++;
//        tmp = tmp * 10 + *p - '0';
//        p++;
//        tmp = tmp * 10 + *p - '0';
//        p++;
//        tmp = tmp * 10 + *p - '0';
//
//        if(nag) feature.push_back(-tmp / 1000.0);
//        else feature.push_back(tmp / 1000.0);
//    }
//    fclose(fp);
//    printf("the test file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    //printf(" test size %lu\n", testDataSet.size());
    //free(buffer);
    return true;
    
    
//    int start = clock();
//    ifstream infile(testFile.c_str());
//    string lineTitle;
//
//    if (!infile) {
//        cout << "打开测试文件失败" << endl;
//        exit(0);
//    }
//
//    while (infile) {
//        vector<double> feature;
//        string line;
//        getline(infile, line);
//        if (line.size() > featuresNum) {
//            stringstream sin(line);
//            double dataV;
//            int i;
//            char ch;
//            i = 0;
//            while (i < featuresNum && sin) {
//                char c = sin.peek();
//                if (int(c) != -1) {
//                    sin >> dataV;
//                    feature.push_back(dataV);
//                    sin >> ch;
//                    i++;
//                } else {
//                    cout << "测试文件数据格式不正确" << endl;
//                    return false;
//                }
//            }
//            testDataSet.push_back(Data(feature, 0));
//        }
//    }
//
//    infile.close();
//    printf("the test file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
//    return true;
}

bool loadAnswerData(string awFile, vector<int> &awVec)
{
//    int start = clock();
//    struct stat sb;
//    char *buf;
//    int fd = open(awFile.c_str(), O_RDONLY);
//    fstat(fd, &sb);
//    buf = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
//    int tmp = 0;
//    for(char *p = buf; *p && p - buf <= sb.st_size; p++){
//        if(*p == '\n') continue;
//        tmp = *p - '0';//printf("res tmp: %d\n", tmp);
//        awVec.push_back(tmp);
//    }
//    close(fd);
//    munmap(buf, sb.st_size);
//    printf("the res file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
//    printf("size %lu\n", awVec.size());
//    return true;
    
    int start = clock();
    FILE *fp = fopen(awFile.c_str(),"r");
    int len = fread(buffer, 1, MAXLEN, fp);
    int tmp = 0;
    for(char *p = buffer; *p && p - buffer <= len; p++){
        if(*p == '\n') continue;
        tmp = *p - '0';//printf("res tmp: %d\n", tmp);
        awVec.push_back(tmp);
    }
    fclose(fp);
    //printf("the res file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    //printf("size %lu\n", awVec.size());
    return true;
    
//    int start = 0;
//    ifstream infile(awFile.c_str());
//    if (!infile) {
//        cout << "打开答案文件失败" << endl;
//        exit(0);
//    }
//
//    while (infile) {
//        string line;
//        int aw;
//        getline(infile, line);
//        if (line.size() > 0) {
//            stringstream sin(line);
//            sin >> aw;
//            awVec.push_back(aw);
//        }
//    }
//
//    infile.close();
//    printf("the res file cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
//    return true;
}

int LR::storePredict(vector<int> &predict)
{
    string line;
    int i;

    ofstream fout(predictOutFile.c_str());
    if (!fout.is_open()) {
        cout << "打开预测结果文件失败" << endl;
    }
    for (i = 0; i < predict.size(); i++) {
        fout << predict[i] << endl;
    }
    fout.close();
    return 0;
}

int main(int argc, char *argv[])
{
    int start = clock();
    vector<int> answerVec;
    vector<int> predictVec;
    int correctCount;
    double accurate;
    //line
     string trainFile = "/data/train_data.txt";
     string testFile = "/data/test_data.txt";
     string predictFile = "/projects/student/result.txt";
     string answerFile = "/projects/student/answer.txt";
    //local
//    string trainFile = "/Users/alviney/test/data/train_data.txt";
//    string testFile = "/Users/alviney/test/data/test_data.txt";
//    string predictFile = "/Users/alviney/test/projects/student/result.txt";
//    string answerFile = "/Users/alviney/test/projects/student/answer.txt";

    LR logist(trainFile, testFile, predictFile);

    //cout << "ready to train model" << endl;
    logist.train();

    //cout << "training ends, ready to store the model" << endl;
    logist.storeModel();

#ifdef TEST
    //cout << "ready to load answer data" << endl;
    loadAnswerData(answerFile, answerVec);
#endif

    //cout << "let's have a prediction test" << endl;
    logist.predict();

#ifdef TEST
    loadAnswerData(predictFile, predictVec);
    cout << "test data set size is " << predictVec.size() << endl;
    correctCount = 0;
    for (int j = 0; j < predictVec.size(); j++) {
        if (j < answerVec.size()) {
            if (answerVec[j] == predictVec[j]) {
                correctCount++;
            }
        } else {
            cout << "answer size less than the real predicted value" << endl;
        }
    }

    accurate = ((double)correctCount) / answerVec.size();
    cout << "the prediction accuracy is " << accurate << endl;
#endif
    printf("the all cost is: %.3f\n", float(clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
