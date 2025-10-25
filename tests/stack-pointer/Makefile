# Makefile for C/Assembly program

TARGET = stack
C_SOURCE = main.c
ASM_SOURCE = stack.s
C_OBJECT = $(C_SOURCE:.c=.o)
ASM_OBJECT = $(ASM_SOURCE:.s=.o)

# Default target: Link the executable
all: $(TARGET)

$(TARGET): $(C_OBJECT) $(ASM_OBJECT)
	gcc $^ -o $@

# Rule to compile the C source
$(C_OBJECT): $(C_SOURCE)
	gcc -c $< -o $@

# Rule to assemble the Assembly source (using Intel syntax and 64-bit)
$(ASM_OBJECT): $(ASM_SOURCE)
	as -msyntax=intel --64 $< -o $@

# Cleanup rule
.PHONY: clean
clean:
	rm -f $(C_OBJECT) $(ASM_OBJECT) $(TARGET)