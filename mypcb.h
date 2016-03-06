#define MAX_TASK_NUM 10
#define KERNEL_STACK_SIZE 1024*8

#define MY_RUNNING 1
#define MY_SLEEP 2
#define MY_DEAD 3

struct Thread
{
    unsigned long ip;
    unsigned long sp;
};

typedef struct PCB
{
    int pid;
    volatile long state;
    char stack[KERNEL_STACK_SIZE];
    struct Thread thread;
    unsigned long entry;
    struct PCB *next;
    unsigned long priority;
    
}tPCB;


void my_schedule(void);
