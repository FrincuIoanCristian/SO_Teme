#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR_AND_LENGHT(X) X, strlen(X)
#define ALINIAMENT 4096

enum operatori
{
    ECHO,
    CREATE_SHM,
    WRITE_TO_SHM,
    MAP__FILE,
    READ_FROM_FILE_OFFSET,
    READ_FROM_FILE_SECTION,
    READ_FROM_LOGICAL_SPACE_OFFSET,
    EXIT
};

typedef struct SECTION
{
    char name[12];
    unsigned short type;
    unsigned int offset;
    unsigned int size;
    unsigned int logical_offset;
} section;

typedef struct HEADER
{
    unsigned char magic;
    unsigned short header_size;
    unsigned char version;
    unsigned char no_of_sections;
    section *sectiuni;
} header;

void request_select(char *request, int *caz)
{
    if (strncmp(request, "ECHO", sizeof("ECHO")) == 0)
    {
        *caz = ECHO;
        return;
    }
    if (strncmp(request, "CREATE_SHM", sizeof("CREATE_SHM")) == 0)
    {
        *caz = CREATE_SHM;
        return;
    }
    if (strncmp(request, "WRITE_TO_SHM", sizeof("WRITE_TO_SHM")) == 0)
    {
        *caz = WRITE_TO_SHM;
        return;
    }
    if (strncmp(request, "MAP_FILE", sizeof("MAP_FILE")) == 0)
    {
        *caz = MAP__FILE;
        return;
    }
    if (strncmp(request, "READ_FROM_FILE_OFFSET", sizeof("READ_FROM_FILE_OFFSET")) == 0)
    {
        *caz = READ_FROM_FILE_OFFSET;
        return;
    }
    if (strncmp(request, "READ_FROM_FILE_SECTION", sizeof("READ_FROM_FILE_SECTION")) == 0)
    {
        *caz = READ_FROM_FILE_SECTION;
        return;
    }
    if (strncmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET", sizeof("READ_FROM_LOGICAL_SPACE_OFFSET")) == 0)
    {
        *caz = READ_FROM_LOGICAL_SPACE_OFFSET;
        return;
    }
    if (strncmp(request, "EXIT", sizeof("EXIT")) == 0)
    {
        *caz = EXIT;
        return;
    }
}

header *parsare(char *data, int size)
{
    header *h = (header *)malloc(1 * sizeof(header));
    h->magic = data[size - 1];
    unsigned short *header_size = (unsigned short *)(data + size - 3);
    h->header_size = *(header_size);
    if (h->magic != '0')
    {
        free(h);
        return NULL;
    }
    if (h->header_size > size)
    {
        free(h);
        return NULL;
    }
    int index = size - h->header_size;
    h->version = data[index++];
    if (h->version < 72 || h->version > 156)
    {
        free(h);
        return NULL;
    }
    h->no_of_sections = data[index++];
    if (h->no_of_sections < 3 || h->no_of_sections > 19)
    {
        free(h);
        return NULL;
    }
    h->sectiuni = (section *)malloc(h->no_of_sections * sizeof(section));
    unsigned int logical_off = 0;
    for (int i = 0; i < h->no_of_sections; i++)
    {
        strncpy(h->sectiuni[i].name, data + index, 11);
        index += 11;
        h->sectiuni[i].type = *(unsigned short *)(data + index);
        index += 2;
        h->sectiuni[i].offset = *(unsigned int *)(data + index);
        index += 4;
        h->sectiuni[i].size = *(unsigned int *)(data + index);
        index += 4;
        h->sectiuni[i].logical_offset = logical_off;
        unsigned int sz = h->sectiuni[i].size;
        logical_off += ALINIAMENT;
        while (sz > ALINIAMENT)
        {
            logical_off += ALINIAMENT;
            sz -= ALINIAMENT;
        }
        if (h->sectiuni[i].type == 57 || h->sectiuni[i].type == 63 || h->sectiuni[i].type == 15 || h->sectiuni[i].type == 29)
        {
            continue;
        }
        else
        {
            free(h->sectiuni);
            free(h);
            return NULL;
        }
    }
    return h;
}

int main()
{
    unlink("RESP_PIPE_91328");

    if (mkfifo("RESP_PIPE_91328", 0600) != 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }
    int fd1, fd2, fd;
    fd1 = open("REQ_PIPE_91328", O_RDONLY);
    if (fd1 == -1)
    {
        printf("ERROR\ncannot open the request pipe\n");
        return 1;
    }
    fd2 = open("RESP_PIPE_91328", O_WRONLY);

    write(fd2, STR_AND_LENGHT("CONNECT$"));
    printf("SUCCESS\n");

    int shmFD;
    volatile char *sharedMem = NULL;
    char *data = NULL;
    shm_unlink("/mGQYlA");
    unsigned int size, size_data;
    char file[256];
    while (true)
    {
        char *request_string = (char *)malloc(50 * sizeof(char));
        char c;
        read(fd1, &c, 1);
        int i = 0;
        while (c != '$')
        {
            request_string[i++] = c;
            read(fd1, &c, 1);
        }
        request_string[i] = '\0';
        int request;
        request_select(request_string, &request);
        printf("%s\n", request_string);
        switch (request)
        {
        case ECHO:
        {
            write(fd2, STR_AND_LENGHT("ECHO$VARIANT$"));
            unsigned int variant = 91328;
            write(fd2, &variant, 4);
            break;
        }
        case CREATE_SHM:
        {
            read(fd1, &size, 4);
            printf("size=%d\n", size);
            shmFD = shm_open("/mGQYlA", O_CREAT | O_RDWR, 0664);
            if (shmFD < 0)
            {
                write(fd2, STR_AND_LENGHT("CREATE_SHM$ERROR$"));
                break;
            }
            ftruncate(shmFD, size);
            sharedMem = (volatile char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
            if (sharedMem == (void *)-1)
            {
                write(fd2, STR_AND_LENGHT("CREATE_SHM$ERROR$"));
                break;
            }
            write(fd2, STR_AND_LENGHT("CREATE_SHM$SUCCESS$"));
            break;
        }
        case WRITE_TO_SHM:
        {
            unsigned int offset, value;
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &value, sizeof(unsigned int));
            printf("offset=%d, value=%d\n", offset, value);
            if (offset < 0 || offset + sizeof(unsigned int) > size)
            {
                write(fd2, STR_AND_LENGHT("WRITE_TO_SHM$ERROR$"));
                break;
            }
            *((unsigned int *)(sharedMem + offset)) = value;
            write(fd2, STR_AND_LENGHT("WRITE_TO_SHM$SUCCESS$"));
            break;
        }
        case MAP__FILE:
        {
            i = 0;
            while (true)
            {
                read(fd1, &c, 1);
                if (c == '$')
                {
                    break;
                }
                file[i++] = c;
            }
            file[i] = '\0';
            printf("file=%s\n", file);
            fd = open(file, O_RDONLY);
            if (fd == -1)
            {
                write(fd2, STR_AND_LENGHT("MAP_FILE$ERROR$"));
                break;
            }
            size_data = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            data = (char *)mmap(NULL, size_data, PROT_READ, MAP_PRIVATE, fd, 0);
            if (data == (void *)-1)
            {
                write(fd2, STR_AND_LENGHT("MAP_FILE$ERROR$"));
                break;
            }
            write(fd2, STR_AND_LENGHT("MAP_FILE$SUCCESS$"));
            break;
        }
        case READ_FROM_FILE_OFFSET:
        {
            unsigned int offset, no_of_bytes;
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &no_of_bytes, sizeof(unsigned int));
            printf("offset=%d, no_of_bytes=%d\n", offset, no_of_bytes);
            if (offset < 0 || offset + no_of_bytes > size_data || sharedMem == (void *)-1 || data == (void *)-1)
            {
                write(fd2, STR_AND_LENGHT("READ_FROM_FILE_OFFSET$ERROR$"));
                break;
            }
            strncpy((char *)sharedMem, data + offset, no_of_bytes);
            write(fd2, STR_AND_LENGHT("READ_FROM_FILE_OFFSET$SUCCESS$"));
            break;
        }
        case READ_FROM_FILE_SECTION:
        {
            unsigned int section_no, offset, no_of_bytes;
            read(fd1, &section_no, sizeof(unsigned int));
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &no_of_bytes, sizeof(unsigned int));
            printf("section_no=%d, offset=%d, no_of_bytes=%d\n", section_no, offset, no_of_bytes);
            header *h = parsare(data, size_data);

            if (h == NULL)
            {
                write(fd2, STR_AND_LENGHT("READ_FROM_FILE_SECTION$ERROR$"));
                break;
            }
            for (int i = 0; i < h->no_of_sections; i++)
            {
                printf("Sectionea:%d, size:%d\n", i + 1, h->sectiuni[i].size);
            }
            if (section_no > h->no_of_sections || offset > h->sectiuni[section_no - 1].size || no_of_bytes > h->sectiuni[section_no - 1].size - offset)
            {
                write(fd2, STR_AND_LENGHT("READ_FROM_FILE_SECTION$ERROR$"));
                free(h->sectiuni);
                free(h);
                break;
            }
            strncpy((char *)sharedMem, data + h->sectiuni[section_no - 1].offset + offset, no_of_bytes);
            write(fd2, STR_AND_LENGHT("READ_FROM_FILE_SECTION$SUCCESS$"));
            free(h->sectiuni);
            free(h);
            break;
        }
        case READ_FROM_LOGICAL_SPACE_OFFSET:
        {
            unsigned int logical_offset, no_of_bytes;
            read(fd1, &logical_offset, sizeof(unsigned int));
            read(fd1, &no_of_bytes, sizeof(unsigned int));
            printf("logical_offset=%d, no_of_bytes=%d\n", logical_offset, no_of_bytes);
            header *h = parsare(data, size_data);
            int section;
            for (section = 0; section < h->no_of_sections; section++)
            {
                if (h->sectiuni[section].logical_offset > logical_offset)
                {
                    break;
                }
            }
            section--;
            if (h->sectiuni[section].size < (logical_offset - h->sectiuni[section].logical_offset) + no_of_bytes || logical_offset > h->sectiuni[h->no_of_sections-1].logical_offset + h->sectiuni[h->no_of_sections-1].size)
            {
                write(fd2, STR_AND_LENGHT("READ_FROM_LOGICAL_SPACE_OFFSET$ERROR$"));
                free(h->sectiuni);
                free(h);
                break;
            }
            strncpy((char *)sharedMem, data + h->sectiuni[section].offset + (logical_offset - h->sectiuni[section].logical_offset), no_of_bytes);
            write(fd2, STR_AND_LENGHT("READ_FROM_LOGICAL_SPACE_OFFSET$SUCCESS$"));
            free(h->sectiuni);
            free(h);
            break;
        }
        case EXIT:
        {
            close(fd1);
            close(fd2);
            close(fd);
            close(shmFD);
            munmap((void *)sharedMem, size);
            munmap(data, size_data);
            free(request_string);
            shm_unlink("/mGQYlA");
            unlink("RESP_PIPE_91328");
            return 0;
        }
        default:
            return 1;
        }
        free(request_string);
    }

    return 0;
}