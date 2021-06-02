#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* getname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    /* find first character after last slash */
    for (p = path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    /* end name with null */
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    buf[strlen(p)] = 0;
    return buf;
}

void find(char *path, char *file)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    /* checkers */
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type) {
        case T_FILE:
            if (!strcmp(getname(path), file))
                printf("%s\n", path);
            break;
        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long\n");
                break;
            }
            /* append slash at path */
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
                    continue;
                /* call find with new path */
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                find(buf, file);
            }
            break;
        default:
            break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "usage: find [directory] [file]\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
