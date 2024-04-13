#include <assert.h>
#include <pthread.h>
#include <stdio.h>

/* How many threads (aside from main) to create */
#define THREAD_CNT 127

#define COUNTER_FACTOR 10000

// locations for return values
int some_value[THREAD_CNT];

struct protected_int {
  int val;
  pthread_mutex_t mutex;
};

struct protected_int protected_int;

void *increment(void *arg) {
  int my_num = (long int)arg;
  int c = (my_num + 1) * COUNTER_FACTOR;
  int i;

  pthread_mutex_lock(&protected_int.mutex);
  protected_int.val = 0;
  for (i = 0; i < c; i++) {
    assert(protected_int.val == i);
    if ((i % 10000) == 0) {
      printf("id: 0x%lx num %d counted to %d of %d\n", pthread_self(), my_num,
             i, c);
    }
    protected_int.val++;
  }
  pthread_mutex_unlock(&protected_int.mutex);

  some_value[my_num] = my_num;
  pthread_exit(&some_value[my_num]);
  return NULL;
}

int main() {
  pthread_t threads[THREAD_CNT];
  unsigned long int i;

  pthread_mutex_init(&protected_int.mutex, NULL);

  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, increment, (void *)i);
  }
  for (i = 0; i < THREAD_CNT; i++) {
    void *pret;
    int ret;
    pthread_join(threads[i], &pret);
    assert(pret);
    ret = *(int *)pret;
    assert(ret == i);
  }

  pthread_mutex_destroy(&protected_int.mutex);
  return 0;
}
