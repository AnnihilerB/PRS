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

pthread_mutex_t mux;

struct file {
    struct elem_file *premier;
};

struct elem_file{
    
    void *param_event;
    struct itimerval it;
    struct elem_file *suivant;
    struct elem_file *pre;
    unsigned long temps;
};


struct file *File;
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
   // fprintf (stderr, "sdl_push_event(%p) appelée au temps %ld\n", File->premier->param_event, get_time ());
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
        sdl_push_event(File->premier->param_event);
        pthread_mutex_lock(&mux);
        supprimer_premier_element_file();
        triFile();
        pthread_mutex_unlock(&mux);
        setitimer(ITIMER_REAL, &File->premier->it, NULL);
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
    pthread_mutex_init(&mux, NULL);

    //Creation d'un masque pour bloquer la transmission du SIGALRM dans le processus
    pthread_t thread;
    sigset_t block_mask;
    
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGALRM);
    //Blocage du signal SIGALRM
    sigprocmask(SIG_BLOCK, &block_mask, NULL);

    pthread_create(&thread, NULL, demon, NULL);
    //Allocation de la Strucure de données
    File = malloc(sizeof(struct file));
    File->premier = NULL;
    return 1; // Implementation not ready
}

void triFile()
{
    struct itimerval it;
    getitimer(ITIMER_REAL, &it);
    //Calcul du delai restant
    long delai_restant_premier_elem = it.it_value.tv_sec*1000 + it.it_value.tv_usec/1000;
    
    struct elem_file *e;
    if (File->premier != NULL && File->premier->suivant != NULL)
    {
        e = File->premier->suivant;
        while (e->suivant != NULL)
        {
            //Récupération du temps de l'élément suivant
            long temps = e->suivant->it.it_value.tv_usec/1000 + e->suivant->it.it_value.tv_sec*1000;   
            
            //Tri de la file via décalage du premier élément.
            if (delai_restant_premier_elem < temps - delai_restant_premier_elem)
            {
                printf("Dans if tri\n");
                if (e == File->premier->suivant)
                {
                    printf("premier\n");
                    e->suivant->pre = File->premier->suivant;
                    File->premier->pre = e;
                    File->premier->suivant = e->suivant;
                    File->premier = e;
                    File->premier->pre = NULL;
                }
                else
                {
                    printf("deuxieme\n");
                    e->suivant->pre = e->pre;
                    e->pre->suivant = e->suivant;
                    e->pre = e->suivant;
                    e->suivant = e->suivant->suivant;
                    e->pre->suivant = e;
                }
            }
            e = e->suivant;
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
    
    // Allocation de la Strucure et affectation des paramètres.
    struct elem_file *elem = malloc(sizeof(struct elem_file));
        
    elem->suivant = elem->pre = NULL;
    elem->param_event = param;
    elem->it = it;
    elem->temps = get_time();
        
    pthread_mutex_lock(&mux);
    ajouter_element_file(elem);
    pthread_mutex_unlock(&mux);
        
    setitimer(ITIMER_REAL, &File->premier->it, NULL);
        
    triFile();
        

}

void ajouter_element_file(struct elem_file *e)
{
    //Ajout de l'élément en premier si la file ne contient pas d'élément.
    if (File->premier == NULL)
    {
        File->premier = e;
        File->premier->suivant = File->premier->pre = NULL;
    }
    //Ajout de l'élément à la suite des autres.
    else{
        struct elem_file *el;
        el = File->premier;
        while (el->suivant != NULL){
            el = el->suivant;
        }
        e->pre = el;
        e->suivant = NULL;
        el->suivant = e;
    }
    
}

void supprimer_premier_element_file()
{
    //Déréférencement et free du premier élément. 
    if (File->premier != NULL)
    {
        struct elem_file *e = File->premier;
        File->premier = File->premier->suivant;
        free(e->pre);
    }
}

void afficherFile()
{
    struct elem_file *e;
    e = File->premier;
    do{
        printf("%p: temps %d et %d adresse --> ", e, e->it.it_value.tv_sec, e->it.it_value.tv_usec);
        e = e->suivant;
    }while(e != NULL);
    printf("\n");
}
#endif


