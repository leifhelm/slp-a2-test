##
# SLP A2 testsystem
#
# @file
# @version 1.0.0

CC=gcc
C_FLAGS=-Wall -D_GNU_SOURCE -pthread

CLANG_FORMAT=clang-format

C_FILES=main.c vector.c worker.c invariants.c state.c
H_FILES=vector.h worker.h invariants.h state.h error.h
ALL_FILES=$(C_FILES) $(H_FILES)

RMF=rm -f

all:: slp-a2-test
slp-a2-test: $(C_FILES)
	$(CC) $(C_FLAGS) -o $@ $^

format: $(ALL_FILES)
	$(CLANG_FORMAT) -i $^

check:: check-format
check-format: $(ALL_FILES)
	$(CLANG_FORMAT) -Werror -n $^

clean:: ; $(RMF) slp-a2-test


# end
