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
#define size_citire 300

enum erori_header
{
    MAGIC,
    VERSION,
    SECT_NR,
    TYPES,
    PATH,
};

enum optiuni
{
    VARIANT,
    LIST,
    PARSE,
    EXTRACT,
    FINDALL,
};
typedef struct LIST
{
    // optiuni
    int optiune;
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

typedef struct SECTION
{
    char name[12];
    unsigned short type;
    unsigned int offset;
    unsigned int size;
} section;

typedef struct HEADER
{
    unsigned char magic;
    unsigned short header_size;
    unsigned char version;
    unsigned char no_of_sections;
    section *sectiuni;
} header;

list *parsareArgumente(int n, char **arg)
{
    list *l = (list *)malloc(1 * sizeof(list));
    for (int i = 1; i < n; i++)
    {
        if (strcmp(arg[i], "variant") == 0)
        {
            l->optiune = VARIANT;
        }
        else if (strcmp(arg[i], "list") == 0)
        {
            l->optiune = LIST;
        }
        else if (strcmp(arg[i], "recursive") == 0)
        {
            l->recursiv = true;
        }
        else if (strncmp(arg[i], "name_starts_with=", 17) == 0)
        {
            l->name_start = true;
            l->string = (char *)malloc(strlen(arg[i] + 16) * sizeof(char));
            sscanf(arg[i] + 17, "%s", l->string);
        }
        else if (strncmp(arg[i], "size_greater=", 13) == 0)
        {
            l->greater_size = true;
            sscanf(arg[i] + 13, "%d", &l->size);
        }
        else if (strcmp(arg[i], "parse") == 0)
        {
            l->optiune = PARSE;
        }
        else if (strcmp(arg[i], "extract") == 0)
        {
            l->optiune = EXTRACT;
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
        else if (strcmp(arg[i], "findall") == 0)
        {
            l->optiune = FINDALL;
        }
        else if (strncmp(arg[i], "path=", 5) == 0)
        {
            l->path = true;
            l->path_string = (char *)malloc(strlen(arg[i] + 4) * sizeof(char));
            sscanf(arg[i] + 5, "%s", l->path_string);
        }
    }
    return l;
}

void listare(char *path, list *l, bool ok)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    dir = opendir(path);
    char filePath[512];
    if (dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    if (ok)
    {
        printf("SUCCESS\n");
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        sprintf(filePath, "%s/%s", path, entry->d_name);
        if (lstat(filePath, &statbuf) == 0)
        {
            if (l->recursiv)
            {
                if (S_ISDIR(statbuf.st_mode))
                {
                    listare(filePath, l, false);
                }
            }
            if (l->name_start && strncmp(entry->d_name, l->string, strlen(l->string)) != 0)
            {
                continue;
            }
            if (l->greater_size && S_ISREG(statbuf.st_mode) && statbuf.st_size <= l->size)
            {
                continue;
            }
            if (l->greater_size)
            {
                if (!S_ISDIR(statbuf.st_mode))
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
    closedir(dir);
}

header *parsare(char *path, int *eroare)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        *eroare = PATH;
        return NULL;
    }
    header *h = (header *)malloc(1 * sizeof(header));
    lseek(fd, -3, SEEK_END);
    read(fd, &h->header_size, 2);
    read(fd, &h->magic, 1);
    if (h->magic != '0')
    {
        *eroare = MAGIC;
        close(fd);
        free(h);
        return NULL;
    }
    lseek(fd, 0, SEEK_SET);
    int size = lseek(fd, 0, SEEK_END);
    if (h->header_size > size)
    {
        close(fd);
        free(h);
        return NULL;
    }
    lseek(fd, -(h->header_size), SEEK_END);
    read(fd, &h->version, 1);
    if (h->version < 72 || h->version > 156)
    {
        *eroare = VERSION;
        close(fd);
        free(h);
        return NULL;
    }
    read(fd, &h->no_of_sections, 1);
    if (h->no_of_sections < 3 || h->no_of_sections > 19)
    {
        *eroare = SECT_NR;
        close(fd);
        free(h);
        return NULL;
    }
    h->sectiuni = (section *)malloc(h->no_of_sections * sizeof(section));
    for (int i = 0; i < h->no_of_sections; i++)
    {
        read(fd, h->sectiuni[i].name, 11);
        read(fd, &h->sectiuni[i].type, 2);
        read(fd, &h->sectiuni[i].offset, 4);
        read(fd, &h->sectiuni[i].size, 4);
        if (h->sectiuni[i].type == 57 || h->sectiuni[i].type == 63 || h->sectiuni[i].type == 15 || h->sectiuni[i].type == 29)
        {
            continue;
        }
        else
        {
            *eroare = TYPES;
            close(fd);
            free(h->sectiuni);
            free(h);
            return NULL;
        }
    }
    close(fd);
    return h;
}

void print_header(header *h, int eroare)
{
    if (h == NULL)
    {
        switch (eroare)
        {
        case MAGIC:
        {
            printf("ERROR\nwrong magic\n");
            break;
        }
        case VERSION:
        {
            printf("ERROR\nwrong version\n");
            break;
        }
        case SECT_NR:
        {
            printf("ERROR\nwrong sect_nr\n");
            break;
        }
        case TYPES:
        {
            printf("ERROR\nwrong sect_types\n");
            break;
        }
        default:
            printf("ERROR\ninvalid directory path\n");
        }
    }
    else
    {
        printf("SUCCESS\nversion=%d\nnr_sections=%d\n", h->version, h->no_of_sections);
        for (int i = 0; i < h->no_of_sections; i++)
        {
            printf("section%d: %s %d %d\n", i + 1, h->sectiuni[i].name, h->sectiuni[i].type, h->sectiuni[i].size);
        }
        free(h->sectiuni);
        free(h);
    }
}

bool nr_line_of_section(char *path, header *h, int section)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("ERROR\ninvalid path\n");
        return false;
    }
    lseek(fd, h->sectiuni[section].offset, SEEK_SET);
    int nr_line = 1;
    int size = 0, size2;
    char a = 0, b;
    char *s;
    while ((h->sectiuni[section].size - size) != 0)
    {
        if (h->sectiuni[section].size - size > size_citire)
        {
            s = (char *)malloc(size_citire * sizeof(char));
            read(fd, s, size_citire);
            size += size_citire;
            size2 = size_citire;
        }
        else
        {
            s = (char *)malloc((h->sectiuni[section].size - size) * sizeof(char));
            read(fd, s, h->sectiuni[section].size - size);
            size2 = h->sectiuni[section].size - size;
            size += (h->sectiuni[section].size - size);
        }
        for (int i = 0; i < size2; i++)
        {
            b = s[i];
            if (a == '\r' && b == '\n')
            {
                nr_line++;
            }
            if (nr_line > 15)
            {
                close(fd);
                free(s);
                return false;
            }
            a = b;
        }
        free(s);
    }
    close(fd);
    if (nr_line == 15)
    {
        return true;
    }
    return false;
}

void extract(list *l)
{
    int n = 0;
    header *h = parsare(l->path_string, &n);
    if (h == NULL)
    {
        printf("ERROR\ninvalid file\n");
        return;
    }
    if (l->section_nr > h->no_of_sections || h->no_of_sections < 1)
    {
        printf("ERROR\ninvalid section\n");
        free(h->sectiuni);
        free(h);
        return;
    }
    int fd = open(l->path_string, O_RDONLY);
    lseek(fd, h->sectiuni[l->section_nr - 1].offset, SEEK_SET);
    int nr = 1;
    char a, b;
    int poz1 = 0, poz2 = 0;
    read(fd, &a, 1);
    if (l->line_nr == 1)
    {
        poz1 = h->sectiuni[l->section_nr - 1].offset;
    }
    for (unsigned int i = 1; i < h->sectiuni[l->section_nr - 1].size; i++)
    {
        read(fd, &b, 1);
        if (a == '\r' && b == '\n')
        {
            nr++;
        }
        if (nr == l->line_nr && poz1 == 0)
        {
            poz1 = lseek(fd, 0, SEEK_CUR);
        }
        if (nr == (l->line_nr + 1))
        {
            poz2 = lseek(fd, 0, SEEK_CUR) - 2;
            break;
        }
        a = b;
    }
    if (l->line_nr > nr)
    {
        printf("ERROR\ninvalid line\n");
        close(fd);
        free(h->sectiuni);
        free(h);
        return;
    }
    if (poz2 == 0)
    {
        poz2 = lseek(fd, 0, SEEK_CUR) - 1;
    }
    printf("SUCCESS\n");
    for (int i = poz2; i >= poz1; i--)
    {
        lseek(fd, i, SEEK_SET);
        read(fd, &a, 1);
        printf("%c", a);
    }
    printf("\n");
    close(fd);
    free(h->sectiuni);
    free(h);
}

void find_all(char *path, list *l, bool ok)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    dir = opendir(path);
    char filePath[512];
    if (dir == NULL)
    {
        printf("ERROR\ninvalid directory path\n");
        return;
    }
    if (ok)
    {
        printf("SUCCESS\n");
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        sprintf(filePath, "%s/%s", path, entry->d_name);
        if (lstat(filePath, &statbuf) == 0)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                find_all(filePath, l, false);
            }
            else
            {
                int n = 0;
                header *h = parsare(filePath, &n);
                if (h == NULL)
                {
                    continue;
                }
                int verif = 0;
                if (h->no_of_sections >= 2)
                {
                    for (int i = 0; i < h->no_of_sections; i++)
                    {
                        if (nr_line_of_section(filePath, h, i))
                        {
                            verif++;
                        }
                        if (verif == 2)
                        {
                            printf("%s\n", filePath);
                            break;
                        }
                    }
                }
                free(h->sectiuni);
                free(h);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        list *l = parsareArgumente(argc, argv);
        switch (l->optiune)
        {
        case VARIANT:
        {
            printf("91328\n");
            break;
        }
        case LIST:
        {
            listare(l->path_string, l, true);
            break;
        }
        case PARSE:
        {
            header *h;
            int eroare = 0;
            h = parsare(l->path_string, &eroare);
            print_header(h, eroare);
            break;
        }
        case EXTRACT:
        {
            extract(l);
            break;
        }
        case FINDALL:
        {
            find_all(l->path_string, l, true);
            break;
        }
        }
        free(l->path_string);
        free(l->string);
        free(l);
    }
    return 0;
}