APP = window

BIN_DIR = bin
SRC_DIR = src
BUILD_DIR = build

SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

CC = clang
CFLAGS = -g -Wall
INC_FLAGS = -I$(SRC_DIR) -I/usr/include
LINK_FLAGS = -lwayland-client
DEFINES = 

all : build

build : $(BIN_DIR)/$(APP)

$(BIN_DIR)/$(APP) : $(OBJ)
	@mkdir -p $(BIN_DIR)
	clang $(CFLAGS) $(LINK_FLAGS) $(OBJ) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(INC_FLAGS) -c $< -o $@

run: build
	@cd $(BIN_DIR) && ./$(APP)

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

.PHONY: all build run clean