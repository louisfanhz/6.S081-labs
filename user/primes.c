#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N 35

int main(int argc, char *argv[])
{
    int numbers[N+1], p[2];     /* remaining numbers and pipe */
    int index = 0;              /* remaining number index */
    int pid, prime, temp;

    /* initialize the numbers array to hold 2-35 */
    for (int i = 2; i < N+1; i++) {
        numbers[index++] = i;
    }

    while (index > 0) {
        pipe(p);

        /* create child to deal with each sequence */
        if ((pid = fork()) < 0) {
            fprintf(2, "fork error\n");
            exit(1);
        }
        else if (pid == 0) {
            close(p[1]);
            index = 0;
            /* read and print out prime number */
            read(p[0], (void *)(&prime), sizeof(int));
            printf("prime %d\n", prime);

            /* update numbers array */
            while (read(p[0], (void *)(&temp), sizeof(int))) {
                if (temp % prime)
                    numbers[index++] = temp;
            }
            close(p[0]);
        }
        else {
            close(p[0]);
            /* parent write to the pipe the current numbers in number array */
            for (int i = 0; i < index; i++) {
                write(p[1], &numbers[i], sizeof(int));
            }
            close(p[1]);
            wait(0);
            exit(0);
        }
    }
    exit(0);
}
