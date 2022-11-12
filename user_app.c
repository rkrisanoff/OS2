
#include "chardev.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
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

void ioctl_set_request(int fd, struct lab_request *lab_req)
{
    int ret_val = ioctl(fd, IOCTL_SET_INFO, lab_req);

    if (ret_val < 0)
    {
        printf("ioctl_set_net_device_info failed:%d\n", ret_val);
        exit(ret_val);
    }
}

void ioctl_get_response(int fd,struct lab_response *lab_res)
{
    int ret_val;
    ret_val = ioctl(fd, IOCTL_GET_INFO, lab_res);

    if (ret_val < 0)
    {
        printf("ioctl_get_msg failed: %d\n", ret_val);
        exit(ret_val);
    }
}

int main()
{
    int fd;

    fd = open(DEVICE_FILE_NAME, 0);
    if (fd < 0)
    {
        printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        exit(-1);
    }

    struct lab_request  * lab_req = malloc(sizeof(struct lab_request));
    lab_req->pid = 1;
    struct lab_response * lab_res = malloc(sizeof(struct lab_response));

    ioctl_set_request(fd, lab_req);
    ioctl_get_response(fd,lab_res);
    close(fd);

    printf("number of device is %d\n", lab_res->lnd.number);
    printf("name is %s\n", lab_res->lnd.name);
    printf("state is %lu\n", lab_res->lnd.state);

    printf("flags of page: %x\n", lab_res->lp.flags);
    printf("virtual address is %lu\n", lab_res->lp.virtual_address);
    return 0;
}
