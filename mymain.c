/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/stackprotector.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/bootmem.h>
#include <linux/acpi.h>
#include <linux/tty.h>
#include <linux/percpu.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/kernel_stat.h>
#include <linux/start_kernel.h>
#include <linux/security.h>
#include <linux/smp.h>
#include <linux/profile.h>
#include <linux/rcupdate.h>
#include <linux/moduleparam.h>
#include <linux/kallsyms.h>
#include <linux/writeback.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/cgroup.h>
#include <linux/efi.h>
#include <linux/tick.h>
#include <linux/interrupt.h>
#include <linux/taskstats_kern.h>
#include <linux/delayacct.h>
#include <linux/unistd.h>
#include <linux/rmap.h>
#include <linux/mempolicy.h>
#include <linux/key.h>
#include <linux/buffer_head.h>
#include <linux/page_cgroup.h>
#include <linux/debug_locks.h>
#include <linux/debugobjects.h>
#include <linux/lockdep.h>
#include <linux/kmemleak.h>
#include <linux/pid_namespace.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/idr.h>
#include <linux/kgdb.h>
#include <linux/ftrace.h>
#include <linux/async.h>
#include <linux/kmemcheck.h>
#include <linux/sfi.h>
#include <linux/shmem_fs.h>
#include <linux/slab.h>
#include <linux/perf_event.h>
#include <linux/file.h>
#include <linux/ptrace.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>

#include <asm/io.h>
#include <asm/bugs.h>
#include <asm/setup.h>
#include <asm/sections.h>
#include <asm/cacheflush.h>

#ifdef CONFIG_X86_LOCAL_APIC
#include <asm/smp.h>
#endif

#include "mypcb.h"

tPCB my_task[MAX_TASK_NUM];
tPCB *my_current = NULL;

extern volatile int my_need_sched;

void my_process(void)
{
    unsigned count = 0;
    unsigned slice = sizeof(count);
    while (1)
    {
        count += 1;
        if (!(count<<slice))
        {
            if (my_need_sched == 1)
            {
                my_need_sched = 0;
                my_schedule();
            }
            else
            {
                printk(KERN_NOTICE "process %d is running\n", my_current->pid);
            }
        } 
    }
}

void __init my_start_kernel(void)
{
    int i;

    /* init task 0 */
    my_task[0].pid = 0;
    my_task[0].state = MY_RUNNING;
    my_task[0].thread.sp = (unsigned long)&my_task[0].stack[KERNEL_STACK_SIZE-1];
    my_task[0].entry = my_task[0].thread.ip = (unsigned long)my_process; 
    my_task[0].next = &my_task[0];

    /* then init other "processes" */
    for (i = 1; i < MAX_TASK_NUM; i++)
    {
        memcpy(&my_task[i], &my_task[0], sizeof(tPCB));
        my_task[i].pid = i;
        my_task[i].state = MY_SLEEP;
        my_task[i].thread.sp += (unsigned long)sizeof(tPCB);
        /* to make the list a big loop */
        my_task[i].next = my_task[i-1].next;
        my_task[i-1].next = &my_task[i];

    }
    
    /* going to switch to task 0! */
    printk(KERN_NOTICE "main going to switch to task 0\n");
    my_current = &my_task[0];
    asm volatile(
        "movl %1, %%esp\n\t"
        "movl %1, %%ebp\n\t"
        "pushl %0\n\t"
        "ret\n\t"
        :
        :"c"(my_task[0].thread.ip), "d"(my_task[0].thread.sp)
    );
}
