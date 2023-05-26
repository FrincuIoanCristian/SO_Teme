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

    write(fd2, "CONNECT$", 8);
    printf("SUCCESS\n");

    int shmFD;
    volatile char *sharedMem = NULL;
    char *data = NULL;
    shm_unlink("/mGQYlA");
    unsigned int size, size_data;
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

        switch (request)
        {
        case ECHO:
        {
            write(fd2, "ECHO$", 5);
            write(fd2, "VARIANT$", 8);
            unsigned int variant = 91328;
            write(fd2, &variant, 4);
            break;
        }
        case CREATE_SHM:
        {
            read(fd1, &size, 4);

            shmFD = shm_open("/mGQYlA", O_CREAT | O_RDWR, 0664);
            if (shmFD < 0)
            {
                write(fd2, "CREATE_SHM$", 11);
                write(fd2, "ERROR$", 6);
                break;
            }
            ftruncate(shmFD, size);
            sharedMem = (volatile char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
            if (sharedMem == (void *)-1)
            {
                write(fd2, "CREATE_SHM$", 11);
                write(fd2, "ERROR$", 6);
                break;
            }
            write(fd2, "CREATE_SHM$", 11);
            write(fd2, "SUCCESS$", 8);
            break;
        }
        case WRITE_TO_SHM:
        {
            unsigned int offset, value;
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &value, sizeof(unsigned int));
            // printf("%d, %d\n", offset, value);
            if (offset < 0 || offset + sizeof(unsigned int) > 4051247)
            {
                write(fd2, "WRITE_TO_SHM$", sizeof("WRITE_TO_SHM$"));
                write(fd2, "ERROR$", 6);
                break;
            }
            *(unsigned int*)(sharedMem + offset) = value;
            write(fd2, "WRITE_TO_SHM$", sizeof("WRITE_TO_SHM$"));
            write(fd2, "SUCCESS$", 8);
            break;
        }
        case MAP__FILE:
        {
            char file[256];
            i = 0;
            while (true)
            {
                read(fd1, &c, 1);
                if(c == '$'){
                    break;
                }
                file[i++] = c;
            }
            file[i] = '\0';
            fd = open(file, O_RDONLY);
            if (fd == -1)
            {
                write(fd2, "MAP_FILE$ERROR$", sizeof("MAP_FILE$ERROR$"));
                break;
            }
            size_data = lseek(fd, 0,SEEK_END);
            lseek(fd, 0, SEEK_SET);
            data = (char *)mmap(NULL, size_data, PROT_READ, MAP_PRIVATE, fd, 0);
            if(data == (void*)-1){
                write(fd2, "MAP_FILE$ERROR$", sizeof("MAP_FILE$ERROR$"));
                break;
            }
            write(fd2, "MAP_FILE$SUCCESS$", sizeof("MAP_FILE$SUCCESS$"));
            break;
        }
        case READ_FROM_FILE_OFFSET:{
            unsigned int offset, no_of_bytes;
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &no_of_bytes, sizeof(unsigned int));
            // printf("%d, %d\n", offset, no_of_bytes);
            if(offset < 0 || offset + no_of_bytes > size_data || sharedMem == (void *)-1 || data == (void*)-1){
                write(fd2, "READ_FROM_FILE_OFFSET$ERROR$", sizeof("READ_FROM_FILE_OFFSET$ERROR$"));
                break;
            }
            strncpy((char *)sharedMem, data+offset, no_of_bytes);
            write(fd2, "READ_FROM_FILE_OFFSET$SUCCESS$", sizeof("READ_FROM_FILE_OFFSET$SUCCESS$"));
            break;
        }
        case EXIT:
        {
            close(fd1);
            close(fd2);
            close(fd);
            shm_unlink("/mGQYlA");
            close(shmFD);
            munmap((void *)sharedMem, size);
            munmap(data, size_data);
            free(request_string);
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