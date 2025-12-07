# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lSDL2 -lSDL2_gfx -lSDL2_ttf -lm

# Target name
TARGET := exe

# Source and object files
SRC := main.c effects.c notes.c
OBJ := $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Link objects into final executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile each .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Utility rules
.PHONY: clean run

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)
