#include "ec440threads.h"
#include <pthread.h>
#include <stdio.h>

#define THREAD_CNT 3

void *hello(void *arg) {
  printf("Hello from thread %ld\n", pthread_self());
  return NULL;
}

int main() {
  pthread_t threads[THREAD_CNT];
  unsigned long int i;
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, hello, (void *)i);
  }
}
