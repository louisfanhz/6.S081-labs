#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXLINE 1024

int main(int argc, char *argv[])
{
    int i, n;
    char line[MAXLINE], *s, *p, c;
    char *args[MAXARG];
    char *cmd;

    cmd = argv[1];
    for (i = 0; i < argc-1; i++) {
        args[i] = argv[i+1];
    }

    while ((n = read(0, line, MAXLINE)) > 0) {
        if (fork() == 0) {
            /* extract arguments */
            p = line;
            while (*p == ' ')   p++;    /* remove leading spaces */
            s = p;
            for (int k = 0; k < n; k++) {
                c = *p;
                /* if found ' ', place argument in args list */
                if (c == ' ' || c == '\n') {
                    *p = 0;
                    args[i] = (char *)malloc(strlen(s));
                    strcpy(args[i], s);
                    s = p+1;    /* s points to next argument */
                    i++;
                }
                if (i >= MAXARG) {
                    fprintf(2, "xargs: exceed arg limit\n");
                    exit(1);
                }
                p++;
            }
            exec(cmd, args);
            /* control should never reach here */
            fprintf(2, "xargs: call exec on %s failed\n", cmd);
            exit(1);
        }
        else {
            wait((int *)0);
        }
    }
    exit(0);
}

