#include "../ec440threads.h"
#include <pthread.h>
#include <stdio.h>

#define THREAD_CNT 3

void *hello(void *arg) {
  printf("Hello from thread %ld\n", pthread_self());
  pthread_exit(&arg);
  return NULL;
}

int main() {
  pthread_t threads[THREAD_CNT];
  unsigned long i;
  for (i = 0; i < THREAD_CNT; i++) {
    pthread_create(&threads[i], NULL, hello, (void *)i);
  }

  // run all threads
  schedule(0);

  // wait for all threads to finish
  for (i = 0; i < THREAD_CNT; i++) {
    void *pret;
    int ret;
    pthread_join(threads[i], &pret);
    ret = *(unsigned long *)pret;
    assert(ret == i);
  }
}
