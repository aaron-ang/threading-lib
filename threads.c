#include "ec440threads.h"
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

enum thread_status { TS_EXITED, TS_RUNNING, TS_READY };

typedef struct thread_control_block {
  int id;
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

static TCB *get_new_thread();

static void thread_init(TCB *new_thread);

static void reg_init(TCB *new_thread, void *(*start_routine)(void *),
                     void *arg);

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

TCB *get_new_thread() {
  static int i = 0;
  int seen = 0;
  while (threads[i].status != TS_EXITED || threads[i].stack) {
    if (++seen >= MAX_THREADS)
      exit(EXIT_FAILURE);
    i = (i + 1) % MAX_THREADS;
  }
  threads[i].id = i;
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
  longjmp(threads[current_thread].registers, threads[current_thread].id + 1);
}

/*
 * Don't implement main in this file!
 * This is a library of functions, not an executable program. If you
 * want to run the functions in this file, create separate test programs
 * that have their own main functions.
 */
