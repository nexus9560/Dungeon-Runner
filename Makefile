# Makefile for DungeonRunner

# Compiler and flags
CC = gcc
CFLAGS = -g

# -Wall -Wextra
# Source files and output
SRC_DIR = Dungeon-Runner
SRC = $(wildcard $(SRC_DIR)/*.c)
OUT_DIR = $(SRC_DIR)/build
OBJ = $(SRC:.c=.o)
EXECUTABLE = $(SRC_DIR)/

# Default target
all: $(EXECUTABLE)

# Linking
$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compilation
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@ -lm

# Clean up
clean:
	rm -f $(SRC_DIR)/*.o $(EXECUTABLE)

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run
