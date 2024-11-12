# Define the compiler
CC = gcc

# Define compiler flags
CFLAGS = -Wall -Wextra -O2

# Define the source files and the target executable
SRC = main.c player.c referee.c animation.c  spinningsquare.c anim.c
OBJ = main.o player.o referee.o animation.o  spinningsquare.o anim.o
TARGET = game

# Default target to compile the program
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lGL -lGLU -lglut -lpthread -lm  # Link OpenGL, pthread, and math libraries

# Compile source files into object files
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

player.o: player.c
	$(CC) $(CFLAGS) -c player.c

referee.o: referee.c
	$(CC) $(CFLAGS) -c referee.c

animation.o: animation.c  # Added rule for animation.o
	$(CC) $(CFLAGS) -c animation.c

spinningsquare.o: spinningsquare.c  # Added rule for animation.o
	$(CC) $(CFLAGS) -c spinningsquare.c

anim.o: anim.c  # Added rule for animation.o
	$(CC) $(CFLAGS) -c anim.c

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up the generated files
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets so Make doesn't confuse them with files
.PHONY: all run clean
