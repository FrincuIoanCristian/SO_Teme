#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct
{
    int number_thread;
    int number_proces;
} TH_STRUCT;

sem_t sem1_1, sem1_2, sem2;
sem_t *sem6_5, *sem6_2;
// int nr_thread_2;

void *thread_function(void *arg)
{
    TH_STRUCT *s = (TH_STRUCT *)arg;
    if (s->number_proces == 2)
    {
        sem_wait(&sem2);
    }
    if(s->number_proces == 3 && s->number_thread == 3){
        sem_wait(&sem1_1);
    }
    if(s->number_proces == 3 && s->number_thread == 4){
        sem_wait(sem6_2);
    }
    if(s->number_proces == 6 && s->number_thread == 5){
        sem_wait(sem6_5);
    }
    info(BEGIN, s->number_proces, s->number_thread);

    if(s->number_proces == 3 && s->number_thread == 1){
        sem_post(&sem1_1);
        sem_wait(&sem1_2);
    }

    info(END, s->number_proces, s->number_thread);
    if(s->number_proces == 3 && s->number_thread == 3){
        sem_post(&sem1_2);
    }
    if (s->number_proces == 2)
    {
        sem_post(&sem2);
    }
    if(s->number_proces == 6 && s->number_thread == 2){
        sem_post(sem6_2);
    }
    if(s->number_proces == 3 && s->number_thread == 4){
        sem_post(sem6_5);
    }
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);
    sem_init(&sem2, 0, 5);
    sem_init(&sem1_1, 0, 0);
    sem_init(&sem1_2, 0, 0);
    sem_unlink("sem6_2");
    sem_unlink("sem6_5");
    sem6_2 = sem_open("/sem6_2", O_CREAT, 0644, 0);
    sem6_5 = sem_open("/sem6_5", O_CREAT, 0644, 0);

    if (fork() == 0)
    {
        info(BEGIN, 2, 0);
        pthread_t T2[39];
        TH_STRUCT s2[39];
        for (int i = 0; i < 39; i++)
        {
            s2[i].number_proces = 2;
            s2[i].number_thread = i + 1;
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
                s3[i].number_thread = i + 1;
                pthread_create(&T3[i], NULL, thread_function, &s3[i]);
            }
            for (int i = 0; i < 4; i++)
            {
                pthread_join(T3[i], NULL);
            }
            sem_destroy(&sem1_1);
            sem_destroy(&sem1_2);
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
                    s6[i].number_thread = i + 1;
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
    sem_close(sem6_2);
    sem_close(sem6_5);
    info(END, 1, 0);
    return 0;
}
