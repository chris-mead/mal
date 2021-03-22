
DEBUG?=1

CC:=g++

ifeq ($(DEBUG), 1)
    OPT_FLAGS:=-O0 -g
else
    OPT_FLAGS:=-O2
endif

WARNFLAGS:=-Wall -Wextra -Wextra -pedantic

CFLAGS:=$(OPT_FLAGS) --std=c++17 $(WARNFLAGS)

#STEPS_SRC:=$(wildcard step*.cpp)
#STEPS:=$(STEPS_SRC:%.cpp:=%)

step0_repl: step0_repl.cpp
	$(CC) $(CFLAGS) -o $@ $^

step1_read_print: step1_read_print.cpp
	$(CC) $(CFLAGS) -o $@ $^
