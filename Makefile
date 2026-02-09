# Default target: make uses the first non-comment target as the default. In your file that's tempraturec, not run. If you intended run to be default, put it first or add an all target.

# run: main
# 	./build/main
#
# main: ./src/main.c ./src/helper.c
# 	mkdir -p ./build/
# 	gcc ./src/main.c ./src/helper.c -o ./build/main
#
# clean:
# 	rm -f ./build/main

CC = gcc
CFLAGS = -I./include -Wall -Wextra -g
SRC = ./src/main.c ./src/helper.c ./game/main.c
BUILD_DIR = build
# OBJ = $(SRC:./src/%.c=$(BUILD_DIR)/%.o)
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/main

all: $(TARGET)

run: $(TARGET)
	@echo "Running program..."
	@$(TARGET)

debug: $(TARGET)
	@echo "Starting debugger..."
	@codelldb $(TARGET)

# Compile: src/*.c → build/*.o
# $(BUILD_DIR)/%.o: ./src/%.c | $(BUILD_DIR)
# 	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

ifeq ($(OS),Windows_NT)
$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
endif

clean:
	rm -rf $(BUILD_DIR)

