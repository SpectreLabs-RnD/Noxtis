# -------------------------
# Compiler settings
# -------------------------
CC       := gcc
CFLAGS   := -std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -Wno-unused-result -I/usr/include -I./src
LDFLAGS  := -lsodium

BUILD_DIR := build
SRC_DIR   := src

# -------------------------
# Source files
# -------------------------
LOCAL_SRCS  := $(wildcard $(SRC_DIR)/*_local.c)
LOCAL_OBJS  := $(LOCAL_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
LOCAL_TARGET := noxtis_local

REMOTE_SRCS  := $(wildcard $(SRC_DIR)/*_remote.c)
REMOTE_OBJS  := $(REMOTE_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
REMOTE_TARGET := noxtis_remote

# -------------------------
# Default target
# -------------------------
all: local remote

# -------------------------
# Build rules
# -------------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile _local objects
$(BUILD_DIR)/%_local.o: $(SRC_DIR)/%_local.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile _remote objects
$(BUILD_DIR)/%_remote.o: $(SRC_DIR)/%_remote.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Local binary
local: $(LOCAL_OBJS)
	$(CC) $(LOCAL_OBJS) -o $(LOCAL_TARGET) $(LDFLAGS)

# Remote binary
remote: $(REMOTE_OBJS)
	$(CC) $(REMOTE_OBJS) -o $(REMOTE_TARGET) $(LDFLAGS)

# -------------------------
# Utilities
# -------------------------
clean:
	rm -rf $(BUILD_DIR) $(LOCAL_TARGET) $(REMOTE_TARGET)

debug: CFLAGS := -std=c11 -g -O0 -Wall -Wextra -Wshadow -fsanitize=address,undefined -I/usr/include -I./src
debug: LDFLAGS := -lsodium
debug: local

.PHONY: all clean debug local remote
