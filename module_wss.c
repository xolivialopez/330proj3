#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/moduleparam.h>

void addressSpace();
unsigned long int pid;
module_param(pid,long,S_IRUSR);

int pages(mm_struct* mm, unsigned long int address){
    int pageReturn;
    pgd_t *pgd;
    p4d_t *p4d;
    pmd_t *pmd;
    pud_t *pud;
    pte_t *ptep, pte;

    struct page *page = NULL;
    struct mm_struct *mm = current->mm;

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
    int current;
    for_each_process(task_list){
        if(task_list->pid == pid){
            struct vm_area_struct* area = task_list->mm->mmap;
            while(area != NULL){
                unsigned long int areaAddr = area->vm_start;
                while(areaAddr < area->vm_end){
                    struct mm_struct* mm = task_list->mm;
                    current = pages(mm, areaAddr);
                    if(current != 0){
                        wss += PAGE_SIZE;
                    }
                    areaAddr += PAGE_SIZE;
                }
                area = area->vm_next;
            }
            printk(KERN_INFO "[%ld]\t: [%d kB]", pid, wss);
            wss = 0;
        }
    }
}

int init_module(){
    printk(KERN_INFO "[PID]\t: [WSS]");
    addressSpace();
    return 0;
}

void cleanup_module(){
    printk(KERN_INFO "");
    return;
}