
DEBUG?=1

CC:=g++

ifeq ($(DEBUG), 1)
    OPT_FLAGS:=-O0 -g
else
    OPT_FLAGS:=-O2
endif

BUILD=build

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD)/$*.d
WARNFLAGS:=-Wall -Wextra -Wextra -pedantic
CFLAGS=$(OPT_FLAGS) --std=c++17 $(WARNFLAGS)

LIB_SRCS:=lexer.cpp\
          parser.cpp
LIB_OBJS:=$(patsubst %.cpp,$(BUILD)/%.o,$(LIB_SRCS))

STEP_SRCS:=step0_repl.cpp\
           step1_read_print.cpp

ALL_SRCS:=$(LIB_SRC) $(STEP_SRCS)
DEPS:=$(patsubst %.cpp,$(BUILD)/%.d,$(ALL_SRCS))

$(BUILD)/%.o: %.cpp | build
	$(CC)  $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

step0_repl: $(BUILD)/step0_repl.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

step1_read_print: $(BUILD)/step1_read_print.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

-include $(DEPS)
