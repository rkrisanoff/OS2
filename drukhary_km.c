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
#include "chardev.h"

#include <asm/siginfo.h>    //siginfo
#include <linux/rcupdate.h> //rcu_read_lock
#include <linux/sched.h>    //find_task_by_pid_type
#define SUCCESS 0

static int is_device_open = 0;
static char message[BUFFER_SIZE];
static char *message_ptr;
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
        else
        {
            strcat(output, "Multiprocess's list is absolutely empty\n");
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
static int get_page_info(int pid, char *output)
{
    struct task_struct *t = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    char buff_int[20];
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
            long virtual_address;

            for (virtual_address = vas->vm_start; virtual_address <= vas->vm_end; virtual_address += PAGE_SIZE)
            {
                printk(KERN_INFO "virtual_address->%x\n", virtual_address);

                page_struct = get_current_page(mm, virtual_address);
                if (page_struct != NULL)
                {
                    strcat(output, "page_struct->flags ");
                    sprintf(buff_int, "%x", page_struct->flags);
                    strcat(output, buff_int);
                    strcat(output, "\n");
                    strcat(output, "virtual_address ");
                    sprintf(buff_int, "%x", virtual_address);
                    strcat(output, buff_int);
                    strcat(output, "\n");
                    strcat(output, "page_struct->mapping ");
                    sprintf(buff_int, "%x", page_struct->mapping);
                    strcat(output, buff_int);
                    strcat(output, "\n");
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

static int device_open(struct inode *inode, struct file *file)
{
    // logs
    printk(KERN_INFO "device_open(%p)\n", file);

    if (is_device_open)
    {
        return -EBUSY;
    }
    // notice the device is open
    is_device_open = 1;
    // zeroing out message_ptr
    message_ptr = message;
    // Increment the reference count of current module
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    // logs
    printk(KERN_INFO "device_release(%p,%p)\n", inode, file);
    // notice the device is close
    is_device_open = 0;
    // Decrement the reference count of current module
    module_put(THIS_MODULE);
    return SUCCESS;
}

static ssize_t device_read(
    struct file *file,
    char __user *buffer,
    size_t length,
    loff_t *offset)
{
    int bytes_read = 0;

    // logs
    printk(KERN_INFO "device_read(%p,%p,%lu)\n", file, buffer, length);
    if ((*message_ptr) == 0)
    {
        return 0;
    }
    while (length && (*message_ptr))
    {

        // copy char from kernelland to userland
        put_user(*(message_ptr), buffer);
        message_ptr++;
        buffer++;
        length--;
        bytes_read++;
    }

    printk(KERN_INFO "Read %d bytes, %lu left\n", bytes_read, length);
    return bytes_read;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    int i;
    printk(KERN_INFO "device_write(%p,%s,%lu)", file, buffer, length);
    for (i = 0; (i < length) && (i < BUFFER_SIZE); i++)
    {
        // copy char from userland to kernelland
        get_user(message[i], buffer + i);
        printk(KERN_INFO "device_write(temp_char=%c,i=%d)", message[i], i);
    }

    message_ptr = message;
    return i;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
static int device_ioctl(struct inode *inode,
                        struct file *file,
                        unsigned int ioctl_num,
                        unsigned long ioctl_param)
#else
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
#endif
{
    printk(KERN_INFO "device_ioctl(%p,%lu,%lu)", file, ioctl_num, ioctl_param);
    int i;
    char *ch_user_space_ptr;
    char ch;
    switch (ioctl_num)
    {
    case IOCTL_SET_MSG:
        ch_user_space_ptr = (char *)ioctl_param;
        get_user(ch, ch_user_space_ptr);
        for (i = 0; ch && i < BUFFER_SIZE; i++, ch_user_space_ptr++)
        {
            get_user(ch, ch_user_space_ptr);
        }

        device_write(file, (char *)ioctl_param, i, 0);
        break;

    case IOCTL_GET_MSG:
        char *output = vmalloc(sizeof(char) * BUFFER_SIZE);
        char *temporary = vmalloc(sizeof(char) * BUFFER_SIZE);
        char *command = vmalloc(sizeof(char) * BUFFER_SIZE);
        char *separator = vmalloc(sizeof(char) * BUFFER_SIZE);
        int pid;
        printk(KERN_INFO "Received msg from user_space: %s\n", message);
        sprintf(output, "%s", "");
        sprintf(temporary, "%s", message);
        strcpy(temporary, message);

        separator = strchr(temporary, ' ');
        if (separator == NULL)
        {
            command = temporary;
            pid = -1;
        }
        else
        {
            command = strsep(&temporary, " ");
            temporary = strsep(&temporary, " ");
            if (temporary != NULL)
            {
                int res_val = kstrtoint(temporary, 10, &pid);
                printk(KERN_INFO "received number: %d\n", res_val);
                printk(KERN_INFO "received number: %d\n", pid);
            }
            else
            {
                printk(KERN_INFO "Silly number: %s\n", temporary);
            }
        }

        if (strcmp(command, "net_device") == 0)
        {
            struct net_device *n_dev;
            read_lock(&dev_base_lock);
            n_dev = first_net_device(&init_net);
            if (!n_dev)
            {
                strcat(output, "No network devices ;(((");
            }
            int c;
            c = 0;
            char buff_int[10];
            while (n_dev)
            {
                sprintf(buff_int, "%d", c);
                strcat(output, "Number of net device: ");
                strcat(output, buff_int);
                strcat(output, "\n");
                sprintf(buff_int, "%s", n_dev->name);
                strcat(output, buff_int);
                strcat(output, "\n");
                strcat(output, "State: ");
                sprintf(buff_int, "%lu", n_dev->state);
                strcat(output, buff_int);
                n_dev = next_net_device(n_dev);
                c++;
                strcat(output, "\n\n");
            }
            strcat(output, "Total number of ");
            sprintf(buff_int, "%d", c);
            strcat(output, buff_int);
            read_unlock(&dev_base_lock);
        }
        if (strcmp(command, "pt_regs") == 0)
        {
            struct pt_regs *regs = task_pt_regs(current);
            char buff_int[10];
            strcat(output, "REGS: \n");
            strcat(output, "r10 ");
            sprintf(buff_int, "%lu", regs->r10);
            strcat(output, buff_int);
            strcat(output, "\nsp ");
            sprintf(buff_int, "%lu", regs->sp);
            strcat(output, buff_int);
        }
        if (strcmp(command, "ms") == 0)
        {
            int result = get_multiprocess_signals_info(pid, output);
            if (result)
            {
                return 1;
            }
        }
        if (strcmp(command, "pg") == 0)
        {
            int result = get_page_info(pid, output);
            if (result)
            {
                return 1;
            }
        }
        if (strcmp(output, "") == 0)
        {
            sprintf(output, "%s", "Wrong input");
        }
        strcat(output, "\n");

        printk(KERN_ALERT "output: %s", output);
        // zeroing up the message
        sprintf(message, "%s", "");

        for (i = 0; i < strlen(output) && i < BUFFER_SIZE; ++i)
        {
            message[i] = output[i];
        }

        vfree(output);
        vfree(temporary);
        vfree(command);
        vfree(separator);
        message_ptr = message;

        i = device_read(file, (char *)ioctl_param, i, 0);

        put_user('\0', (char *)ioctl_param + i);

        break;

    case IOCTL_GET_NTH_BYTE:

        return message[ioctl_param];
        break;
    }
    return SUCCESS;
}

struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35))
    .ioctl = device_ioctl
#else
    .unlocked_ioctl = device_ioctl
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

MODULE_VERSION("1.9.17");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Drukhary");
MODULE_DESCRIPTION("OS LAB2");
void cleanup_module()
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}
