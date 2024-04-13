#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define THREAD_CNT 127
#define DURATION 3

pthread_mutex_t mutex;
pthread_barrier_t barrier;
time_t end_time;

struct thread_info {
  pthread_t tid;
  unsigned iters_done;
};

static void *thread_func(void *arg) {
  struct thread_info *my_info = (struct thread_info *)arg;
  unsigned i = 0;

  pthread_barrier_wait(&barrier); // Wait for all threads to be created

  while (time(NULL) < end_time) {
    pthread_mutex_lock(&mutex);
    i++;
    pthread_mutex_unlock(&mutex);
  }

  my_info->iters_done = i;
  pthread_exit(arg);
}

int main() {
  struct thread_info *tinfo;
  int i;
  void *pret;

  end_time = time(NULL) + DURATION;
  assert(pthread_mutex_init(&mutex, NULL) == 0);
  assert(pthread_barrier_init(&barrier, NULL, THREAD_CNT) == 0);

  printf(
      "Main thread: Beginning test with %d threads, running for %d seconds\n",
      THREAD_CNT, DURATION);

  tinfo = (struct thread_info *)calloc(THREAD_CNT, sizeof(struct thread_info));

  /* We create the requested number of threads, all doing the same work */
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&tinfo[i].tid, NULL, thread_func, (void *)&tinfo[i]);
  }

  unsigned min_iters = UINT_MAX;
  unsigned max_iters = 0;
  unsigned tot_iters = 0;
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_join(tinfo[i].tid, &pret);
    struct thread_info *my_info = (struct thread_info *)pret;
    tot_iters += my_info->iters_done;
    if (my_info->iters_done < min_iters) {
      min_iters = my_info->iters_done;
    }
    if (my_info->iters_done > max_iters) {
      max_iters = my_info->iters_done;
    }
    printf("Thread %ld completed %u iters\n", (long)my_info->tid,
           my_info->iters_done);
  }

  assert(pthread_mutex_destroy(&mutex) == 0);
  assert(pthread_barrier_destroy(&barrier) == 0);

  unsigned diff = max_iters - min_iters;
  printf("\nTOTAL: %u iterations completed.\n", tot_iters);
  printf("Most productive thread completed %u iterations, least productive "
         "thread completed %u iterations\n",
         max_iters, min_iters);
  printf("Difference is %u iters\n", diff);

  free(tinfo);
  return 0;
}
