#include "ec440threads.h"
#include <pthread.h>
#include <stdio.h>

void *hello(void *arg) {
  printf("Hello from thread %ld\n", pthread_self());
  return NULL;
}

int main() {
  pthread_t thread;
  void *pret;

  pthread_create(&thread, NULL, hello, NULL);
  schedule(0);
  pthread_join(thread, &pret);
}
