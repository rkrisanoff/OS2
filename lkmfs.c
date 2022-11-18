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
        return 1;
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
int get_page_struct_info(struct lab_page *lp, int pid, int page_number)
{
    struct task_struct *t = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    printk(KERN_INFO "task_struct->%p\n", t);

    if (t == NULL)
    {
        printk(KERN_ERR "Task with <PID> = %d! doesn't exit!\n", pid);
        return 1;
    }
    else
    {
        struct page *page_struct;
        struct mm_struct *mm = t->mm;
        if (mm == NULL)
        {
            printk(KERN_ERR "Memoty desctiptor of process with <PID> = %d! doesn't exit!\n", pid);
            return 2;
        }
            
            struct vm_area_struct *vas = mm->mmap;
            unsigned long virtual_address;
            int page_counter = 1;

            for (virtual_address = vas->vm_start, page_counter = 1;
                 virtual_address <= vas->vm_end;
                 virtual_address += PAGE_SIZE, page_counter++)

            {
                printk(KERN_INFO "virtual_address->%x\n", virtual_address);

                page_struct = get_current_page(mm, virtual_address);
                if (page_number == page_counter && page_struct != NULL)
                {
                    lp->flags = page_struct->flags;
                    lp->virtual_address = virtual_address;
                    lp->page_index = page_struct->index;
                    lp->page_type = page_struct->page_type;
                    return 0;
                }
            }

            if (page_struct == NULL)
            {
                printk(KERN_ERR "Error while mapping page!\n");
                return 3;
            }
    }
    return 0;
};
int get_net_device_struct_info(struct lab_net_device *lnd)
{
    struct net_device *n_dev;
    read_lock(&dev_base_lock);
    n_dev = first_net_device(&init_net);
    if (!n_dev)
    {
        printk(KERN_ERR "There are not a net_device at all!\n");
        return 1;
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

int get_thread_struct_info(struct lab_thread *lts, int pid)
{

    struct task_struct *t = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (t == NULL)
    {
        printk(KERN_ERR "task_struct with pid=%d does not exist\n", pid);
        return 1;
    }
    struct thread_struct th = t->thread;

    lts->sp = th.sp;
    // printk(KERN_INFO "th.cr2->%lu\n", th.cr2);
    // printk(KERN_INFO "th.ds->%u\n", th.ds);
    // printk(KERN_INFO "th.error_code->%lu\n", th.error_code);
    // printk(KERN_INFO "th.es->%u\n", th.es);
    // printk(KERN_INFO "th.fs->%lu\n", th.pkru);
    // printk(KERN_INFO "th.io_bitmap->%x\n", th.io_bitmap);
    // printk(KERN_INFO "th.iopl_emul->%lu\n", th.iopl_emul);
    // printk(KERN_INFO "th.iopl_warn->%u\n", th.iopl_warn);
    // printk(KERN_INFO "th.ptrace_dr7->%lu\n", th.ptrace_dr7);
    // printk(KERN_INFO "th.sig_on_uaccess_err->%u\n", th.sig_on_uaccess_err);
    // printk(KERN_INFO "th.sp->%lu\n", th.sp);
    // printk(KERN_INFO "th.trap_nr->%lu\n", th.trap_nr);
    // printk(KERN_INFO "th.virtual_dr6->%lu\n", th.virtual_dr6);
    return 0;
}