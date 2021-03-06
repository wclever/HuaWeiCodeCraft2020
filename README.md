热身赛146/944 0.96    西北赛区：初赛 40/975 0.4798   复活赛区：复赛 a榜 26/266  b榜 4

> 题目简述：
>
> 热身赛：机器学习二分类，训练算法提升时间和准确度，根据时间和准确度综合判分
>
> 初赛：循环转账，寻找全部循环路径在[3,7]之间
>
> 复赛a榜：在初赛要求基础上使得前后转账金额在一定范围内
>
> 复赛b榜：转账金额整数变为浮点数，环数改为[3,8]之间

**trick:(时间根据1963w结果计算)**

读入数据100ms

* 使用mmap方式读入最快，200w条读入存储小于
* `,`方式分割数据，`\n`划分转账金额
* 换行通配符（很多大佬复赛b榜滑铁卢）由于window下的换行是`\r\n`CRLF，Linux下是`\n`LF，所以每次判读前先判断`\r`后判断`\n`
* 数据使用数组存储，边的顶点存在一起，金额另外数组存储

预处理，建图1.6s偏慢

* 拷贝一份输入数组排序去重统计顶点数量
* 使用unordered_map映射成连续的顶点ID
* 使用vector存储顶点转换的结果字符串`"123,"`,`"123\n"`
* 初赛数据小使用二维数组建图，复赛使用vector\<vector\<Pair>>方式存图（比自己定义的结构体快），使用sort对每个点的邻接边排序
* 统计出入度
* 拓扑排序（去掉不成环的点）实际作用不大，后期去掉了

找环

* 使用4+3双向搜索的方式（可以根据出入度对比分别使用4+3和3+4得到3.5+3.5）
* 使用unordered_map<uint32_t, vector\<Index>>存储反向搜索得到的反向边并且需要排序后再找环
* 采用抢占负载均衡多线程找环，atomic_flag，四个线程搜索未置位的点找环
* dfs方式使用for循环展开成反3+正4
* 减少使用if continue的操作，使得程序强行跳转

写入文件

* 使用new数组存储结果数据，重新连接负载均衡找环的结果
* 使用mmap写文件，将结果字符串拼接，使用strcpy传入内存
* ftruncate重新划定文件大小

其他没来的及实现的trick

* mmap多线程读取

* 前向星存图

* unordered_map换成robin_map或者dense hash，据说比unordered_map快十倍，

* sort针对大部分有序数组不友好，可以换插入排序

* cachedline对齐，提高内存使用效率

* 可节省压缩使用的字节存储类型，uint8_t

* memcpy的16对齐

* 结果过程转换字符或者找环时转换

* 写入文件并行

### 上分经历

#### 初赛

**1.官方样例**

> 官方31%正确率和68%正确率，使用ifstream读取文件，使用梯度下降GD算法

**2.改为SDG随机梯度下降**

> 使用fread读取文件
>
> 正确率74%，时间81s

**3.mmap读取文件，改为批SGD算法**

> 正确率74%，时间1.6s

**4.调整参数，归一化全部特征，多线程读取文件**

> 正确率84%，时间0.96s

#### 初赛

**1.main1.cpp**

> mmap读取数据
>
> unordered_map存取图信息并排序
>
> 使用朴素dfs递归实现，vector判断*
>
> *visited标记是否环有重复节点*
>
> *set作为结果的集合，减少判断排序*
>
> *每个顺序的下一个节点不能超过首结点，避免排序*
>
> *加入入度判断*
>
> *输出文件用fpringf*
>
> *使用拓扑排序删除入度出度0的key*
>
> 线上：13.9502s

**2.main2.cpp**

> *mmap写入*
>
> *做ID映射*
>
> *用pair读取数据并排序*
>
> *能用数组就用数组，效率比vector快的多*
>
> *vector比unordermap快很多*
>
> *lower_bound求第一个不大于首ID的ID*
>
> 找环改成使用6+1
>
> 线上：4.9420s

**3.main3.cpp**

> 找环使用5+2
>
> 线上：3.2477s

**4.main6.cpp**

> 使用3邻域和6+1，通过正搜索三层和反向搜索三层求得能够访问到的节点，再进行6+1找环
>
> 用二维静态数组存图
>
> 线上：2.6497s

**5.main10.cpp**

> 数组基数排序统计ID数量
>
> 根据数据滤掉5W以上的节点
>
> 去掉拓扑排序
>
> 使用整数数组存结果
>
> 求领域进行dfs的for展开
>
> 采用八线程根据ID数量进行比例划分1，2，3，4...份，前面的ID划分数量越少
>
> 线上：0.4798s~0.8939s

#### 复赛

**1.初始版本.  main511.cpp**

> 实现初赛终版6+1四线程，并全部改回使用STL的方式
>
> 线上分数：178.6279s

**2.改成4+3.  main513.cpp**

> 使用vector<vector\<Index>>方式存储反向边 后改为unordered_map
>
> 线上分数：10.0126s

**3.使用抢占式调度.  main516.cpp**

> atomic_flag方式抢占调度
>
> 没有测试线上效果

**复赛b榜.  main5161.cpp**

> 4+3方式直接改为4+4方式，抢占式调度版本，转账金额使用double存储，结果错误
>
> 非抢占调度版本，转账金额使用double存储，结果错误
>
> 抢占式版本，转账金额使用long long存储，结果通过，时间10.4028s
>
> 然后就没时间改了。。。

#### 感想

> 自己做比赛的确精力不够，很多算法思路自己都没来得及实现，听大家交流经验发现其实这些思路的确有很多作用，获得很多代码风格和性能优化的tips，而且终于知道为什么同一个解题方法，实现的数据结构和代码流程不同，效率会有天差地别，初赛前排大佬0.01s～后排1000s，复赛数据集28w----200w，结果由289w----2000w，排行榜的时间变为1.7s～1000s，差距差的很大，有良好的代码格式和意识很重要。

#### 大佬的分析总结

* [华为软件精英挑战赛-2020-初赛复赛-题目分析/算法Baseline （求出有向图中所有的环）](https://zhuanlan.zhihu.com/p/125764650)

* [华为2020软件精英挑战赛成渝赛区初赛赛后方案分享](https://blog.csdn.net/qq_34914551/article/details/105788200)

* [记2020年华为软件精英挑战赛（初赛)](https://zhuanlan.zhihu.com/p/136785097)

* [武长赛区hust_1037参赛经验](https://github.com/trybesthbk/HuaweiCodeCraft2020)

* [2020华为软件精英挑战赛-武长赛区-团队“您吃了么”初赛/复赛代码分享](https://github.com/liuwentao1992/HuaWei_CodeCraft_2020)

#### 各赛区前排大佬开源

* [My thoughts of 2020 Huawei Code Craft.----ddd](https://github.com/justarandomstring/2020-Huawei-Code-Craft)

* [2020华为软挑初赛粤港澳赛区第一，复赛粤港澳赛区A榜第一解决方案](https://github.com/cxq80803716/2020codecraft)

* [2020华为软挑初赛武长赛区第一，复赛武长赛区A榜第二解决方案](https://github.com/yoghurt-lee/HuaWeiCodeCraft2020)

* [华为2020软件精英挑战赛复赛代码开源-京津东北赛区1504b,复赛Ａ榜 rank1](https://github.com/WavenZ/CodeCraft2020)

* [粤港澳复赛A榜第2，全国第12](https://github.com/CodeCraft2020/CodeCraft2020)

#### 其他

* [民间测试数据集](https://github.com/liusen1006/2020HuaweiCodecraft-TestData)

* [ddd2020](https://github.com/justarandomstring/2020-Huawei-Code-Craft)

* [数据生成](https://github.com/byl0561/HWcode2020-TestData)

