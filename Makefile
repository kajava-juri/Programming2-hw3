# Syntax help

# $^ : all dependencies
# $@ : target
# $< : first dependency
# $* : target without extension

# modified from https://gist.github.com/D3r3k23/b2174dbdc8c256958bf480abc8117ab2
# compiles all .c files in the current directory to build directory

CC = gcc
CFLAGS = -Wall -Wextra -g -MMD
LDFLAGS = -lsqlite3
BUILD_DIR = build
TARGET = $(BUILD_DIR)/hw3

# Find all .c files in current directory and subdirectories
SRCS := $(shell find ./ -name '*.c')
# Extract just filenames (without paths) and put in BUILD_DIR
OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# This is the tricky part - we need to find the source file for each object
$(BUILD_DIR)/%.o: | $(BUILD_DIR)
	$(eval SRC_FILE := $(shell find ./ -name '$(patsubst %.o,%.c,$(notdir $@))'))
	$(CC) $(CFLAGS) -c $(SRC_FILE) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Include dependency files
-include $(BUILD_DIR)/*.d

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean
