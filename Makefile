CC=clang
CFLAGS:=-Wall -Wextra -I./include -g

SRC_DIR=src
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
BIN_DIR=$(BUILD_DIR)/bin

HDRS = $(shell find include -type f -name "*.h")
SRCS = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = $(BIN_DIR)/my_program

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	@echo "=> Linking $(patsubst $(OBJ_DIR)/%,%,$^) -> $(TARGET)"
	@$(CC) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	@echo "===> Compiling $(patsubst $(SRC_DIR)/%,%,$<) -> $(patsubst $(OBJ_DIR)/%,%,$@)"
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

clean:
	@echo ":: Cleaning up"
	@rm -rf $(BUILD_DIR)

run: all
	@echo ":: Running target"
	@./$(TARGET) $(ARGS)

re: clean all

.PHONY: re clean run all dirs
