# Syntax help

# $^ : all dependencies
# $@ : target
# $< : first dependency
# $* : target without extension

CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = main.c db.c
OBJECTS = $(SRC:%.c=$(BIN_DIR)/%.o)
BIN_DIR = bin
TARGET = $(BIN_DIR)/hw3.o

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Create bin directory if it doesn't exist
$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


all: $(TARGET)
