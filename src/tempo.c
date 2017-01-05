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
    unsigned long temps;
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
   // fprintf (stderr, "sdl_push_event(%p) appelée au temps %ld\n", File->first->param_event, get_time ());
}

void* demon (void* arg){
    
    struct itimerval it;
    getitimer(ITIMER_REAL, &it);
    
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
    
    //Boucle pour traiter les events en continu.
    while(1){
        //Attente de SIGALRM
        sigsuspend(&mask);

        
        //traitement de l'event
        sdl_push_event(File->first->param_event);
        pthread_mutex_lock(&mux);
        supprimer_premier_element_file();
        pthread_mutex_unlock(&mux);
        setitimer(ITIMER_REAL, &File->first->it, NULL);
        triFile();
        
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

    pthread_create(&th, NULL, demon, NULL);
    
    File = malloc(sizeof(struct file));
    File->first = NULL;
    
    sup_elem_file = false;
    
    return 1; // Implementation not ready
}
void triFile()
{
    
    struct itimerval it;
    
    long delai_restant_premier_elem = it.it_value.tv_sec*1000 + it.it_value.tv_usec/1000;
    
    struct elem_file *e;
    if (File->first != NULL && File->first->next != NULL)
    {
        e = File->first->next;
        while (e->next != NULL)
        {
            long temps = e->next->it.it_value.tv_usec/1000 + e->next->it.it_value.tv_sec*1000;   
            
            if (delai_restant_premier_elem < temps - delai_restant_premier_elem)
            {
                printf("Dans if tri\n");
                if (e == File->first->next)
                {
                    printf("premier\n");
                    e->next->pre = File->first->next;
                    File->first->pre = e;
                    File->first->next = e->next;
                    File->first = e;
                    File->first->pre = NULL;
                }
                else
                {
                    printf("deuxieme\n");
                    e->next->pre = e->pre;
                    e->pre->next = e->next;
                    e->pre = e->next;
                    e->next = e->next->next;
                    e->pre->next = e;
                }
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
        elem->temps = get_time();
        pthread_mutex_lock(&mux);
        ajouter_element_file(elem);
        pthread_mutex_unlock(&mux);
        sup_elem_file = false;
        elem->temps = get_time();
        setitimer(ITIMER_REAL, &File->first->it, NULL);
        triFile();
        afficherFile();
        
        
}

void ajouter_element_file(struct elem_file *e)
{
    printf("Ajout\n");
    if (File->first == NULL)
    {
        printf("if\n");
        File->first = e;
        File->first->next = File->first->pre = NULL;
    }
    else{
        printf("else\n");
        struct elem_file *el;
        el = File->first;
        while (el->next != NULL){
            printf("while\n");
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
         //afficherFile();
    }
}
void afficherFile()
{
    struct elem_file *e;
    e = File->first;
    do{
        printf("%p: temps %d et %d adresse --> ", e, e->it.it_value.tv_sec, e->it.it_value.tv_usec);
        e = e->next;
    }while(e != NULL);
    printf("\n");
}
#endif


