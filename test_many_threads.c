#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* How many threads (aside from main) to create */
#define THREAD_CNT 127

/* Each counter goes up to a multiple of this value. If your test is too fast
 * use a bigger number. Too slow? Use a smaller number. See the comment about
 * sleeping in count() to avoid this size-tuning issue.
 */
#define COUNTER_FACTOR 10000

// locations for return values
int some_value[THREAD_CNT];

void *count(void *arg) {
  int my_num = (long int)arg;
  int c = (my_num + 1) * COUNTER_FACTOR;
  int i;

  for (i = 0; i < c; i++) {
    if ((i % 10000) == 0) {
      printf("id: 0x%lx num %d counted to %d of %d\n", pthread_self(), my_num,
             i, c);
    }
  }
  some_value[my_num] = my_num;

  pthread_exit(&some_value[my_num]);
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t threads[THREAD_CNT];
  unsigned long int i;

  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, count, (void *)i);
  }

  /* Collect statuses of the other threads, waiting for them to finish */
  for (i = 0; i < THREAD_CNT; i++) {
    void *pret;
    int ret;
    pthread_join(threads[i], &pret);
    ret = *(int *)pret;
    assert(ret == i);
  }
  return 0;
}
