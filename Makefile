override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.

# you can uncomment following to add sanitizer for development; note make sure to not have this for submissions
#override CFLAGS :=  -fsanitize=undefined $(CFLAGS)
#override LDFLAGS := -fsanitize=undefined -fsanitize=leak $(LDLAGS)

TESTDIR=tests

# Add any additional tests here
test_files=test_busy_threads test_many_threads \
 test_random_threads test_new_threads \
 test_zombie_threads test_wait_thread \
 test_sync counter_mutex test_barrier

custom_tests= test_one_thread test_custom_schedule \
 test_early_exit

PREEMPT=1
ifeq ($(PREEMPT),0)
  override CFLAGS += -DPREEMPT=0
  test_files=$(custom_tests)
endif

# Build the threads.o file
threads.o: threads.c ec440threads.h

all: check

# You might find it useful to write test programs and get them working against the installed pthreads
# to do so, set TSTMYPTHREADS to a value other than 1
TSTMYPTHREADS=1
ifeq ($(TSTMYPTHREADS),1)
  mythread=threads.o
else
  mythread=
  override LDFLAGS += -pthread
endif

CC = gcc

# rules to build each of the tests
test_files := $(addprefix $(TESTDIR)/,$(test_files))
objects := $(addsuffix .o,$(test_files))

$(objects): %.o: %.c

$(test_files): %: %.o $(mythread)


.PHONY: clean check checkprogs

# Build all of the test programs
checkprogs: $(test_files)

# Run the test programs
check: checkprogs
	/bin/bash run_tests.sh $(test_files)

clean:
	rm -f *.o *~ $(TESTDIR)/*.o $(test_files)
