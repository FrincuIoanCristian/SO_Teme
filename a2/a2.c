#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct
{
    int numer_thread;
    int number_proces;
    sem_t *sem;
} TH_STRUCT;

void *thread_function(void *arg)
{
    TH_STRUCT *s = (TH_STRUCT *)arg;
    if (s->number_proces == 2)
    {
        sem_wait(s->sem);
    }
    info(BEGIN, s->number_proces, s->numer_thread);

    info(END, s->number_proces, s->numer_thread);
    if (s->number_proces == 2)
    {
        sem_post(s->sem);
    }
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);

    if (fork() == 0)
    {
        info(BEGIN, 2, 0);
        pthread_t T2[39];
        TH_STRUCT s2[39];
        sem_t sem2;
        sem_init(&sem2, 0, 5);
        for (int i = 0; i < 39; i++)
        {
            s2[i].number_proces = 2;
            s2[i].numer_thread = i + 1;
            s2[i].sem = &sem2;
            pthread_create(&T2[i], NULL, thread_function, &s2[i]);
        }

        for (int i = 0; i < 39; i++)
        {
            pthread_join(T2[i], NULL);
        }
        sem_destroy(&sem2);
        if (fork() == 0)
        {
            info(BEGIN, 3, 0);
            // creare thread-uri pentru procesul 3
            pthread_t T3[4];
            TH_STRUCT s3[4];
            for (int i = 0; i < 4; i++)
            {
                s3[i].number_proces = 3;
                s3[i].numer_thread = i + 1;
                pthread_create(&T3[i], NULL, thread_function, &s3[i]);
            }

            for (int i = 0; i < 4; i++)
            {
                pthread_join(T3[i], NULL);
            }

            info(END, 3, 0);
            exit(0);
        }
        if (fork() == 0)
        {
            info(BEGIN, 4, 0);
            if (fork() == 0)
            {
                info(BEGIN, 6, 0);
                pthread_t T6[6];
                TH_STRUCT s6[6];
                for (int i = 0; i < 6; i++)
                {
                    s6[i].number_proces = 6;
                    s6[i].numer_thread = i + 1;
                    pthread_create(&T6[i], NULL, thread_function, &s6[i]);
                }

                for (int i = 0; i < 6; i++)
                {
                    pthread_join(T6[i], NULL);
                }
                info(END, 6, 0);
                exit(0);
            }
            wait(NULL);
            info(END, 4, 0);
            exit(0);
        }
        wait(NULL);
        wait(NULL);
        info(END, 2, 0);
        exit(0);
    }
    if (fork() == 0)
    {
        info(BEGIN, 5, 0);

        info(END, 5, 0);
        exit(0);
    }

    if (fork() == 0)
    {
        info(BEGIN, 7, 0);

        info(END, 7, 0);
        exit(0);
    }
    wait(NULL);
    wait(NULL);
    wait(NULL);
    info(END, 1, 0);
    return 0;
}
