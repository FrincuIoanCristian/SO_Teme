#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct LIST
{
    bool recursiv;
    bool greater_size;
    bool name_start;
    int size;
    char string[20];
} list;

int listare(char *path, list l, int ok)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    dir = opendir(path);
    char filePath[512];
    if (dir == NULL)
    {
        return -1;
    }
    if (ok == 1)
    {
        printf("SUCCESS\n");
        ok++;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            sprintf(filePath, "%s/%s", path, entry->d_name);
            if (lstat(filePath, &statbuf) == 0)
            {
                if (l.recursiv) // daca e recursiv
                {
                    if (l.greater_size)
                    {
                        if (l.name_start)
                        {
                            if (statbuf.st_size > l.size && strncmp(entry->d_name, l.string, strlen(l.string)) == 0)
                            {
                                printf("%s\n", filePath);
                            }
                            if (S_ISDIR(statbuf.st_mode))
                            {
                                listare(filePath, l, ok);
                            }
                        }
                        else
                        {
                            if (statbuf.st_size > l.size)
                            {
                                printf("%s\n", filePath);
                            }
                            if (S_ISDIR(statbuf.st_mode))
                            {
                                listare(filePath, l, ok);
                            }
                        }
                    }
                    else
                    {
                        if (l.name_start)
                        {
                            if (strncmp(entry->d_name, l.string, strlen(l.string)) == 0)
                            {
                                printf("%s\n", filePath);
                            }
                            if (S_ISDIR(statbuf.st_mode))
                            {
                                listare(filePath, l, ok);
                            }
                        }
                        else
                        {
                            printf("%s\n", filePath);
                            if (S_ISDIR(statbuf.st_mode))
                            {
                                listare(filePath, l, ok);
                            }
                        }
                    }
                }
                else
                {
                    if (l.greater_size)
                    {
                        if (l.name_start)
                        {
                            if (statbuf.st_size > l.size && strncmp(entry->d_name, l.string, strlen(l.string)) == 0)
                            {
                                printf("%s\n", filePath);
                            }
                        }
                        else
                        {
                            if (statbuf.st_size > l.size)
                            {
                                printf("%s\n", filePath);
                            }
                        }
                    }
                    else
                    {
                        if (l.name_start)
                        {
                            if (strncmp(entry->d_name, l.string, strlen(l.string)) == 0)
                            {
                                printf("%s\n", filePath);
                            }
                        }
                        else
                        {
                            printf("%s\n", filePath);
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char **argv)
{
    int ok;
    if (argc >= 2)
    {
        // variant
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("91328\n");
        }
        // list
        if (strcmp(argv[1], "list") == 0 && strncmp(argv[argc - 1], "path=", 5) == 0)
        {
            list l;
            for (int i = 2; i < argc - 1; i++)
            {
                if (strcmp(argv[i], "recursive") == 0)
                {
                    l.recursiv = true;
                }
                if (strncmp(argv[i], "size_greater=", 13) == 0)
                {
                    l.greater_size = true;
                    sscanf(argv[i] + 13, "%d", &l.size);
                }
                if (strncmp(argv[i], "name_starts_with=", 17) == 0)
                {
                    l.name_start = true;
                    sscanf(argv[i] + 17, "%s", l.string);
                }
            }
            ok = listare(argv[argc - 1] + 5, l, 1);
            if (ok == -1)
            {
                printf("ERROR\ninvalid directory path\n");
            }
        }
    }
    return 0;
}