#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

#include "timer.h"

bool sup_elem_file;
pthread_mutex_t mux;

struct file {
    struct elem_file *first;
};
struct file *File;

struct elem_file{
    
    void *param_event;
    struct itimerval it;
    struct elem_file *next;
    struct elem_file *pre;
};

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

// Routine de traitement
void routine(int sig){
    fprintf (stderr, "sdl_push_event(%p) appelée au temps %ld\n", File->first->param_event, get_time ());
}

void* demon (void* arg){
    
    //Strucure obligatoire car SIGALRM fait quitter le programme si il n'est pas traité par un handler.
    struct sigaction s;
    s.sa_flags = 0;
    sigemptyset(&s.sa_mask);
    s.sa_handler = routine;
    
    sigaction(SIGALRM, &s, NULL);
    
    sigset_t mask;
    //Remplissage du masque de signaux pour bloquer tous les signaux.
    sigfillset(&mask);
    //Suppression de SIGALRM du masque pour qu'il puisse être délivré. Il est le seul signal non bloqué.
    sigdelset(&mask, SIGALRM);
    
    fprintf(stderr, "Thread de pid : %d\n", getpid());
    
    //Boucle pour traiter les events en continu.
    while(1){
        //Attente de SIGALRM
        sigsuspend(&mask);
        fprintf(stderr, "Apres sigsuspend\n");
        //traitement de l'event
        
        sdl_push_event(File->first->param_event);
        pthread_mutex_lock(&mux);
        supprimer_premier_element_file();
        pthread_mutex_unlock(&mux);
        fprintf(stderr, "Apres pushevent\n");
        
        
        
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
    pthread_mutex_init(&mux, NULL);
    fprintf(stderr, "Timer init\n");
    
    //Creation d'un masque pour bloquer la transmission du SIGALRM dans le processus
    pthread_t th;
    sigset_t block_mask;
    
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGALRM);
    
    sigprocmask(SIG_BLOCK, &block_mask, NULL);

    fprintf(stderr, "Create\n");
    pthread_create(&th, NULL, demon, NULL);
    
    File = malloc(sizeof(struct file));
    File->first = NULL;
    
    sup_elem_file = false;
    
    return 1; // Implementation not ready
}
void triFile()
{
    struct itimerval it;
    getitimer(ITIMER_REAL, &it);
    long delai_restant_premier_elem = it.it_value.tv_sec*1000 + it.it_value.tv_usec/1000;
    struct elem_file *e = File->first->next;
    if (e != NULL)
    {
        while (e->next != NULL)
        {
            long temps = e->next->it.it_value.tv_usec/1000 + e->next->it.it_value.tv_sec*1000;   
            if (delai_restant_premier_elem > temps - delai_restant_premier_elem)
            {
                if (e == File->first->next)
                {
                    e->next->pre = File->first;
                    File->first->pre = e;
                    File->first->next = e->next;
                    File->first = e;
                    File->first->pre = NULL;
                }
                else
                {
                    e->next->pre = e->pre;
                    e->pre->next = e->next;
                    e->pre = e->next;
                    e->next = e->next->next;
                    e->pre->next = e;
                }
                
                e->it.it_value.tv_sec -= it.it_value.tv_sec;
                e->it.it_value.tv_usec -= it.it_value.tv_usec;
            }
            e = e->next;
        }
    }
}
void timer_set (Uint32 delay, void *param)
{        
    long int sec = delay / 1000;
    // Recuperation du reste de la division par mille et conversion en micro-secondes
    long int micro = (delay % 1000) * 1000;
    
    fprintf(stderr, "Delay : %d, sec : %ld, micro : %ld\n", delay, sec, micro);
    
    struct itimerval it;
    
    struct timeval tv_interval;
    struct timeval tv_value;
    
     // les deux valeurs sont importantes. pour 2,2 secondes, sec = 2 (en secondes) et micro = 200000 (0,2s converties en micro secondes) 
    tv_value.tv_sec = sec;
    tv_value.tv_usec = micro;
    
    it.it_interval = tv_interval;
    it.it_value = tv_value;
    
    
        struct elem_file *elem = malloc(sizeof(struct elem_file));
        elem->next = elem->pre = NULL;
        elem->param_event = param;
        elem->it = it;
        pthread_mutex_lock(&mux);
        ajouter_element_file(elem);
        triFile();
        afficherFile();
        pthread_mutex_unlock(&mux);
        sup_elem_file = false;
        setitimer(ITIMER_REAL, &File->first->it, NULL); 
        
   
    

    
}

void ajouter_element_file(struct elem_file *e)
{
    if (File->first == NULL)
    {
        printf("test\n");
        File->first = e;
        File->first->next = File->first->pre = NULL;
    }
    else{
        struct elem_file *el;
        el = File->first;
        while (el->next != NULL){
            el = el->next;
        }
        e->pre = el;
        e->next = NULL;
        el->next = e;
    }
    
}
void supprimer_premier_element_file()
{
    if (File->first != NULL)
    {
         File->first->pre = NULL;
         File->first = File->first->next;
         if (File->first != NULL && File->first->next != NULL)
            File->first->next->pre = File->first;
         printf("caca\n");
    }
}
void afficherFile()
{
    struct elem_file *e;
    e = File->first;
    do{
        printf("%p: temps %d et %d --> ", e, e->it.it_value.tv_sec, e->it.it_value.tv_usec);
        e = e->next;
    }while(e != NULL);
    printf("\n");
}
#endif


