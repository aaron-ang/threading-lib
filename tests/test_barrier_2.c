#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 10

pthread_barrier_t barrier;

void *thread_func(void *arg) {
  long thread_id = (long)arg;
  printf("Thread %ld: Before barrier\n", thread_id);

  // Wait for all threads to reach the barrier
  pthread_barrier_wait(&barrier);

  printf("Thread %ld: After barrier\n", thread_id);

  pthread_exit(NULL);
}

void *print_hello(void *arg) {
  for (int ii = 0; ii < 4; ++ii) {
    printf("hello!\n");
    usleep(100000);
  }
  pthread_exit(NULL);
}

int main() {
  pthread_t threads[NUM_THREADS];
  pthread_t hellothread;
  void *pret;

  // Initialize the barrier
  pthread_barrier_init(&barrier, NULL, NUM_THREADS);

  // Create threads
  pthread_create(&hellothread, NULL, print_hello, NULL);
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, thread_func, (void*)(long)i);
  }

  // Wait for all threads to finish
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], &pret);
  }
  pthread_join(hellothread, &pret);

  // Destroy the barrier
  pthread_barrier_destroy(&barrier);

  return 0;
}
