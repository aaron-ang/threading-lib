# Threading Library
This project implements a simple user-level scheduler within a process with thread synchronization.

## Scheduler
Implements the following pthread interfaces:
- `pthread_create`: Creates a new thread with the specified start routine and arguments
- `pthread_exit`: Terminates the calling thread and makes its exit status available
- `pthread_join`: Waits for a specific thread to terminate and retrieves its exit status
- `pthread_self`: Returns the ID of the calling thread

Threads are cycled in a [round-robin](https://en.wikipedia.org/wiki/Round-robin_scheduling) fashion and the scheduler uses `SIGALRM` for timeouts.

## Synchronization features
1. Thread locking and unlocking, to prevent the scheduler from running at specific times.
2. An implementation of mutex interfaces:
   - `pthread_mutex_init`: Initializes a mutex with default attributes
   - `pthread_mutex_lock`: Acquires a lock on the specified mutex
   - `pthread_mutex_destroy`: Destroys a mutex, freeing its resources
   - `pthread_mutex_unlock`: Releases a lock on the specified mutex
3. An implementation of barrier interfaces:
   - `pthread_barrier_init`: Initializes a barrier with a specific thread count
   - `pthread_barrier_wait`: Makes the calling thread wait at a barrier until all threads reach it
   - `pthread_barrier_destroy`: Destroys a barrier, freeing its resources

## Data Structures
```c
enum thread_status { TS_EXITED, TS_READY, TS_RUNNING, TS_BLOCKED };

typedef struct thread_control_block {
  pthread_t id;
  jmp_buf registers;
  void *stack;
  enum thread_status status;
  void *ret_val;
  bool has_mutex;
} TCB;

typedef struct {
  int flag;
} mutex_t;

typedef union {
  pthread_mutex_t mutex;
  mutex_t my_mutex;
} my_mutex_t;

typedef struct {
  queue_t waitqueue;
  unsigned limit;
  unsigned count;
} barrier_t;

typedef union {
  pthread_barrier_t barrier;
  barrier_t my_barrier;
} my_barrier_t;
```

## Additions
### Helper functions
- `void scheduler_init()`: Initializes the scheduler by assigning the first thread as the main thread
- `void init_handler()`: Initializes the signal handler (schedule) for the timer
- `TCB *get_new_thread()`: Returns a new thread from the thread pool
- `void thread_init(TCB *new_thread)`: Allocates the stack for the new thread
- `void reg_init(TCB *new_thread, void *(*start_routine)(void *), void *arg)`: Initializes the registers for the new thread to run the start routine
- `void lock()`: Blocks SIGALRM to prevent scheduling
- `void unlock()`: Unblocks SIGALRM to allow scheduling
- `void clear_waitlist(queue_t *waitqueue)`: Moves all threads from the wait queue to the ready queue

### Test cases
- [test_one_thread.c](tests/test_one_thread.c): Tests the creation, execution, and cleanup of a single thread
- [test_custom_schedule.c](tests/test_custom_schedule.c): Tests the scheduling of a few threads
- [test_early_exit.c](tests/test_early_exit.c): Tests the handling of main thread exiting early
- [test_many_threads.c](tests/test_many_threads.c): Tests the handling of the maximum allowed number of threads
- [test_random_threads.c](tests/test_random_threads.c): Tests the handling of a few threads counting up to a random number
- [test_new_threads.c](tests/test_new_threads.c): Tests the cleanup of old threads and the creation of new threads
- [test_zombie_threads.c](tests/test_zombie_threads.c): Tests the handling of zombie threads (threads that have exited but have not been joined)
- [test_wait_thread.c](tests/test_wait_thread.c): Tests waiting for one busy thread to finish
- [test_mutex.c](tests/test_mutex.c): Tests basic mutex operations
- [test_mutex_2.c](tests/test_mutex_2.c): Tests mutex with multiple threads
- [test_mutex_3.c](tests/test_mutex_3.c): Tests mutex contention scenarios
- [test_barrier.c](tests/test_barrier.c): Tests basic barrier functionality
- [test_barrier_2.c](tests/test_barrier_2.c): Tests barriers with multiple threads
- [test_barrier_3.c](tests/test_barrier_3.c): Tests complex barrier scenarios
- [test_busy_threads.c](tests/test_busy_threads.c): Tests scheduler with CPU-intensive threads
- [test_sync.c](tests/test_sync.c): Tests synchronization between threads




