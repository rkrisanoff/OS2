#include "character_dev.h"
#include "lab_struct.h"
#include "output_struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

void ioctl_set_request(int fd, struct lab_request *lab_req)
{
    int ret_val = ioctl(fd, IOCTL_SET_INFO, lab_req);

    if (ret_val < 0)
    {
        printf("ioctl_set_net_device_info failed:%d\n", ret_val);
        exit(ret_val);
    }
}

void ioctl_get_response(int fd, struct lab_response *lab_res)
{
    int ret_val;
    ret_val = ioctl(fd, IOCTL_GET_INFO, lab_res);

    if (ret_val < 0)
    {
        printf("ioctl_get_msg failed: %d\n", ret_val);
        exit(ret_val);
    }
}

int main(int argc, char **argv)
{
    int fd;

    if (argc < 3 || (strcmp(argv[1], "help") == 0))
    {
        printf("Usage %s <PID> <PAGE-NUMBER>\n", argv[0]);
        printf("<PID> must be integer more than zero\n");
        printf("<PAGE-NUMBER> must be integer more than zero\n");

        return 0;
    }

    // request data declare
    struct lab_request *lab_req = malloc(sizeof(struct lab_request));
    lab_req->page_number = 0;
    lab_req->pid;
    lab_req->pid = atoi(argv[1]);
    lab_req->page_number = atoi(argv[2]);
    if (lab_req->pid)
    {
        printf("Entered <PID> is %d\n", lab_req->pid);
    }
    else
    {
        printf("Wrong <PID>\n");
        return 1;
    }
    
    if (lab_req->page_number)
    {
        printf("Entered <PAGE-NUMBER> is %d\n", lab_req->page_number);
    }
    else
    {
        printf("Wrong <PAGE-NUMBER>\n");
        return 2;
    }

    char output[1024];

    fd = open(DEVICE_NAME, 0);

    if (fd < 0)
    {
        printf("Can't open device file: %s\n", DEVICE_NAME);
        exit(-1);
    }

    struct lab_response *lab_res = malloc(sizeof(struct lab_response));

    ioctl_set_request(fd, lab_req);
    ioctl_get_response(fd, lab_res);
    close(fd);

    sprintf_net_device(&lab_res->lnd, output);
    sprintf_page(&lab_res->lp, output);
    sprintf_thread(&lab_res->lt, output);
    printf("%s", output);

    return 0;
}
