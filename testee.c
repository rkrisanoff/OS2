#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/sigaction.h>

// void chld_handler(int signum)
// {
//     printf("SIGCHLD signal is captured!\n");
// }
// void int_handler(int signum)
// {
//     printf("SIGINT signal is captured!\n");
// }


int main()
{
    // signal(SIGCHLD, chld_handler);
    // signal(SIGINT, int_handler);
    // sigprocmask(SIG_BLOCK, &chld_handler,
    //             NULL);
    sleep(1000);
    printf("Main function has finished\n");
}