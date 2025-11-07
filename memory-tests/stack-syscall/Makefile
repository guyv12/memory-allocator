# Makefile for C/Assembly program (32-bit x86)

TARGET = stack.exe
C_SOURCE = main.c
ASM_SOURCE = stack.s
C_OBJECT = $(C_SOURCE:.c=.o)
ASM_OBJECT = $(ASM_SOURCE:.s=.o)

# Default target: Link the executable
all: $(TARGET)

$(TARGET): $(C_OBJECT) $(ASM_OBJECT)
	gcc -m32 $^ -o $@

# Rule to compile the C source
$(C_OBJECT): $(C_SOURCE)
	gcc -m32 -c $< -o $@

# Rule to assemble the Assembly source (Intel syntax, 32-bit)
$(ASM_OBJECT): $(ASM_SOURCE)
	as --32 -msyntax=intel $< -o $@

# Cleanup rule
.PHONY: clean
clean:
	rm -f $(C_OBJECT) $(ASM_OBJECT) $(TARGET)

# (chatGPT generated)