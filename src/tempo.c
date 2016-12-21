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

void* param_global = NULL; 
void handler(int s)
{
  if (param_global != NULL)
     printf("sdl_push_event(%p) appelée au temps %ld\n", param_global, get_time());
  else
     printf("test: %d\n", (int)pthread_self());
}
void *f (void* arg)
{
   struct sigaction sg;
   sg.sa_flags = 0;
   sg.sa_handler = handler;
   sigemptyset(&sg.sa_mask);
   sigaddset(&sg.sa_mask, SIGALRM);
   
   sigaction(SIGALRM, &sg, NULL);
   
   sigset_t old_mask;
   
   sigprocmask(SIG_BLOCK, &sg.sa_mask, &old_mask);
   
   sigsuspend(&old_mask);
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
  pthread_t th;
  pthread_create(&th, NULL, f, NULL);
  alarm(1);
  alarm(1);
  alarm(1);
  
  
  return 0; // Implementation not ready
}
/*
jmp_buf buf;
void handler (int s)
{
    siglongjmp(buf, 1);
}*/

void timer_set (Uint32 delay, void *param)
{
    struct itimerval ti;
    //ti.it_interval = NULL; Incompatibel pointer type a voir
    ti.it_value.tv_usec = delay;
    param_global = param;
    setitimer(ITIMER_REAL, &ti, NULL);
    
}

int main(int argc, char* argv[])
{
  timer_init();
  return 0;
}
#endif

