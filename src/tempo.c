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
void* demon (void* arg){
    
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);
    
    fprintf(stderr, "Thread de pid : %d\n", getpid());
    
    //Remplacement du mask courant par l'ancien dans le thread pour recevoir SIGALRM
    
    sigsuspend(&mask);
    printf ("sdl_push_event(%p) appelée au temps %ld\n", param_global, get_time ());
    fprintf(stderr, "Apres sigsuspend");
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
    fprintf(stderr, "Timer init\n");
    
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
    
    param_global = param;
    
    struct itimerval it;
    
    struct timeval tv_interval;
    struct timeval tv_value;
    
    tv_value.tv_sec = delay / 1000;
    
    it.it_interval = tv_interval;
    it.it_value = tv_value;
    
    setitimer(ITIMER_REAL, &it, NULL); 

    
}
#endif

