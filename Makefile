# Compiler and flags
CC = gcc
WARNINGS = -Wall -Werror
OPTFLAGS = -O0 -g
STDFLAGS = -std=gnu99
override CFLAGS := $(WARNINGS) $(STDFLAGS) $(OPTFLAGS) $(CFLAGS) -I.

# Debug support - uncomment for development
#override CFLAGS += -fsanitize=undefined
#override LDFLAGS += -fsanitize=undefined -fsanitize=leak

# Directories
TESTDIR=tests

# Test configuration
test_files=test_busy_threads test_many_threads \
 test_random_threads test_new_threads \
 test_zombie_threads test_wait_thread \
 test_sync test_mutex test_mutex_2 mutextest \
 test_barrier test_barrier_2 barriertest

custom_tests= test_one_thread test_custom_schedule \
 test_early_exit test_mutex mutextest \
 test_barrier barriertest

# Preemption configuration
PREEMPT=1
ifeq ($(PREEMPT),0)
  override CFLAGS += -DPREEMPT=0
  test_files=$(custom_tests)
endif

# Library configuration
TSTMYPTHREADS=1
ifeq ($(TSTMYPTHREADS),1)
  mythread=threads.o
else
  mythread=
  override LDFLAGS += -pthread
endif

# Build rules
threads.o: threads.c ec440threads.h

# Test building rules
test_files := $(addprefix $(TESTDIR)/,$(test_files))
objects := $(addsuffix .o,$(test_files))

$(objects): %.o: %.c

$(test_files): %: %.o $(mythread)

# Automatic dependency generation
%.d: %.c
	@$(CC) $(CFLAGS) -MM -MT '$*.o $@' $< > $@

-include $(objects:.o=.d)
-include threads.d

# Targets
.PHONY: all build clean check checkprogs version

all: check

build: threads.o checkprogs

# Build all test programs
checkprogs: $(test_files)

# Run the test programs
check: checkprogs
	@echo "Running tests..."
	@/bin/bash run_tests.sh $(test_files)

version:
	@echo "Threading Library Build System"
	@echo "CC: $(shell $(CC) --version | head -n 1)"
	@echo "CFLAGS: $(CFLAGS)"

clean:
	rm -f *.o *.d *~ $(TESTDIR)/*.o $(TESTDIR)/*.d $(test_files)
