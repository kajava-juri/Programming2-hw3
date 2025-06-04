# Syntax help

# $^ : all dependencies
# $@ : target
# $< : first dependency
# $* : target without extension

# modified from https://gist.github.com/D3r3k23/b2174dbdc8c256958bf480abc8117ab2
# compiles all .c files in the current directory to build directory

CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -fanalyzer -fsanitize=address
LDFLAGS = -lsqlite3 -lc
BUILD_DIR = build
TARGET = $(BUILD_DIR)/hw3

SRCS := $(shell find ./ -name '*.c')
OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

# Tell Make where to find .c files
vpath %.c . ./db_api

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Include dependency files
-include $(BUILD_DIR)/*.d

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean
