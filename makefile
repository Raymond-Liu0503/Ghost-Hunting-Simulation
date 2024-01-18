# Compiler and compiler flags
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -pthread

# Source files
SOURCES := evidence.c ghost.c house.c hunter.c main.c logger.c room.c utils.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Target executable
TARGET := fp

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(TARGET)