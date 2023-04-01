#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct LIST
{
    bool recursiv;
    bool greater_size;
    bool name_start;
    bool list;
    bool path;
    char path_string[50];
    int size;
    char string[20];
} list;

list *parsareArgumente(int n, char **arg)
{
    list *l = (list *)malloc(1 * sizeof(list));
    for (int i = 1; i < n; i++)
    {
        if (strcmp(arg[i], "recursive") == 0)
        {
            l->recursiv = true;
        }
        else if (strncmp(arg[i], "size_greater=", 13) == 0)
        {
            l->greater_size = true;
            sscanf(arg[i] + 13, "%d", &l->size);
        }
        else if (strncmp(arg[i], "name_starts_with=", 17) == 0)
        {
            l->name_start = true;
            sscanf(arg[i] + 17, "%s", l->string);
        }
        else if (strcmp(arg[i], "list") == 0)
        {
            l->list = true;
        }
        else if (strncmp(arg[i], "path=", 5) == 0)
        {
            l->path = true;
            sscanf(arg[i] + 5, "%s", l->path_string);
        }
    }
    return l;
}

int listare(char *path, list* l, int ok)
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
                if (l->recursiv) // daca e recursiv
                {
                    if (l->greater_size)
                    {
                        if (l->name_start)
                        {
                            if (statbuf.st_size > l->size && strncmp(entry->d_name, l->string, strlen(l->string)) == 0)
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
                            if (statbuf.st_size > l->size)
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
                        if (l->name_start)
                        {
                            if (strncmp(entry->d_name, l->string, strlen(l->string)) == 0)
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
                    if (l->greater_size)
                    {
                        if (l->name_start)
                        {
                            if (statbuf.st_size > l->size && strncmp(entry->d_name, l->string, strlen(l->string)) == 0)
                            {
                                printf("%s\n", filePath);
                            }
                        }
                        else
                        {
                            if (statbuf.st_size > l->size)
                            {
                                printf("%s\n", filePath);
                            }
                        }
                    }
                    else
                    {
                        if (l->name_start)
                        {
                            if (strncmp(entry->d_name, l->string, strlen(l->string)) == 0)
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
        else
        {
            list *l = parsareArgumente(argc, argv);
            // printf("list: %d, path:%d\n", l->list, l->path);
            // if(l->path){
            //     printf("%s\n", l->path_string);
            // }
            // printf("recurvive: %d, name_starts: %d, size_greater: %d\n", l->recursiv, l->name_start, l->greater_size);
            // if(l->name_start){
            //     printf("string: %s\n", l->string);
            // }
            // if(l->greater_size){
            //     printf("size: %d\n", l->size);
            // }
            if(l->list){
                if(l->path){
                    ok = listare(l->path_string, l, 1);
                    if(ok != 0){
                        printf("ERROR\ninvalid directory path\n");
                    }
                }else{
                    printf("ERROR\ninvalid directory path\n");
                }
            }
            free(l);
        }
    }
    return 0;
}