#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

void addressSpace(void);
int pages(struct mm_struct* mm, unsigned long int address);

unsigned long int pid;
module_param(pid,long,S_IRUSR);

int pages(struct mm_struct* mm, unsigned long int address){
    int pageReturn;
    pgd_t *pgd;
    p4d_t *p4d;
    pmd_t *pmd;
    pud_t *pud;
    pte_t *ptep, pte;

    pgd = pgd_offset(mm, address);                    // get pgd from mm and the page address
    if (pgd_none(*pgd) || pgd_bad(*pgd)){           // check if pgd is bad or does not exist
        return 0;
    }

    p4d = p4d_offset(pgd, address);                   // get p4d from from pgd and the page address
    if (p4d_none(*p4d) || p4d_bad(*p4d)){          // check if p4d is bad or does not exist
        return 0;
    }

    pud = pud_offset(p4d, address);                   // get pud from from p4d and the page address
    if (pud_none(*pud) || pud_bad(*pud)){          // check if pud is bad or does not exist
        return 0;
    }

    pmd = pmd_offset(pud, address);               // get pmd from from pud and the page address
    if (pmd_none(*pmd) || pmd_bad(*pmd)){	   // check if pmd is bad or does not exist
        return 0;
    }

    ptep = pte_offset_map(pmd, address);      // get pte from pmd and the page address
    if (!ptep){
        return 0;
    }                                         // check if pte does not exist
    pte = *ptep;
    pageReturn = pte_young(pte);
    pte_mkold(pte);
    return pageReturn;
}

void addressSpace(){
    struct task_struct *task_list;
    int wss = 0;
    int curr;
    int final;
    unsigned long int areaAddr;
    struct vm_area_struct* area;
    struct mm_struct* mm;
    
    for_each_process(task_list){
        if(task_list->pid == pid){
            mm = task_list->mm;
            area = task_list->mm->mmap;
            while(area != NULL){
            	areaAddr = area->vm_start;
                while(areaAddr < area->vm_end){
                    curr = pages(mm, areaAddr);
                    if(curr != 0){
                        wss += PAGE_SIZE;
                    }
                    areaAddr += PAGE_SIZE;
                }
                area = area->vm_next;
            }
            final = wss/1024;
            printk(KERN_INFO "[%ld]\t: [%d kB]", pid, final);
            wss = 0;
        }
    }
}

// Set interval to 1 second
unsigned long timer_interval_ns = 5e9;
static struct hrtimer hr_timer;

// Callback executed periodically, reset timer
enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
    ktime_t currtime , interval;
    currtime  = ktime_get();
    interval = ktime_set(0,timer_interval_ns);
    hrtimer_forward(timer_for_restart, currtime , interval);
    addressSpace();
    return HRTIMER_RESTART;
}

// Initialize timer with callback
static int timer_init(void) {
    ktime_t ktime;
    ktime = ktime_set( 0, timer_interval_ns );
    hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    hr_timer.function = &timer_callback;
    hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );  // Start timer
    return 0;
}

// Remove timer
static void timer_exit(void) {
    int ret;
    ret = hrtimer_cancel( &hr_timer );
    if (ret) printk("Timer was still in use!\n");
    printk("HR Timer removed\n");
}
MODULE_LICENSE("GPL");

int init_module(){
    printk(KERN_INFO "[PID]\t: [WSS]");
    addressSpace();
    timer_init();
    return 0;
}

void cleanup_module(){
    printk(KERN_INFO "");
    timer_exit();
    return;
}
