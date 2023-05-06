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
#include <stdbool.h>

typedef struct
{
    int number_thread;
    int number_proces;
} TH_STRUCT;


//semafoarele anonime pentru ex 2.3
sem_t sem1_1, sem1_3;

//semafoarele cu nume pentru ex 2.5
sem_t *sem6_5, *sem6_2;

//variabilele globale, semaforul anonim, lacatul si condiitiile pentru ex 2.4
int nr_thread_block;
bool T2_13_close;
sem_t sem2;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

void *thread_function(void *arg)
{
    TH_STRUCT *s = (TH_STRUCT *)arg;
    if (s->number_proces == 3 && s->number_thread == 3) 
    {
        sem_wait(&sem1_1);  //daca suntem pe thread-ul T3.3 asteptam sa inceapa mai intai T3.1
    }
    if (s->number_proces == 3 && s->number_thread == 4)
    {
        sem_wait(sem6_2);
    }
    if (s->number_proces == 6 && s->number_thread == 5)
    {
        sem_wait(sem6_5);
    }
    info(BEGIN, s->number_proces, s->number_thread);

    if (s->number_proces == 3 && s->number_thread == 1)
    {
        sem_post(&sem1_1);  //am pornit T3.1 si trezim T3.3
        sem_wait(&sem1_3);  //daca suntem pe thread-ul T3.1 asteptam sa se termine mai intai T3.3
    }

    info(END, s->number_proces, s->number_thread);
    if (s->number_proces == 3 && s->number_thread == 3)
    {
        sem_post(&sem1_3);  //am inchis T3.3 si trezim T3.1
    }
    if (s->number_proces == 6 && s->number_thread == 2)
    {
        sem_post(sem6_2);
    }
    if (s->number_proces == 3 && s->number_thread == 4)
    {
        sem_post(sem6_5);
    }
    return NULL;
}


//->pentru ex 2.4 am facut o functie de thread separata pentru a-mi fi mai usor de observat 
//->pentru rezolvare am ales un semafor anonim care sa permita maxim 5 thread-uri sa ruleze simultan
//->pentru ultimul sub-punct am ales sa pornesc mai intai primele 4 thread-uri si thread-ul 13, restul le pun sa astepte pana cand voi inchide 
//thread-ul special, adica 13. Pentru primele 4 thread-uri le pornesc si le pun si pe ele sa astepte. In thread-ul 13 astept pana cand primele
//4 thread-uri au fost pornite dupa care afisez "END", si trezesc toate thread-urile blocate de cond si cond2 si le las sa isi execute in 
//continuare executia in mod normal

void *thread_function_process_2(void *param)
{
    int number_thread = *(int *)param;

    if(T2_13_close == false && number_thread > 4 && number_thread != 13){
        pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond2, &lock);
        pthread_mutex_unlock(&lock);
    }
    sem_wait(&sem2);

    info(BEGIN, 2, number_thread);

    if (number_thread < 5)
    {
        pthread_mutex_lock(&lock);
        nr_thread_block++;
        pthread_cond_wait(&cond, &lock);
        pthread_mutex_unlock(&lock);
    }

    if (number_thread == 13)
    {
        while (nr_thread_block < 4)
        {
            continue;
        }
    }

    info(END, 2, number_thread);

    if (number_thread == 13)
    {
        T2_13_close = true;
        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&cond);
        pthread_cond_broadcast(&cond2);
        pthread_mutex_unlock(&lock);
    }
    sem_post(&sem2);
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);
    sem_init(&sem2, 0, 5);
    sem_init(&sem1_1, 0, 0);
    sem_init(&sem1_3, 0, 0);
    sem_unlink("sem6_2");
    sem_unlink("sem6_5");
    sem6_2 = sem_open("/sem6_2", O_CREAT, 0644, 0);
    sem6_5 = sem_open("/sem6_5", O_CREAT, 0644, 0);

    if (fork() == 0)
    {
        info(BEGIN, 2, 0);
        pthread_t T2[39];
        int thread[39];
        T2_13_close = false;
        for (int i = 0; i < 39; i++)
        {

            thread[i] = i + 1;
            pthread_create(&T2[i], NULL, thread_function_process_2, &thread[i]);
        }

        for (int i = 0; i < 39; i++)
        {
            pthread_join(T2[i], NULL);
        }
        sem_destroy(&sem2);
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
        pthread_cond_destroy(&cond2);
        if (fork() == 0)
        {
            info(BEGIN, 3, 0);
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
            sem_destroy(&sem1_3);
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
