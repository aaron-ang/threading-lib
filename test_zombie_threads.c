#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* How many threads (aside from main) to create */
#define THREAD_CNT 127

// locations for return values
int some_value[THREAD_CNT];

void *hello(void *arg) {
  int my_num = (long int)arg;
  int val = rand();
  some_value[my_num] = val;
  printf("Hello from thread %ld with value %d\n", pthread_self(), val);

  pthread_exit(&some_value[my_num]);
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
  unsigned long int i;
  srand(time(NULL));

  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, hello, (void *)i);
  }

  void *pret;
  int ret;
  /* Wait for second half of the threads to finish.
  At this point most likely more than half of the threads are zombies.
  */
  for (int i = THREAD_CNT - 1; i >= THREAD_CNT / 2; i--) {
    pthread_join(threads[i], &pret);
    ret = *(int *)pret;
    assert(ret == some_value[i]);
  }

  /* Create more threads.
  The first half of the threads should not be overwritten by the new threads,
  since they are still zombies.
  */
  for (i = THREAD_CNT / 2; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, hello, (void *)i);
  }

  /* Collect statuses of all threads */
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_join(threads[i], &pret);
    ret = *(int *)pret;
    assert(ret == some_value[i]);
  }

  return 0;
}
