# Compiler and flags
CC = gcc
WARNINGS = -Wall -Werror
OPTFLAGS = -O0 -g
STDFLAGS = -std=gnu99
override CFLAGS := $(WARNINGS) $(STDFLAGS) $(OPTFLAGS) $(CFLAGS) -I.

# Debug support
DEBUG=0
ifeq ($(DEBUG),1)
  override CFLAGS += -fsanitize=undefined
  override LDFLAGS += -fsanitize=undefined -fsanitize=leak
endif

# Directories
TESTDIR=tests

# Test configuration
test_files=test_busy_threads test_many_threads \
 test_random_threads test_new_threads \
 test_zombie_threads test_wait_thread test_sync \
 test_mutex test_mutex_2 test_mutex_3 \
 test_barrier test_barrier_2 test_barrier_3

custom_tests= test_one_thread test_custom_schedule \
 test_early_exit test_mutex test_barrier

# For cleanup, we need all test files regardless of PREEMPT setting
all_test_files=$(test_files) $(custom_tests)
all_test_binaries := $(addprefix $(TESTDIR)/,$(all_test_files))

# Preemption configuration
PREEMPT=1
ifeq ($(PREEMPT),0)
  override CFLAGS += -DPREEMPT=0
  test_files=$(custom_tests)
endif

# Library configuration
MY_PTHREADS=1
ifeq ($(MY_PTHREADS),1)
  mythread=threads.o
else
  mythread=
  override LDFLAGS += -pthread
endif

# Test building rules
test_files := $(addprefix $(TESTDIR)/,$(test_files))
objects := $(addsuffix .o,$(test_files))


# Targets
.PHONY: all build clean test  version

all: test

build: threads.o $(test_files)

# Run the test programs
test: $(test_files)
	@echo "Running tests..."
	@/bin/bash run_tests.sh $(test_files)

version:
	@echo "Threading Library Build System"
	@echo "CC: $(shell $(CC) --version | head -n 1)"
	@echo "CFLAGS: $(CFLAGS)"

clean:
	rm -f *.o *.d *~ $(TESTDIR)/*.o $(TESTDIR)/*.d $(all_test_binaries)

# Automatic dependency generation
%.d: %.c
	@$(CC) $(CFLAGS) -MM -MT '$*.o $@' $< > $@

-include $(objects:.o=.d)
-include threads.d

threads.o: threads.c ec440threads.h

$(test_files): %: %.o $(mythread)