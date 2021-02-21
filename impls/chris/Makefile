
DEBUG?=1

ifeq ($(DEBUG), 1)
    OPT_FLAGS:=-O0 -g
else
    OPT_FLAGS:=-O2
endif

CFLAGS:=$(OPT_FLAGS) --std=c++17

#STEPS_SRC:=$(wildcard step*.cpp)
#STEPS:=$(STEPS_SRC:%.cpp:=%)

step0_repl: step0_repl.cpp
	$(CC) $(CFLAGS) -o $@ $^

step1_read_print: step1_read_print.cpp
	$(CC) $(CFLAGS) -o $@ $^
