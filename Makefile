# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -D_GNU_SOURCE -pthread -Iinclude
LDFLAGS = -pthread

# Target binary name
TARGET  = my_project

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Find all .c files in the src directory
SRCS    = $(wildcard $(SRC_DIR)/*.c)

# Convert src/filename.c to obj/filename.o
OBJS    = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default rule
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into an .o file inside the obj/ directory
# The '| $(OBJ_DIR)' ensures the directory exists before compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up build files and the object directory
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
