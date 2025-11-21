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
SRC = ./src/main.c ./src/helper.c
OBJ = $(SRC:.c=.o)
BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

all: $(TARGET)

run: $(TARGET)
	@echo "Running program..."
	@$(TARGET)

$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# OS-specific directory creation
ifeq ($(OS),Windows_NT)
$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
endif

clean:
	rm -rf $(BUILD_DIR)

