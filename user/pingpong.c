#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    char b[1];
    int pc[2], pp[2];

    pipe(pc);
    pipe(pp);

    if (fork() == 0) {
        close(pp[1]);
        close(pc[0]);
        if ((read(pp[0], b, 1)) < 0) {
            printf("pingpong: read failed\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        if ((write(pc[1], b, 1)) < 0) {
            printf("pingpong: write failed\n");
            exit(1);
        }
        exit(0);
    }
    else {
        close(pc[1]);
        close(pp[0]);
        if ((write(pp[1], b, 1)) < 0) {
            printf("pingpong: write failed\n");
            exit(1);
        }
        wait(0);
        if ((read(pc[0], b, 1)) < 0) {
            printf("pingpong: read failed\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());
        exit(0);
    }
}
