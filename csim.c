/*
stu-name:刘志杰
stu-ID:22307130110
*/


#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//定义结构体
typedef struct{  //行
  int v;
  int tag;
  int time;
}Line;

typedef struct{  //组
  Line *lines;
}Set;

typedef struct{  //缓存空间
  int S;
  int E;
  int B;
  Set *sets;
}Cache;


int hit,miss,evict=0;   //命中、未命中、驱逐的次数
int verbose = 0;   //verbose模式，高有效

//初始化缓存空间
Cache Init_cache(int s,int E,int b){
  int S = 1<<s;   //S=2^s
  int B = 1<<b;   //B=2^b
  Cache cache;
  cache.S = S;
  cache.E = E;
  cache.B = B;
  cache.sets = (Set*)malloc(S*sizeof(Set));   //为组申请缓存空间
  int i=0,j=0;
  for(i = 0;i < S;i++){
    cache.sets[i].lines = (Line*)malloc(sizeof(Line)*E);    //为行申请缓存空间
    for(j = 0;j < E;j++){   //初始化行内参数
      cache.sets[i].lines[j].v = 0;
      cache.sets[i].lines[j].tag = 0;
      cache.sets[i].lines[j].time = 0;
    }
  }
  return cache;
}

//判断是否有空位
int is_empty(Cache cache,int op_set,int E){
  int i = 0;
  for(i=0;i<E;i++){
    if(cache.sets[op_set].lines[i].v == 0){
      return i;
    }
  }
  return -1;
}

//更新缓存空间
void update_cache(Cache cache,int pos,int op_tag,int op_set){
  int j = 0;
  cache.sets[op_set].lines[pos].v = 1;
  cache.sets[op_set].lines[pos].tag = op_tag;
  for(j=0;j<cache.E;j++){ //更新时间戳
    if(cache.sets[op_set].lines[j].v == 1){
      cache.sets[op_set].lines[j].time++;
    }
  }
  cache.sets[op_set].lines[pos].time = 0; //被访问的行时间戳清零
}

//寻找最大的时间戳，LRU规则
int find_max_time(Cache cache,int op_set,int E){
  int k = 0;
  int max = 0;
  int index = 0;
  for(k=0;k<E;k++){
    if(cache.sets[op_set].lines[k].time > max){
      max = cache.sets[op_set].lines[k].time;
      index = k;
    }
  }
  return index;
}

//访问缓存
void cache_access(Cache cache,int op_tag,int op_set,int E){
  int i=0;
  int state = 0;
  for(i=0;i<E;i++){   //判断命中还是未命中
    if(cache.sets[op_set].lines[i].v == 1 && cache.sets[op_set].lines[i].tag == op_tag){
      state = i;
      break;
    }
    else
      state = -1;
  }
  
  if(state == -1){  //未命中
    miss++;   //未命中次数+1
    if(verbose) printf("miss ");
    int pos = is_empty(cache,op_set,E);
      if(pos==-1){  //若无空位，则判定为驱逐
        evict++;   //驱逐次数+1
        if(verbose) printf("eviction ");
        pos = find_max_time(cache,op_set,E);
      }
      update_cache(cache,pos,op_tag,op_set);  //若有空位，替换到空位；否则按LRU规则，替换时间戳最大的位置
  }
  
  else{   //命中
    hit++;    //命中次数+1
    if(verbose) printf("hit ");
    update_cache(cache,state,op_tag,op_set);  //清空时间戳
  }
  
}

//打印帮助信息
void print_help(){
  printf("Help information:\n");
  printf("Usage: ./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");

}

//释放缓存空间
void free_cache(Cache cache) {
    for (int i = 0; i < cache.S; i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
}


int main(int argc,char **argv)
{
    int s=0,E=0,b=0;
    char *tracefile = NULL;
    char c;
    char op_type;
    unsigned long address;
    int size;
    
    while((c=getopt(argc, argv, "s:E:b:t:vh"))!=-1){  //读取指令
      switch(c){
        case 's': s = atoi(optarg);break;
        case 'E': E = atoi(optarg);break;
        case 'b': b = atoi(optarg);break;
        case 't': tracefile = optarg;break;
        case 'v': verbose = 1;break;
        case 'h': print_help();return 0;
        default: printf("invalid\n");return 1;
        }
    }
    
    if(!tracefile){
      printf("No tracefile");
      return 1;
    }
    
    Cache cache = Init_cache(s,E,b);
    
    FILE *file = fopen(tracefile,"r");
    if(!file){
      printf("Can't open tracefile");
      return 1;
    }
    
    while(fscanf(file,"%c %lx,%d",&op_type,&address,&size)>0){  //解析tracefile
      int op_tag = address >> (s + b);
      int op_set = (address >> b) & ((unsigned)(-1) >> (8*sizeof(unsigned)-s));
      switch(op_type){
        case 'M': if (verbose) printf("%c %lx,%d ", op_type, address, size);  //若为verbose模式，打印相关参数
                  cache_access(cache,op_tag,op_set,E);
                  cache_access(cache,op_tag,op_set,E);
                  break;
        case 'L': if (verbose) printf("%c %lx,%d ", op_type, address, size);
                  cache_access(cache,op_tag,op_set,E);
                  break;
        case 'S': if (verbose) printf("%c %lx,%d ", op_type, address, size);
                  cache_access(cache,op_tag,op_set,E);
                  break;
      }
      if (verbose) 
        printf("\n");
    }
    fclose(file);
    free_cache(cache);  //释放空间
    printSummary(hit, miss, evict);   //打印统计信息
    
    return 0;
}
