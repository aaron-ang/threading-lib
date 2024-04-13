#include "ec440threads.h"
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PREEMPT
#define PREEMPT 1 /* flag to enable preemption */
#endif

#define MAX_THREADS 128             /* number of threads you support */
#define THREAD_STACK_SIZE (1 << 15) /* size of stack in bytes */
#define QUANTUM (50 * 1000)         /* quantum in usec */

typedef struct {
  int flag;
  int *waitlist;
} mutex_t;

typedef union {
  pthread_mutex_t mutex;
  mutex_t my_mutex;
} my_mutex_t;

typedef struct {
  int *waitlist;
  unsigned limit;
  unsigned count;
  unsigned phase;
} barrier_t;

typedef union {
  pthread_barrier_t barrier;
  barrier_t my_barrier;
} my_barrier_t;

enum thread_status { TS_EXITED, TS_BLOCKED, TS_RUNNING, TS_READY };

typedef struct thread_control_block {
  pthread_t id;
  jmp_buf registers;
  void *stack;
  enum thread_status status;
  void *ret_val;
} TCB;

TCB threads[MAX_THREADS];
int current_thread = 0;
int num_threads = 0;

static void scheduler_init();

static void init_handler();

static void lock();

static void unlock();

static TCB *get_new_thread();

static void thread_init(TCB *new_thread);

static void reg_init(TCB *new_thread, void *(*start_routine)(void *),
                     void *arg);

static void add_thread_to_waitlist(int thread_index, int *waitlist);

static void clear_waitlist(int *waitlist);

void scheduler_init() {
  assert(num_threads == 0);

  TCB *main_thread = get_new_thread();
  thread_init(main_thread);

  assert(num_threads == 1);
  main_thread->status = TS_RUNNING;

  if (PREEMPT)
    init_handler();
}

void init_handler() {
  struct sigaction sa = {
      .sa_handler = schedule,
      .sa_flags = SA_NODEFER,
  };
  sigaction(SIGALRM, &sa, NULL);
  ualarm(QUANTUM, QUANTUM);
}

void lock() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  assert(sigprocmask(SIG_BLOCK, &mask, NULL) == 0);
}

void unlock() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  assert(sigprocmask(SIG_UNBLOCK, &mask, NULL) == 0);
}

int test_and_set(int *flag) {
  int ret = *flag;
  *flag = 1;
  return ret;
}

TCB *get_new_thread() {
  static long i = 0;
  int seen = 0;
  while (threads[i].status != TS_EXITED || threads[i].stack) {
    if (++seen >= MAX_THREADS)
      exit(EXIT_FAILURE);
    i = (i + 1) % MAX_THREADS;
  }
  threads[i].id = (pthread_t)i;
  return &threads[i];
}

void thread_init(TCB *new_thread) {
  new_thread->stack = malloc(THREAD_STACK_SIZE);
  assert(new_thread->stack);
  num_threads++;
}

void reg_init(TCB *new_thread, void *(*start_routine)(void *), void *arg) {
  if (setjmp(new_thread->registers))
    return;

  set_reg(&new_thread->registers, JBL_PC, (unsigned long)start_thunk);
  set_reg(&new_thread->registers, JBL_R12, (unsigned long)start_routine);
  set_reg(&new_thread->registers, JBL_R13, (unsigned long)arg);

  unsigned long *sp = new_thread->stack + THREAD_STACK_SIZE;
  set_reg(&new_thread->registers, JBL_RSP, (unsigned long)--sp);
  *sp = (unsigned long)pthread_exit;
}

void schedule(int signal) {
  if (threads[current_thread].status == TS_RUNNING) {
    if (setjmp(threads[current_thread].registers))
      return;
    threads[current_thread].status = TS_READY;
  }

  int i = current_thread;
  do {
    i = (i + 1) % MAX_THREADS;
    if (i == current_thread)
      return;
  } while (threads[i].status != TS_READY);

  current_thread = i;
  threads[current_thread].status = TS_RUNNING;
  longjmp(threads[current_thread].registers,
          (long)threads[current_thread].id + 1);
}

void add_thread_to_waitlist(int thread_index, int *waitlist) {
  assert(waitlist);
  for (int i = 0; waitlist[i]; i++) {
    if (waitlist[i] == thread_index)
      return;
    if (waitlist[i] == 0) {
      waitlist[i] = thread_index;
      return;
    }
  }
}

void clear_waitlist(int *waitlist) {
  if (waitlist == NULL)
    return;
  for (int i = 0; waitlist[i]; i++) {
    threads[waitlist[i]].status = TS_READY;
    waitlist[i] = 0;
  }
}

// Library functions

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg) {
  // Create the timer and handler for the scheduler. Create thread 0.
  static bool is_first_call = true;
  if (is_first_call) {
    is_first_call = false;
    scheduler_init();
  }

  if (num_threads >= MAX_THREADS)
    exit(EXIT_FAILURE);

  TCB *new_thread = get_new_thread();
  *thread = (pthread_t)new_thread->id;

  thread_init(new_thread);
  reg_init(new_thread, start_routine, arg);
  new_thread->status = TS_READY;

  return 0;
}

void pthread_exit(void *value_ptr) {
  threads[current_thread].ret_val = value_ptr;
  threads[current_thread].status = TS_EXITED;
  schedule(0);
  exit(EXIT_SUCCESS);
}

pthread_t pthread_self(void) { return (pthread_t)threads[current_thread].id; }

int pthread_join(pthread_t thread, void **retval) {
  if (retval == NULL)
    return -1;

  int id = (long)thread;
  while (threads[id].status != TS_EXITED)
    schedule(0);

  *retval = threads[id].ret_val;
  assert(*retval);

  memset(threads[id].registers, 0, sizeof(jmp_buf));
  free(threads[id].stack);
  threads[id].stack = NULL;
  threads[id].ret_val = NULL;

  num_threads--;
  return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr) {
  my_mutex_t *m = (my_mutex_t *)mutex;
  m->my_mutex.flag = 0;
  m->my_mutex.waitlist = calloc(MAX_THREADS, sizeof(int));
  assert(m->my_mutex.waitlist);
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  my_mutex_t *m = (my_mutex_t *)mutex;
  m->my_mutex.flag = 0;
  clear_waitlist(m->my_mutex.waitlist);
  free(m->my_mutex.waitlist);
  m->my_mutex.waitlist = NULL;
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  lock();
  my_mutex_t *m = (my_mutex_t *)mutex;
  while (test_and_set(&m->my_mutex.flag)) {
    threads[current_thread].status = TS_BLOCKED;
    add_thread_to_waitlist(current_thread, m->my_mutex.waitlist);
    unlock();
    schedule(0);
    lock();
  }
  assert(m->my_mutex.flag == 1);
  unlock();
  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  lock();
  my_mutex_t *m = (my_mutex_t *)mutex;
  m->my_mutex.flag = 0;
  clear_waitlist(m->my_mutex.waitlist);
  unlock();
  return 0;
}

int pthread_barrier_init(pthread_barrier_t *restrict barrier,
                         const pthread_barrierattr_t *attr, unsigned count) {
  if (count == 0) {
    return EINVAL;
  }
  my_barrier_t *b = (my_barrier_t *)barrier;
  b->my_barrier.waitlist = calloc(count, sizeof(int));
  assert(b->my_barrier.waitlist);
  b->my_barrier.limit = count;
  b->my_barrier.count = 0;
  b->my_barrier.phase = 0;
  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
  my_barrier_t *b = (my_barrier_t *)barrier;
  clear_waitlist(b->my_barrier.waitlist);
  free(b->my_barrier.waitlist);
  memset(&b->my_barrier, 0, sizeof(my_barrier_t));
  return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
  lock();
  my_barrier_t *b = (my_barrier_t *)barrier;
  b->my_barrier.count++;
  if (b->my_barrier.count >= b->my_barrier.limit) {
    b->my_barrier.count = 0;
    b->my_barrier.phase++;
    clear_waitlist(b->my_barrier.waitlist);
    unlock();
    return PTHREAD_BARRIER_SERIAL_THREAD;
  } else {
    unsigned phase = b->my_barrier.phase;
    do {
      threads[current_thread].status = TS_BLOCKED;
      add_thread_to_waitlist(current_thread, b->my_barrier.waitlist);
      unlock();
      schedule(0);
      lock();
    } while (phase == b->my_barrier.phase);
    unlock();
    return 0;
  }
}
