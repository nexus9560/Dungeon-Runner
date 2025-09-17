# Makefile for DungeonRunner

# Compiler and flags
CC = gcc
CFLAGS = -g

# -Wall -Wextra
# Source files and output
SRC_DIR = Dungeon-Runner
LIB_DIR = $(SRC_DIR)/adlibs
SRC = $(wildcard $(SRC_DIR)/*.c)
LIB = $(wildcard $(LIB_DIR)/*.c)
OUT_DIR = $(SRC_DIR)/build
OBJ = $(SRC:.c=.o)
EXECUTABLE = $(SRC_DIR)/Dungeon-Runner

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
	rm -f $(SRC_DIR)/*.o $(EXECUTABLE); rm -rf $(SRC_DIR)/debug-data/*.dat ; rm -rf $(SRC_DIR)/data/player.dat ; rm -rf $(SRC_DIR)/data/rooms.dat

# Run the program
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run
