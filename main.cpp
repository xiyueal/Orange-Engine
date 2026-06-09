#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

struct maps_entry {
    uint64_t    vm_start;
    uint64_t    vm_end;
    uint8_t     perm_read;
    uint8_t     perm_write;
    uint8_t     perm_exec;
    uint8_t     perm_shared;
    char        path[256];
} __attribute__((packed));

struct maps_header {
    int32_t     pid;
    int32_t     count;
    uint32_t    entry_size;
    uint64_t    total_size;
} __attribute__((packed));


class OrangeKernel {
  public:
    OrangeKernel(char *Key);   // 初始化对接
    bool DockOrangeKernel(pid_t pid,bool IsKernelTouch);  // 正式对接,可以重复修改pid来实现管理不同进程的内存
    void Exit(void);

    void KTouch_Down(int slot,int x, int y);//手指编号，xy坐标
    void KTouch_Up(int slot);//手指编号

    bool Read(uintptr_t addr, void *buffer, size_t size);
    bool Write(uintptr_t addr, void *buffer, size_t size);

    uintptr_t Get_ModuleBase(char *name);
    uintptr_t Get_ModuleBaseBss(char *name);

    struct maps_header *GetAllMaps(pid_t pid);//新增，获取某pid进程的完整maps信息

    template <typename T> T read(uintptr_t addr);    

    template <typename T> void read_queue(uintptr_t addr,void *buffer,int offset1);    
    template <typename T> void read_queue(uintptr_t addr,void *buffer,int offset1,int offset2);    
    template <typename T> void read_queue(uintptr_t addr,void *buffer,int offset1,int offset2,int offset3);    
    template <typename T> void read_queue(uintptr_t addr,void *buffer,int offset1,int offset2,int offset3,int offset4);    
    template <typename T> void read_queue(uintptr_t addr,void *buffer,int offset1,int offset2,int offset3,int offset4,int offset5);
    void Read_Queue_Async();
        
    template <typename T> T read_queue_now(uintptr_t addr,int offset1);    
    template <typename T> T read_queue_now(uintptr_t addr,int offset1,int offset2);    
    template <typename T> T read_queue_now(uintptr_t addr,int offset1,int offset2,int offset3);    
    template <typename T> T read_queue_now(uintptr_t addr,int offset1,int offset2,int offset3,int offset4);    
    template <typename T> T read_queue_now(uintptr_t addr,int offset1,int offset2,int offset3,int offset4,int offset5);
    
  private:
    int Read_Queue(uintptr_t addr,int count, int offset[],void *buffer, size_t size);
    int Read_Queue_Now(uintptr_t addr,int count, int offset[],void *buffer, size_t size);  
};

OrangeKernel* OrangeKernelTool;

#include "ReadFunc.h"
//在main里声明类指针
//其他地方用extern OrangeKernel* OrangeKernelDock;即可



extern uint64_t getCurrentTimestamp();
long libbase;

int main(int argc, char *argv[]) {


    OrangeKernelTool = new OrangeKernel("key");
    //key是您申请使用的密钥，请妥善保管，禁止倒卖。

    OrangeKernelTool->DockOrangeKernel(26251,true);
    //26251代表您需要进行内存管理的进程pid，true代表使用内核级模拟触控。
    //请确保您有对该进程进行内存管理的行为是合法合规的，然后再使用本工具对该进程进行内存管理或其他操作。
    
    for(int i=100;i<1000;i+=5){
        OrangeKernelTool->KTouch_Down(8,i,i*2);
        usleep(20000);
    }
    //这里是将第九根手指进行模拟滑动，8代表手指序号，从0-9共十指。
    
    OrangeKernelTool->KTouch_Up(8);
    //这里松下前面按下的手指。
    
    
    //读取进程的动态库基址
    libbase=OrangeKernelTool->Get_ModuleBase("libselinux.so");
    printf("base = %lx\n", libbase);
    //lib123456.so是指需要读取的动态库文件名
    
    int result;
    long number = 100000;
    
	uint64_t now = getCurrentTimestamp();
	for (long i = 0; i < number; i++)
	{
	   result =OrangeKernelTool->read<long>(libbase+0x4);
	}	
	printf("run count %ld times cost = %lfs\n", number, (double)(getCurrentTimestamp() - now) / 1000);
    printf("result = %d\n", result);
    //正常读取


	now = getCurrentTimestamp();
	result = 0;
	for (long i = 0; i < number; i++)
	{
    	OrangeKernelTool->read_queue<long>(libbase,&result,0x11259798);
    }
	OrangeKernelTool->Read_Queue_Async();
    printf("run count %ld times cost = %lfs\n", number, (double)(getCurrentTimestamp() - now) / 1000);
    printf("result = %d\n", result);
	/*队列读取规则如下
	1.read_queue后不会立刻读取数据，而是向缓冲区内发送读取命令
	2.手动使用Read_Queue_Async函数，或者缓冲区满了(10240个读取命令)，内核会一次性将缓冲区的读取命令全部完成。
	3.read_queue(addr,buffer,offsetx)
	addr是读取基地址，buffer是最终读取后需要写入到的变量地址，offset是指针链条
	对于指针链条，规则如下
	int a = read<int>(read<uintptr_t>(addr+0x10)+0x20) 等效于 read_queue<uintptr_t>(addr,&a,10,20)
	uintptr_t a = read<uintptr_t>(read<uintptr_t>(addr+0x10)+0x20) 等效于 read_queue<uintptr_t>(addr,&a,10,20)
	
	对于read_queue中的offsetx，最多支持五级链条。
	*/	
    //queue读取

	now = getCurrentTimestamp();
	result = 0;
	for (long i = 0; i < number; i++)
	{
	   result =OrangeKernelTool->read_queue_now<int>(libbase,0x11259798);
	}	
    printf("run count %ld times cost = %lfs\n", number, (double)(getCurrentTimestamp() - now) / 1000);
    printf("result = %d\n", result);
	/*read_queue_now读取规则如下
	1.read_queue_now后立刻读取数据,并返回读取数据	
	2.对于read_queue_now中的offsetx，最多支持五级链条。
	3.参数使用和read_queue基本一致，差别在于可以直接返回数据
	*/	
    
    //这里测试遍历读取全部maps信息
    struct maps_header *maps=OrangeKernelTool->GetAllMaps(getpid());
    printf("PID: %d, Entries: %d, Total: %lu bytes\n",
           maps->pid, maps->count, (unsigned long)maps->total_size);
    printf("%-36s %-5s %-10s %-6s %-10s %s\n",
           "Address Range", "Perms", "Offset", "Dev", "Inode", "Path");
    printf("----------------------------------------------------------------------\n");
    struct maps_entry *entries = (struct maps_entry *)((char *)maps + sizeof(struct maps_header));

    for (int i = 0; i < maps->count; i++) {
        struct maps_entry *e = &entries[i];
        printf("%012lx-%012lx %c%c%c%c %s\n",
               (unsigned long)e->vm_start,
               (unsigned long)e->vm_end,
               e->perm_read   ? 'r' : '-',
               e->perm_write  ? 'w' : '-',
               e->perm_exec   ? 'x' : '-',
               e->perm_shared ? 's' : 'p',
               e->path);
    }               
    free(maps);
    //记得回收内存

    
    char TestMemory[10];   
    //这里是大区域一次性读取，自定义读取任意大小的内存，并写入区域。
    OrangeKernelTool->Read(libbase,&TestMemory,sizeof(TestMemory));
    printf("\n %s",TestMemory);
    
    OrangeKernelTool->Exit();//清理内核读取缓存

    return 0;
}
