#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define THREADS 16

pthread_mutex_t mutex;
int counter = 0;
int ii = 0;

void *thread_function(void *arg) {
  long thread_id = (long)arg;
  for (int i = 0; i < 1e8; ++i) {
    // Simulate some work
  }

  pthread_mutex_lock(&mutex);
  ii++;
  counter += ii;
  printf("Thread %ld: ii value: %d\n", thread_id, ii);
  printf("Thread %ld: Counter value: %d\n", thread_id, counter);
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}

int main() {
  pthread_t threads[THREADS];

  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < THREADS; i++) {
    pthread_create(&threads[i], NULL, thread_function, (void*)(long)i);
  }

  for (int i = 0; i < THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  assert(ii == THREADS);

  pthread_mutex_destroy(&mutex);
  return 0;
}
