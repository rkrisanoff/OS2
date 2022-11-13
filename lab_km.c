#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>

#include <linux/if_tun.h>
#include <linux/if_macvlan.h>
#include <linux/signal.h>
#include <linux/pagemap.h>
#include <linux/page-flags.h>
#include <linux/mm_types.h>
#include "linux/mm.h"
#include <asm/pgtable.h>
#include <asm/page.h>

#include <asm/uaccess.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <asm/siginfo.h>    //siginfo
#include <linux/rcupdate.h> //rcu_read_lock
#include <linux/sched.h>    //find_task_by_pid_type
// MODULE_LICENSE("GPL");
#include "chardev.h"

MODULE_VERSION("1.9.17");
MODULE_AUTHOR("Drukhary");
MODULE_DESCRIPTION("OS LAB2");

static int lab_dev_open(struct inode *inode, struct file *file);
static int lab_dev_release(struct inode *inode, struct file *file);
static ssize_t lab_dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t lab_dev_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static long lab_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct lab_request *lab_req;
static struct lab_response *lab_res;
static int get_multiprocess_signals_info(int pid, char *output)
{
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

    return 0;
}
static struct page *get_current_page(struct mm_struct *mm, long virtual_address)
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
}
static int get_page_info(int pid, struct lab_page *lp)
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
}
static int get_net_device_info(struct lab_net_device *lnd)
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
        lab_res->lnd.number = count;
        strcpy(lab_res->lnd.name, n_dev->name);
        lab_res->lnd.state = n_dev->state;

        n_dev = next_net_device(n_dev);
        count++;
    }
    read_unlock(&dev_base_lock);
    return 0;
}
/*
** This function will be called when we open the Device file
*/
static int lab_dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int lab_dev_release(struct inode *inode, struct file *file)
{

    pr_info("Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t lab_dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t lab_dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
static int device_ioctl(struct inode *inode,
                        struct file *file,
                        unsigned int ioctl_num,
                        unsigned long ioctl_param)
#else
static long lab_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
#endif
{
    printk(KERN_INFO "device_ioctl(%p,%lu,%lu)", file, ioctl_num, ioctl_param);
    unsigned long ret_val_ku;
    switch (ioctl_num)
    {

    case IOCTL_SET_INFO:
        printk(KERN_INFO "IOCTL_SET_INFO\n");
        if (lab_req == NULL)
        {
            lab_req = vmalloc(sizeof(struct lab_request));
        }

        ret_val_ku = copy_from_user(lab_req, (struct lab_request *)ioctl_param, sizeof(struct lab_request));
        if (ret_val_ku < 0)
        {
            printk(KERN_INFO "ERROR WITH COPUIING ");
        }

        if (lab_res != NULL)
        {
            vfree(lab_res);
            lab_res = NULL;
        }
        break;
    case IOCTL_GET_INFO:
        printk(KERN_INFO "IOCTL_GET_INFO\n");
        if (lab_res == NULL)
        {
            lab_res = vmalloc(sizeof(struct lab_response));
        }
        int ret_val = 0;
        ret_val = get_net_device_info(&lab_res->lnd);
        ret_val = get_page_info(lab_req->pid, &lab_res->lp);

        ret_val_ku = copy_to_user((struct lab_response *)ioctl_param, lab_res, sizeof(struct lab_response));

        if (lab_req != NULL)
        {
            vfree(lab_req);
            lab_req = NULL;
        }
        break;
    }
    return 0;
}

struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = lab_dev_read,
    .write = lab_dev_write,
    .open = lab_dev_open,
    .release = lab_dev_release,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
    .ioctl = device_ioctl
#else
    .unlocked_ioctl = lab_dev_ioctl
#endif
};

int init_module()
{
    int ret_val;
    ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &file_ops);
    if (ret_val < 0)
    {
        printk(KERN_ALERT "%s failed with %d\n", "Sorry, registering the character device ", ret_val);
        return ret_val;
    }

    printk(KERN_INFO "\n%s The major device number is %d.\n", "Registration is a success", MAJOR_NUM);
    printk(KERN_INFO "If you want to talk to the device driver,\n");
    printk(KERN_INFO "you'll have to create a device file. \n");
    printk(KERN_INFO "We suggest you use:\n");
    printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
    printk(KERN_INFO "The device file name is important, because\n");
    printk(KERN_INFO "the ioctl program assumes that's the\n");
    printk(KERN_INFO "file you'll use.\n");

    return 0;
}

void cleanup_module()
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}
