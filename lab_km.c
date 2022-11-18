
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/version.h>
#include <linux/fs.h>

#include "character_dev.h"
#include "lab_struct.h"
#include "lkmfs.h"

MODULE_LICENSE("GPL");
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
/*
** This function will be called when we open the Device file
*/
static int lab_dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}
/*
** This function will be called when we close the Device file
*/
static int lab_dev_release(struct inode *inode, struct file *file)
{

    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}
/*
** This function will be called when we read the Device file
*/
static ssize_t lab_dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Read Function\n");
    return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t lab_dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Write function\n");
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
    printk(KERN_INFO "lab_dev_ioctl(%p,%lu,%lu)", file, ioctl_num, ioctl_param);
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

        ret_val = get_net_device_struct_info(&lab_res->lnd);
        ret_val = get_page_struct_info(&lab_res->lp, lab_req->pid, lab_req->page_number);
        ret_val = get_thread_struct_info(&lab_res->lt, lab_req->pid);
        ret_val = get_multiprocess_signals_info(lab_req->pid);

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
        printk(KERN_ALERT "%s failed with %d\n", "Sorry, registering the character device \n", ret_val);
        return ret_val;
    }

    printk(KERN_INFO "\nRegistration is a success The major device number is %d.\n", MAJOR_NUM);
    printk(KERN_INFO "If you want to talk to the device driver,\n");
    printk(KERN_INFO "you'll have to create a device file. \n");
    printk(KERN_INFO "I suggest you use:\n");
    printk(KERN_INFO "mknod %s c %d <MINOR-NUM>\n", DEVICE_NAME, MAJOR_NUM);
    printk(KERN_INFO "The device file name is important, because\n");
    printk(KERN_INFO "the ioctl program assumes that's the\n");
    printk(KERN_INFO "file you'll use.\n");

    return 0;
}

void cleanup_module()
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}