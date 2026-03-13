##CMU-cacheLab##
##-该项目是卡内基梅隆大学的一个课程项目-##

目标：实现一个缓存模拟器，能够替换缓存中的数据，并且统计命中hit、未命中miss、驱逐eviction 的次数。
并且要优化 32×32、64×64、61×67 矩阵的转置函数，使未命中次数尽可能低。

将其拆解为两部分，PartA与PartB

##PartA##
PartA 要写一个 csim.c 文件，实现一个高速缓存模拟器，模拟缓存的替换并统计命中、未命中与驱逐的次数，并且能够根据不同的s、E、b进行初始配置。首先定义结构体Cache、Set、Line，Cache中包含s、E、b的参数以及指向Set的指针，Set中包含指向Line 的指针，这里的Set就对应上了缓存中的组，而Line就对应上了缓存中的行，即一个具体的缓存数据，其中包含有效位v，标签位tag和LRU的时间戳time。
定义完结构体后，我们先写一个初始化缓存空间的函数Init_cache(int s,int E,int b)，能够根据输入的s、E、b申请对应大小的空间用于存放缓存，并对缓存空间中各参数的值初始化为零。

![image](https://github.com/elowkka/cachelab-CMU/blob/main/png/%E7%BC%93%E5%AD%98%E7%A9%BA%E9%97%B4%E7%A4%BA%E6%84%8F%E5%9B%BE.png)

初始化缓存空间后，我们着手开始实现对缓存的读取和替换，即实现access_cache(Cache cache,int op_tag,int op_set,int E)函数。该函数首先要判断对于传进来的set与tag是否命中，若命中，则命中次数+1；若未命中，则miss次数+1，并且需要判断是冷不命中还是冲突不命中，若为冲突不命中，eviction 次数+1。不命中时，还需要判断是否有空位来存入缓存，因此写了一个判断是否有空位的函数is_empty(Cache cache,int op_set,int E)，若有空位，则直接存入；若无空位，则要按实验指导中的 LRU 方法进行判断，将上次被读取时间最久的位置进行替换。
这里就需要用到我们之前在Line 里定义的时间戳time 了，每次访问缓存，未被访问到的位置的time就要+1，若被访问到了，则时间戳time清零。因此time最大的就是上次读取时间最久远的位置，我们就替换这个位置。因此还需要写一个寻找最大时间戳的函数find_max_time(Cache cache,int op_set,int E)来帮我们找到要被替换的位置。 
此外，还有 print_help()，free_cache()函数来打印帮助信息和手动清理缓存空间。在主函数中，用到了内置函数getopt来读取输入的缓存模拟参数和tracefile文件，将输入的指令解析为s、E、b、t、v等参数。之后进行tracefile文件的读取，将读到的数据进行解析，分块为指令类型op_type，地址块address和大小size，再将地址块分块为tag、set，根据不同的op_type，执行不同次数的缓存访问。最后手动清零缓存空间，打印我们统计的命中、未命中和驱逐的次数。

![image]([https://github.com/elowkka/cachelab-CMU/blob/main/png/%E7%BC%93%E5%AD%98%E7%A9%BA%E9%97%B4%E7%A4%BA%E6%84%8F%E5%9B%BE.png](https://github.com/elowkka/cachelab-CMU/blob/main/png/%E5%9C%B0%E5%9D%80%E5%9D%97%E7%A4%BA%E6%84%8F%E5%9B%BE.png))
