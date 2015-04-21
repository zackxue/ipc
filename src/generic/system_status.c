#include "system_status.h"

typedef struct CPU_PACKED 
{
	char name[20];    
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
}CPU_OCCUPY;

typedef struct MEN_PACKED
{
	char name[20];
	unsigned long total; 
	char name2[20];
	unsigned long free;                       
}MEM_OCCUPY;

static void get_memoccupy (MEM_OCCUPY *mem) 
{
    FILE *fd;          
    int n;             
    char buff[256];   
    MEM_OCCUPY *m;
    m=mem;
                                                                                                              
    fd = fopen ("/proc/meminfo", "r"); 
      
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u %s", m->name, &m->total, m->name2); 
    
    fgets (buff, sizeof(buff), fd); 
    sscanf (buff, "%s %u", m->name2, &m->free, m->name2); 
    
    fclose(fd);   
}

static int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
    unsigned long od, nd;    
    unsigned long id, sd;
    int cpu_use = 0;   
    
    od = (unsigned long) (o->user + o->nice + o->system +o->idle);
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);
      
    id = (unsigned long) (n->user - o->user); 
    sd = (unsigned long) (n->system - o->system);
    if((nd-od) != 0)
    cpu_use = (int)((sd+id)*10000)/(nd-od);
    //printf("cpu: %u\n",cpu_use);
    return cpu_use;
}

static void get_cpuoccupy (CPU_OCCUPY *cpust) 
{   
    FILE *fd;         
    int n;            
    char buff[256]; 
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;
                                                                                                               
    fd = fopen ("/proc/stat", "r"); 
    fgets (buff, sizeof(buff), fd);
    
    sscanf (buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);
    
    fclose(fd);     
}

static CPU_OCCUPY cpu_stat_old = {0};


float cpu_get_status()
{
    CPU_OCCUPY cpu_stat_new;
    MEM_OCCUPY mem_stat;
    float cpu;
    
    //获取内存
    //get_memoccupy ((MEM_OCCUPY *)&mem_stat);
    
    if(strlen(cpu_stat_old.name) == 0){
    	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat_old);
    	//sleep(10);
		return 1.0;
    }
    
    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat_new);
    
    //计算cpu使用率
    cpu = (float)cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat_old, (CPU_OCCUPY *)&cpu_stat_new)/100;


	printf("cpu status: %4.2f\r\n", cpu);
    return cpu;
} 



