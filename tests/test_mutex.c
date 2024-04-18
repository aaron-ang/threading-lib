#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#define THREAD_CNT 126
#define NUM_ITERS 10000

pthread_mutex_t mutex;
volatile long shared_counter;

// locations for return values
int some_value[THREAD_CNT];

static void *increment_thread(void *arg) {
  int my_num = (long)arg;
  for (int i = 0; i < NUM_ITERS; i++) {
    pthread_mutex_lock(&mutex);
    shared_counter++;
    pthread_mutex_unlock(&mutex);
  }
  some_value[my_num] = NUM_ITERS;
  pthread_exit(&some_value[my_num]);
}

static void *decrement_thread(void *arg) {
  int my_num = (long)arg;
  for (int i = 0; i < NUM_ITERS; i++) {
    pthread_mutex_lock(&mutex);
    shared_counter--;
    pthread_mutex_unlock(&mutex);
  }
  some_value[my_num] = NUM_ITERS;
  pthread_exit(&some_value[my_num]);
}

int main() {
  pthread_t *tids;
  long i;
  void *pret;

  shared_counter = 0;
  tids = malloc(THREAD_CNT * sizeof(pthread_t));
  assert(pthread_mutex_init(&mutex, NULL) == 0);

  for (i = 0; i < THREAD_CNT; i += 2) {
    pthread_create(&tids[i], NULL, increment_thread, (void *)i);
    pthread_create(&tids[i + 1], NULL, decrement_thread, (void *)i);
  }
  for (i = 0; i < THREAD_CNT; i += 1) {
    pthread_join(tids[i], &pret);
  }

  assert(pthread_mutex_destroy(&mutex) == 0);
  assert(shared_counter == 0);
  return 0;
}
