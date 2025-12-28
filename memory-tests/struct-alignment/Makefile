# Variables
CC = gcc
CFLAGS = -Wall -Wextra -g -O3 -mavx
TARGET = main.exe

# Default target
all: $(TARGET)

# Link and compile in one step for a single file
$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

# Cleanup
.PHONY: clean
clean:
	rm -f $(TARGET)

# (gemini generated)