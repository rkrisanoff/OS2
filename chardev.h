/*
 *  chardev.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in chardev.c) and the process calling ioctl (ioctl.c)
 */

#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>

/*
 * Memblock structure var in order to define how many memory regions has every array
 * */
// #define INIT_MEMBLOCK_REGIONS 128

/*
 * The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it.
 */
#define MAJOR_NUM 100

/*
 * Set the message of the device driver
 */
#define IOCTL_SET_INFO _IOR(MAJOR_NUM, 0, char *)
/*
 * _IOR means that we're creating an ioctl command
 * number for passing information from a user process
 * to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device
 * number we're using.
 *
 * The second argument is the number of the command
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from
 * the process to the kernel.
 */

/*
 * Get the message of the device driver
 */
#define IOCTL_GET_INFO _IOR(MAJOR_NUM, 1, char *)
/*
 * This IOCTL is used for output, to get the message
 * of the device driver. However, we still need the
 * buffer to place the message in to be input,
 * as it is allocated by the process.
 */

/*
 * The name of the device file
 */
#define DEVICE_FILE_NAME "lab_character_device"
#define DEVICE_NAME "lab_character_device"
#define BUFFER_SIZE 256

/**
 * lab network device struct
 * */
struct lab_net_device
{
    int number;
    char name[16];
    unsigned long state;
};
/**
 * lab page struct
 * */
struct lab_page
{
    unsigned long flags;
    unsigned long virtual_address;
};
/**
 * request we sent to device
 * */
struct lab_request
{
    int pid;
};
/**
 * request we get from device
 * */
struct lab_response
{
    struct lab_net_device lnd;
    struct lab_page lp;
};

#endif