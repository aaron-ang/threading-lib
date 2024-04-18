#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define THREADS 10

pthread_mutex_t mutex;
int counter = 0;
int ii = 0;

void *thread_function(void *arg) {
  pthread_mutex_lock(&mutex);
  printf("Thread %d: ii value: %d\n", *((int *)arg), ii);
  ii++;
  sleep(1);
  counter = counter + ii;
  printf("Thread %d: Counter value: %d\n", *((int *)arg), counter);
  pthread_mutex_unlock(&mutex);
  pthread_exit(NULL);
}

int main() {
  pthread_t threads[THREADS];
  int thread_ids[THREADS];

  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < THREADS; i++) {
    thread_ids[i] = i + 1;
    pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
  }

  for (int i = 0; i < THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);
  return 0;
}
