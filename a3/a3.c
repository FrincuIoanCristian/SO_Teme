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

enum
{
    ECHO,
    CREATE_SHM,
    EXIT,

};

void request_select(char *request, int *caz)
{
    if (strncmp(request, "ECHO", 4) == 0)
    {
        *caz = ECHO;
        return;
    }
    if (strncmp(request, "CREATE_SHM", 10) == 0)
    {
        *caz = CREATE_SHM;
        return;
    }
    if (strncmp(request, "EXIT", 4) == 0)
    {
        *caz = EXIT;
        return;
    }
}

int main()
{

    if (mkfifo("RESP_PIPE_91328", 0600) != 0)
    {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }

    int fd1, fd2;
    fd1 = open("REQ_PIPE_91328", O_RDONLY);
    if (fd1 == -1)
    {
        printf("ERROR\ncannot open the request pipe\n");
        return 1;
    }
    // printf("aici\n");
    fd2 = open("RESP_PIPE_91328", O_WRONLY);

    // printf("aici\n");
    write(fd2, "CONNECT$", 8);
    printf("SUCCES\n");

    while (true)
    {
        char *request_string = (char *)malloc(50 * sizeof(char));
        // char request_string[256];
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
            unsigned int size;
            read(fd1, &size, 4);
            write(fd2, "CREATE_SHM$", 11);
            int shmFD;
            volatile char *sharedMem = NULL;
            shmFD = shm_open("/mGQYlA", O_CREAT | O_RDWR, 0664);
            if (shmFD < 0)
            {
                write(fd2, "ERROR$", 6);
                break;
            }
            ftruncate(shmFD, size);
            sharedMem = (volatile char *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
            if (sharedMem == (void *)-1)
            {
                write(fd2, "ERROR$", 6);
                break;
            }
            write(fd2, "SUCCESS$", 8);
            break;
        }
        case EXIT:
        {
            close(fd1);
            close(fd2);
            free(request_string);
            unlink("RESP_PIPE_91328");
        }
        default:
            break;
        }
        free(request_string);
    }

    return 0;
}