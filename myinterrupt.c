/*
 *  linux/mykernel/myinterrupt.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/kernel_stat.h>
#include <linux/export.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/pid_namespace.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/tick.h>
#include <linux/kallsyms.h>
#include <linux/irq_work.h>
#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>

#define CREATE_TRACE_POINTS
#include <trace/events/timer.h>

#include "mypcb.h"

#define COUNT_SLICE (sizeof(unsigned long)*6)

volatile unsigned long time_count = 0;
volatile int my_need_sched = 0;
extern tPCB *my_current;


/*
 * Called by timer interrupt.
 */

void my_timer_handler(void)
{
    time_count += 1;
    if ((!(time_count<<COUNT_SLICE)) && my_need_sched != 1)
    {
        time_count = 0;
        my_need_sched = 1;
    }
}

void my_schedule(void)
{
	//printk(KERN_NOTICE "\n>>>>>>>>>>>>>>>>>my_timer_handler here<<<<<<<<<<<<<<<<<<\n\n");
    tPCB *next = my_current->next;
    tPCB *pre = my_current;

    if (!next)
    {
        printk(KERN_NOTICE "switch to NULL\n");
        while (1);
    }

    printk(KERN_NOTICE "task %d is going to task %d\n", my_current->pid, next->pid);
    my_current = next;

    if (next->state == MY_RUNNING)
    {
       asm volatile (
           "pushl %%ebp\n\t"
           "movl %%esp, %0\n\t"
           "movl $1f, %1\n\t"
           "movl %2, %%esp\n\t"
           "pushl %3\n\t"
           "ret\n\t"
           "1:\n\t"
           "popl %%ebp\n\t"
           :"=m"(pre->thread.sp), "=m"(pre->thread.ip)
           :"m"(next->thread.sp), "m"(next->thread.ip)
       ); 
        
        
    }
    else if (next->state == MY_SLEEP)
    {
        next->state = MY_RUNNING;
        asm volatile (
            "pushl %%ebp\n\t"
            "movl %%esp, %0\n\t"
            "movl $1f, %1\n\t"
            "movl %2, %%esp\n\t"
            "movl %2, %%ebp\n\t"
            "pushl %3\n\t"
            "ret\n\t"
            "1:\n\t"
            "popl %%ebp\n\t" 
            :"=m"(pre->thread.sp), "=m"(pre->thread.ip)
            : "m"(next->thread.sp), "m"(next->thread.ip) 
        );
    }
}
