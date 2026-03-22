# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -D_GNU_SOURCE -pthread -Iinclude
LDFLAGS = -pthread

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Target binary
TARGET_NAME = server 
TARGET      = $(BIN_DIR)/$(TARGET_NAME)

# Find all .c files in the src directory
SRCS    = $(wildcard $(SRC_DIR)/*.c)

# Convert src/filename.c to obj/filename.o
OBJS    = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default rule
all: $(TARGET)

# Link the object files to create the executable
# The '| $(BIN_DIR)' ensures the bin folder exists before linking
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into an .o file inside the obj/ directory
# The '| $(OBJ_DIR)' ensures the obj folder exists before compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory creation rules
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up build files and directories
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
