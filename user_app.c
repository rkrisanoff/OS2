
#include "chardev.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>        
#include <unistd.h>       
#include <sys/ioctl.h>        

void ioctl_set_msg(int fd, char *message) {
    int ret_val = ioctl(fd, IOCTL_SET_MSG, message);

    if (ret_val < 0) {
        printf("ioctl_set_msg failed:%d\n", ret_val);
        exit(ret_val);
    }
}

void ioctl_get_msg(int fd) {
    int ret_val;
    char message[BUFFER_SIZE];

    ret_val = ioctl(fd, IOCTL_GET_MSG, message);

    if (ret_val < 0) {
        printf("ioctl_get_msg failed: %d\n", ret_val);
        exit(ret_val);
    }
	printf("%s", message);
}

// void ioctl_get_nth_byte(int file_desc) {
//     int i;
//     char c;

//     printf("get_nth_byte message:");

//     i = 0;
//     do {
//         c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);

//         if (c < 0) {
//             printf
//                     ("ioctl_get_nth_byte failed at the %d'th byte:\n",
//                      i);
//             exit(-1);
//         }

//         putchar(c);
//     } while (c != 0);
//     putchar('\n');
// }


int main() {
    int fd;
    char msg[BUFFER_SIZE];

    fd = open(DEVICE_FILE_NAME, 0);
    if (fd < 0) {
        printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        exit(-1);
    }

    gets(msg);

    ioctl_set_msg(fd, msg);
    ioctl_get_msg(fd);

    close(fd);
    return 0;
}
