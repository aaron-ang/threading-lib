#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#define THREAD_CNT 126
#define NUM_ITERS 1000000

pthread_mutex_t mutex;
volatile long shared_counter;

static void *increment_thread(void *arg) {
  long niters = (long)arg;
  for (int i = 0; i < niters; i++) {
    pthread_mutex_lock(&mutex);
    shared_counter++;
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(arg);
  return NULL;
}

static void *decrement_thread(void *arg) {
  long niters = (long)arg;
  for (int i = 0; i < niters; i++) {
    pthread_mutex_lock(&mutex);
    shared_counter--;
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(arg);
  return NULL;
}

int main() {
  pthread_t *tids;
  int i;
  void *pret;

  shared_counter = 0;
  tids = (pthread_t *)malloc(THREAD_CNT * sizeof(pthread_t));
  assert(pthread_mutex_init(&mutex, NULL) == 0);

  for (i = 0; i < THREAD_CNT; i += 2) {
    (void)pthread_create(&tids[i], NULL, increment_thread, (void *)NUM_ITERS);
    (void)pthread_create(&tids[i + 1], NULL, decrement_thread,
                         (void *)NUM_ITERS);
  }
  for (i = 0; i < THREAD_CNT; i += 2) {
    pthread_join(tids[i], &pret);
    pthread_join(tids[i + 1], &pret);
  }

  assert(pthread_mutex_destroy(&mutex) == 0);
  assert(shared_counter == 0);
  return 0;
}
