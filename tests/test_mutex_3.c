#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex;

void *thread_func(void *arg) {
  long thread_id = (long)arg;
  if (thread_id != 2) {
    pthread_mutex_lock(&mutex);
    printf("Thread %ld acquired the mutex.\n", thread_id);
    // Simulate some work
    usleep(500000);
    printf("Thread %ld released the mutex.\n", thread_id);
    pthread_mutex_unlock(&mutex);
  } else {
    pthread_mutex_lock(&mutex);
    printf("Thread %ld acquired the mutex.\n", thread_id);
    // Hold the mutex for a longer time to let other threads contend for it
    usleep(1000000);
    printf("Thread %ld released the mutex.\n", thread_id);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main() {
  pthread_t threads[5];

  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < 5; i++) {
    pthread_create(&threads[i], NULL, thread_func, (void*)(long)(i + 1));
  }

  for (int i = 0; i < 5; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);

  return 0;
}