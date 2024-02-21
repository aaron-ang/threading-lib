# EC440: Threading Library

The sample code provided includes:

- [ec440threads.h](ec440threads.h)
- [Makefile](Makefile)
- [threads.c](threads.c)
- [run_tests.sh](run_tests.sh)

Test cases added

- [test_one_thread.c](test_one_thread.c): Tests the creation, execution, and cleanup of a single thread
- [test_custom_schedule.c](test_custom_schedule.c): Tests the scheduling of a few threads
- [test_many_threads.c](test_many_threads.c): Tests the handling of the maximum allowed number of threads
- [test_random_threads.c](test_random_threads.c): Tests the handling of a few threads counting up to a random number
- [test_new_threads.c](test_new_threads.c): Tests the cleanup of old threads and the creation of new threads
- [test_zombie_threads.c](test_zombie_threads.c): Tests the handling of zombie threads (threads that have exited but have not been joined)

## `threads.c`

### Thread control block (TCB) structure:
```c
typedef struct thread_control_block {
  int id;
  jmp_buf registers;
  void *stack;
  enum thread_status status;
  void *ret_val;
} TCB;
```

### Helper functions:
- `void scheduler_init()`: Initializes the scheduler by assigning the first thread as the main thread
- `void init_handler()`: Initializes the signal handler (schedule) for the timer
- `TCB *get_new_thread()`: Returns a new thread from the thread pool
- `void thread_init(TCB *new_thread)`: Allocates the stack for the new thread
- `void reg_init(TCB *new_thread, void *(*start_routine)(void *), void *arg)`: Initializes the registers for the new thread to run the start routine

