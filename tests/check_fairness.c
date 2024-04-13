#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define THREAD_CNT 127
#define DURATION 3

pthread_mutex_t mutex;
atomic_bool start;
time_t end_time;

struct thread_info {
  pthread_t tid;
  unsigned iters_done;
};

static void *thread_func(void *arg) {
  struct thread_info *my_info = (struct thread_info *)arg;
  unsigned i = 0;

  while (!start) {
  }; /* Wait until parent flags that all threads have been created */

  while (time(NULL) < end_time) {
    pthread_mutex_lock(&mutex);
    i++;
    pthread_mutex_unlock(&mutex);
  }

  my_info->iters_done = i;
  return NULL;
}

int main() {
  struct thread_info *tinfo;
  int i;

  start = false;
  end_time = time(NULL) + DURATION;
  assert(pthread_mutex_init(&mutex, NULL) == 0);

  printf(
      "Main thread: Beginning test with %d threads, running for %d seconds\n",
      THREAD_CNT, DURATION);

  tinfo = (struct thread_info *)calloc(THREAD_CNT, sizeof(struct thread_info));

  /* We create the requested number of threads, all doing the same work */
  for (i = 0; i < THREAD_CNT; i++) {
    (void)pthread_create(&tinfo[i].tid, NULL, thread_func, (void *)&tinfo[i]);
  }

  /* Wait for child threads to finish */
  for (i = 0; i < THREAD_CNT; i++) {
    start = true;
    pthread_join(tinfo[i].tid, NULL);
  }

  assert(pthread_mutex_destroy(&mutex) == 0);

  unsigned min_iters = UINT_MAX;
  unsigned max_iters = 0;
  unsigned tot_iters = 0;
  for (i = 0; i < THREAD_CNT; i++) {
    tot_iters += tinfo[i].iters_done;
    if (tinfo[i].iters_done < min_iters) {
      min_iters = tinfo[i].iters_done;
    }
    if (tinfo[i].iters_done > max_iters) {
      max_iters = tinfo[i].iters_done;
    }
    printf("Thread %d completed %u iters\n", i, tinfo[i].iters_done);
  }

  unsigned diff = max_iters - min_iters;
  printf("\nTOTAL: %u iterations completed.\n", tot_iters);
  printf("Most productive thread completed %u iterations, least productive "
         "thread completed %u iterations\n",
         max_iters, min_iters);
  printf("Difference is %u iters\n", diff);

  return 0;
}
