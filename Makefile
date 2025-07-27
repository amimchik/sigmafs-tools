CC=clang
CFLAGS= -Wall -Wextra -I./include

SRC_DIR=src
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
BIN_DIR=$(BUILD_DIR)/bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = $(BIN_DIR)/my_program

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	@echo "=> Linking $(patsubst $(OBJ_DIR)/%,%,$^) -> $(TARGET)"
	@$(CC) $(OBJS) -o $(TARGET)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
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
	@./$(TARGET)

re: clean all

.PHONY: re clean run all dirs
