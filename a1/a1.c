#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#pragma pack(1)

typedef struct LIST
{
    // optiuni
    bool list;
    bool parse;
    bool extract;
    bool findall;
    // pentru list
    bool recursiv;
    bool greater_size;
    bool name_start;
    int size;
    char *string;
    // pentru extract
    bool section;
    bool line;
    int section_nr;
    int line_nr;
    // path
    bool path;
    char *path_string;
} list;

typedef struct SECTION{
    char name[12];
    unsigned short type;
    unsigned int offset;
    unsigned int size;
}section;

typedef struct HEADER{
    unsigned char magic;
    unsigned short header_size;
    unsigned char version;
    unsigned char no_of_sections;
    section* sectiuni;
}header;

void print_arg(list *l)
{
    printf("list: %d, parse: %d, extract: %d, findall: %d, path:%d\n", l->list, l->parse, l->extract, l->findall, l->path);
    if (l->path)
    {
        printf("%s\n", l->path_string);
    }
    if (l->list)
    {
        printf("recurvive: %d, name_starts: %d, size_greater: %d\n", l->recursiv, l->name_start, l->greater_size);
        if (l->name_start)
        {
            printf("string: %s\n", l->string);
        }
        if (l->greater_size)
        {
            printf("size: %d\n", l->size);
        }
    }
    if (l->extract)
    {
        printf("section: %d, line: %d\n", l->section, l->line);
        if (l->section)
        {
            printf("section_nr: %d\n", l->section_nr);
        }
        if (l->line)
        {
            printf("line_nr: %d\n", l->line_nr);
        }
    }
}

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
            l->string = (char *)malloc(strlen(arg[i] + 16) * sizeof(char));
            sscanf(arg[i] + 17, "%s", l->string);
        }
        else if (strcmp(arg[i], "list") == 0)
        {
            l->list = true;
        }
        else if (strncmp(arg[i], "path=", 5) == 0)
        {
            l->path = true;
            l->path_string = (char *)malloc(strlen(arg[i] + 4) * sizeof(char));
            sscanf(arg[i] + 5, "%s", l->path_string);
        }
        else if (strcmp(arg[i], "parse") == 0)
        {
            l->parse = true;
        }
        else if (strcmp(arg[i], "extract") == 0)
        {
            l->extract = true;
        }
        else if (strcmp(arg[i], "findall") == 0)
        {
            l->findall = true;
        }
        else if (strncmp(arg[i], "section=", 8) == 0)
        {
            l->section = true;
            sscanf(arg[i] + 8, "%d", &l->section_nr);
        }
        else if (strncmp(arg[i], "line=", 5) == 0)
        {
            l->line = true;
            sscanf(arg[i] + 5, "%d", &l->line_nr);
        }
    }
    return l;
}

int listare(char *path, list *l, int ok)
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
                            if (statbuf.st_size > l->size && strncmp(entry->d_name, l->string, strlen(l->string)) == 0 && S_ISREG(statbuf.st_mode))
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
                            if (statbuf.st_size > l->size && S_ISREG(statbuf.st_mode))
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
                            if (statbuf.st_size > l->size && strncmp(entry->d_name, l->string, strlen(l->string)) == 0 && S_ISREG(statbuf.st_mode))
                            {
                                printf("%s\n", filePath);
                            }
                        }
                        else
                        {
                            if (statbuf.st_size > l->size && S_ISREG(statbuf.st_mode))
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

void parsare(list* l){
    int fd = open(l->path_string, O_RDONLY);
    if(fd == -1){
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    header* h = (header*)malloc(1* sizeof(header));
    lseek(fd, -3, SEEK_END);
    read(fd, &h->header_size, 2);
    read(fd, &h->magic, 1);
    if(h->magic != '0'){
        printf("ERROR\nwrong magic\n");
        free(h);
        return;
    }

    lseek(fd, -h->header_size, SEEK_END);
    read(fd, &h->version, 1);
    if(h->version < 72 || h->version > 156){
        printf("ERROR\nwrong version\n");
        free(h);
        return;
    }
    read(fd, &h->no_of_sections, 1);
    if(h->no_of_sections < 3 || h->no_of_sections > 19){
        printf("ERROR\nwrong sect_nr\n");
        free(h);
        return;
    }
    h->sectiuni = (section*)malloc(h->no_of_sections*sizeof(section));
    for(int i=0;i<h->no_of_sections;i++){
        read(fd, h->sectiuni[i].name, 11);
        read(fd, &h->sectiuni[i].type, 2);
        read(fd, &h->sectiuni[i].offset, 4);
        read(fd, &h->sectiuni[i].size, 4);
        if(h->sectiuni[i].type == 57 || h->sectiuni[i].type == 63 || h->sectiuni[i].type == 15 || h->sectiuni[i].type == 29){
            continue;
        }else{
            printf("ERROR\nwrong sect_types\n");
            free(h->sectiuni);
            free(h);
            return;
        }
    }
    printf("SUCCESS\nversion=%d\nnr_sections=%d\n", h->version, h->no_of_sections);
    for(int i=0;i<h->no_of_sections;i++){
        printf("section%d: %s %d %d\n", i+1, h->sectiuni[i].name, h->sectiuni[i].type, h->sectiuni[i].size);
    }
    free(h->sectiuni);
    free(h);
}


int main(int argc, char **argv)
{
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
            int ok;
            if (l->list)
            {
                if (l->path)
                {
                    ok = listare(l->path_string, l, 1);
                    if (ok != 0)
                    {
                        printf("ERROR\ninvalid directory path\n");
                    }
                }
                else
                {
                    printf("ERROR\ninvalid directory path\n");
                }
            }else if(l->parse){
                 parsare(l);
                 
            }
            free(l->path_string);
            free(l->string);
            free(l);
        }
        
    }
    return 0;
}