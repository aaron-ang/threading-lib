#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIMIT (1 << 26)

void *busy_func(void *arg) {
  int c = (long int)arg % LIMIT;

  for (int i = 0; i < c; i++) {
    if ((i % 10000) == 0) {
      printf("id: 0x%lx counted to %d of %d\n", pthread_self(), i, c);
    }
  }

  pthread_exit(arg);
  return NULL;
}

int main() {
  pthread_t thread;
  void *pret;
  long test;

  srand(time(NULL));
  test = rand();

  pthread_create(&thread, NULL, busy_func, (void *)test);
  pthread_join(thread, &pret);
  assert((long)pret == test);
}
