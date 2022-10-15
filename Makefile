##
# SLP A2 testsystem
#
# @file
# @version 1.0.0

CC=gcc
C_FLAGS=-Wall -D_GNU_SOURCE -pthread

CLANG_FORMAT=clang-format

RMF=rm -f

all:: slp-a2-test
slp-a2-test: main.c vector.c worker.c
	$(CC) $(C_FLAGS) -o $@ $^

format: main.c vector.h vector.c worker.h worker.c
	$(CLANG_FORMAT) -i $^

clean:: ; $(RMF) slp-a2-test


# end
