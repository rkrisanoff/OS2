#include "lkmfs.h"



int get_multiprocess_signals_info(int pid)
{
    char output[BUFFER_SIZE];
    char buff_int[20];
    strcat(output, "get_multiprocess_signals_info -> ");
    sprintf(buff_int, "%d", pid);
    strcat(output, buff_int);
    strcat(output, "\n");

    struct task_struct *task = NULL;
    struct pid *pid_struct = find_get_pid(pid);

    if (!pid_struct)
    {
        return -1;
    }
    task = get_pid_task(pid_struct, PIDTYPE_PID);

    strcat(output, "tgid of current task -> ");
    sprintf(buff_int, "%d", task->tgid);
    strcat(output, buff_int);
    strcat(output, "\n");

    struct multiprocess_signals *delayed_signals;
    hlist_for_each_entry(delayed_signals, &task->signal->multiprocess, node)
    {
        sigset_t *signal = &delayed_signals->signal;

        if (signal)
        {
            strcat(output, "signal:");
            sprintf(buff_int, "%lu", (unsigned long)signal);
            strcat(output, buff_int);
            strcat(output, "\n");
        }
    }
    strcat(output, "\n");

    if (task->last_siginfo)
    {
        strcat(output, "si_code ");
        sprintf(buff_int, "%d", task->last_siginfo->si_code);
        strcat(output, buff_int);
        strcat(output, "\n");
        strcat(output, "si_errno ");
        sprintf(buff_int, "%d", task->last_siginfo->si_errno);
        strcat(output, buff_int);
        strcat(output, "\n");

        strcat(output, "si_signo ");
        sprintf(buff_int, "%d", task->last_siginfo->si_signo);
        strcat(output, buff_int);
        strcat(output, "\n");
    }
    else
    {
        strcat(output, "task->last_siginfo is NULL");
    }
    printk(KERN_INFO "%s", output);

    return 0;
};
struct page *get_current_page(struct mm_struct *mm, long virtual_address)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page = NULL;
    pgd = pgd_offset(mm, virtual_address);
    if (!pgd_present(*pgd))
    {
        return NULL;
    }
    p4d = p4d_offset(pgd, virtual_address);
    if (!p4d_present(*p4d))
    {
        return NULL;
    }
    pud = pud_offset(p4d, virtual_address);
    if (!pud_present(*pud))
    {
        return NULL;
    }
    pmd = pmd_offset(pud, virtual_address);
    if (!pmd_present(*pmd))
    {
        return NULL;
    }
    pte = pte_offset_kernel(pmd, virtual_address);
    if (!pte_present(*pte))
    {
        return NULL;
    }
    page = pte_page(*pte);
    return page;
};
int get_page_info(int pid, struct lab_page *lp)
{
    struct task_struct *t = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    printk(KERN_INFO "task_struct->%p\n", t);

    if (t == NULL)
    {
        return 0;
    }
    else
    {
        struct page *page_struct;
        struct mm_struct *mm = t->mm;
        printk(KERN_INFO "mm->%p\n", mm);
        if (mm == NULL)
        {
            return 0;
        }
        else
        {
            struct vm_area_struct *vas = mm->mmap;
            unsigned long virtual_address;

            for (virtual_address = vas->vm_start; virtual_address <= vas->vm_end; virtual_address += PAGE_SIZE)
            {
                printk(KERN_INFO "virtual_address->%x\n", virtual_address);

                page_struct = get_current_page(mm, virtual_address);
                if (page_struct != NULL)
                {
                    lp->flags = page_struct->flags;
                    lp->virtual_address = virtual_address;
                    return 0;
                }
            }

            if (page_struct == NULL)
            {
                printk(KERN_ERR "Error while mapping page!\n");
                return 1;
            }
        }
    }
    return 0;
};
int get_net_device_info(struct lab_net_device *lnd)
{
    struct net_device *n_dev;
    read_lock(&dev_base_lock);
    n_dev = first_net_device(&init_net);
    if (!n_dev)
    {
        return -1;
    }
    int count;
    count = 0;
    while (n_dev)
    {
        lnd->number = count;
        strcpy(lnd->name, n_dev->name);
        lnd->state = n_dev->state;

        n_dev = next_net_device(n_dev);
        count++;
    }
    read_unlock(&dev_base_lock);
    return 0;
};