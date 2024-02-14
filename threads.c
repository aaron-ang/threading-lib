#include "ec440threads.h"
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 128             /* number of threads you support */
#define THREAD_STACK_SIZE (1 << 15) /* size of stack in bytes */
#define QUANTUM (50 * 1000)         /* quantum in usec */

/*
   Thread_status identifies the current state of a thread. What states could a
   thread be in? Values below are just examples you can use or not use.
 */
enum thread_status { TS_EXITED, TS_RUNNING, TS_READY };

/* The thread control block stores information about a thread. You will
 * need one of this per thread. What information do you need in it?
 * Hint, remember what information Linux maintains for each task?
 */
typedef struct thread_control_block {
  int id;
  jmp_buf registers;
  void *stack;
  enum thread_status status;
  void *exit_state;
} TCB;

TCB threads[MAX_THREADS];
int current_thread = 0;
int num_threads = 0;

void init_handler();
TCB *get_new_thread();
void thread_init(TCB *new_thread);
void reg_init(TCB *new_thread, void *(*start_routine)(void *), void *arg);

// to supress compiler error saying these static functions may not be used...
static void schedule(int signal) __attribute__((unused));

static void schedule(int signal) {
  if (threads[current_thread].status == TS_RUNNING) {
    if (setjmp(threads[current_thread].registers))
      return;
    threads[current_thread].status = TS_READY;
  }

  int start_thread = current_thread;
  do
    current_thread = (current_thread + 1) % num_threads;
  while (threads[current_thread].status != TS_READY &&
         current_thread != start_thread);

  threads[current_thread].status = TS_RUNNING;
  longjmp(threads[current_thread].registers, 1);
}

static void scheduler_init() {
  /*
     TODO: do everything that is needed to initialize your scheduler.
     For example:
     - allocate/initialize global threading data structures
     - create a TCB for the main thread. so your scheduler will be able to
     schedule it
     - set up your timers to call scheduler...
  */
  assert(num_threads == 0);
  TCB *main_thread = get_new_thread();
  thread_init(main_thread);
  assert(num_threads == 1);
  init_handler();
  main_thread->status = TS_RUNNING;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg) {
  // Create the timer and handler for the scheduler. Create thread 0.
  static bool is_first_call = true;
  if (is_first_call) {
    is_first_call = false;
    scheduler_init();
  }

  /* TODO: Return 0 on successful thread creation, non-zero for an error.
   *       Be sure to set *thread on success.
   *
   * You need to create and initialize a TCB (thread control block) including:
   * - Allocate a stack for the thread
   * - Set up the registers for the functions, including:
   *   - Assign the stack pointer in the thread's registers to point to its
   * stack.
   *   - Assign the program counter in the thread's registers.
   *   - figure out how to have pthread_exit invoked if thread returns
   * - After you are done, mark your new thread as READY
   * Hint: Easiest to use setjmp to save a set of registers that you then
   * modify, and look at notes on reading/writing registers saved by setjmp
   * using Hint: Be careful where the stackpointer is set to it may help to draw
   *       an empty stack diagram to answer that question.
   * Hint: Read over the comment in header file on start_thunk before
   *       setting the PC.
   *
   * Don't forget to assign RSP too! Functions know where to
   * return after they finish based on the calling convention (AMD64 in
   * our case). The address to return to after finishing start_routine
   * should be the first thing you push on your stack.
   */
  TCB *new_thread = get_new_thread();
  *thread = (pthread_t)new_thread->id;

  thread_init(new_thread);
  reg_init(new_thread, start_routine, arg);

  schedule(0);
  return 0;
}

void pthread_exit(void *value_ptr) {
  /* TODO: Exit the current thread instead of exiting the entire process.
   * Hints:
   * - Release all resources for the current thread. CAREFUL though.
   *   If you free() the currently-in-use stack then do something like
   *   call a function or add/remove variables from the stack, bad things
   *   can happen.
   * - Update the thread's status to indicate that it has exited
   * What would you do after this?
   */
  free(threads[current_thread].stack);
  threads[current_thread].exit_state = value_ptr;
  threads[current_thread].status = TS_EXITED;
  schedule(0);
  exit(0);
}

pthread_t pthread_self(void) {
  /*
   * TODO: Return the current thread instead of -1, note it is up to you what
   * ptread_t refers to
   */
  return (pthread_t)threads[current_thread].id;
}

int pthread_join(pthread_t thread, void **retval) {
  /* TODO: wait for the thread identified by the ID “thread” to terminate.
   * If that thread has already terminated, then it returns immediately with the
   * retval passed by pthread_exit. You should clean up all information related
   * to the terminated thread that you did not on pthread_exit.
   */
  *retval = threads[(long)thread].exit_state;
  return 0;
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
  int i = 0;
  while (threads[i].status != TS_EXITED)
    i = (i + 1) % MAX_THREADS;
  threads[i].id = i;
  return &threads[i];
}

// Set up thread control block
void thread_init(TCB *new_thread) {
  new_thread->exit_state = NULL;
  new_thread->stack = malloc(THREAD_STACK_SIZE);
  assert(new_thread->stack);
  if (setjmp(new_thread->registers))
    return;
  new_thread->status = TS_READY;
  num_threads++;
}

// Set up registers
void reg_init(TCB *new_thread, void *(*start_routine)(void *), void *arg) {
  unsigned long pc = get_reg(&new_thread->registers, JBL_PC);
  pc = _ptr_mangle((unsigned long)start_thunk);
  set_reg(&new_thread->registers, JBL_PC, pc);
  set_reg(&new_thread->registers, JBL_R12, (unsigned long)start_routine);
  set_reg(&new_thread->registers, JBL_R13, (unsigned long)arg);

  unsigned long *sp = new_thread->stack + THREAD_STACK_SIZE;
  set_reg(&new_thread->registers, JBL_RSP, (unsigned long)--sp);
  *sp = (unsigned long)pthread_exit;
}

/*
 * Don't implement main in this file!
 * This is a library of functions, not an executable program. If you
 * want to run the functions in this file, create separate test programs
 * that have their own main functions.
 */
