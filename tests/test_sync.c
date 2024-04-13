#include <assert.h>
#include <pthread.h>

/* How many threads (aside from main) to create */
#define THREAD_CNT 127

#define COUNTER_FACTOR 10000

struct protected_int {
  volatile int val;
  pthread_mutex_t mutex;
};

struct protected_int my_int;

void *increment(void *arg) {
  int my_num = (long int)arg;
  int c = (my_num + 1) * COUNTER_FACTOR;
  int i;

  pthread_mutex_lock(&my_int.mutex);
  my_int.val = 0;
  for (i = 0; i < c; i++) {
    assert(my_int.val == i);
    my_int.val++;
  }
  pthread_mutex_unlock(&my_int.mutex);
  return NULL;
}

int main() {
  pthread_t threads[THREAD_CNT];
  unsigned long int i;

  pthread_mutex_init(&my_int.mutex, NULL);

  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, increment, (void *)i);
  }
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&my_int.mutex);
  return 0;
}
