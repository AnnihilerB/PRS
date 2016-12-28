#define _XOPEN_SOURCE 600

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

void *param_global;

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
    fprintf (stderr, "sdl_push_event(%p) appelée au temps %ld\n", param_global, get_time ());
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
        fprintf(stderr, "Apres sigsuspend");
        //traitement de l'event
        sdl_push_event(param_global);
        fprintf(stderr, "Apres pushevent");
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
    fprintf(stderr, "Timer init\n");
    
    //Creation d'un masque pour bloquer la transmission du SIGALRM dans le processus
    pthread_t th;
    sigset_t block_mask;
    
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGALRM);
    
    sigprocmask(SIG_BLOCK, &block_mask, NULL);

    fprintf(stderr, "Create\n");
    pthread_create(&th, NULL, demon, NULL);
    return 1; // Implementation not ready
}


void timer_set (Uint32 delay, void *param)
{
    
    long int sec = delay / 1000;
    // Recuperation du reste de la division par mille et conversion en micro-secondes
    long int micro = (delay % 1000) * 1000;
    
    fprintf(stderr, "Delay : %d, sec : %ld, micro : %ld\n", delay, sec, micro);
    param_global = param;
    
    struct itimerval it;
    
    struct timeval tv_interval;
    struct timeval tv_value;
    
    // les deux valeurs sont importantes. pour 2,2 secondes, sec = 2 (en secondes) et micro = 200000 (0,2s converties en micro secondes) 
    tv_value.tv_sec = sec;
    tv_value.tv_usec = micro;
    
    it.it_interval = tv_interval;
    it.it_value = tv_value;
    
    setitimer(ITIMER_REAL, &it, NULL); 

    
}
#endif

