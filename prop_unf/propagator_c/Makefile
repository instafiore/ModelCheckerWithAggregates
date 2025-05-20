CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -I/opt/homebrew/include -I $(CONDA_PREFIX)/include $(LOGGER_ACTIVE) -O3 $(EXTRA_CFLAGS)
LDFLAGS = -L/opt/homebrew/lib -L$(CONDA_PREFIX)/lib -Wl,-rpath,/opt/homebrew/lib -Wl,-rpath,$(CONDA_PREFIX)/lib -lclingo $(EXTRA_LDFLAGS)
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
TARGET = propagator
C = cpp
SRCS = $(shell find $(SRC_DIR) -name '*.$(C)')
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.$(C)=.o)))

# Default to release mode
BUILD_MODE ?= release
# EXTRA_CFLAGS = -DLOGGER_ACTIVE -fsanitize=address -g
ifeq ($(BUILD_MODE), test)
    EXTRA_CFLAGS  = -DLOGGER_ACTIVE -fsanitize=address -g
    EXTRA_LDFLAGS = -fsanitize=address
else
    EXTRA_CFLAGS  =
    EXTRA_LDFLAGS =
endif

all: $(TARGET)

test:
	@$(MAKE) BUILD_MODE=test

release:
	@$(MAKE) BUILD_MODE=release

$(TARGET): $(OBJS)
	@echo "[make] Linking $@"
	@mkdir -p $(BIN_DIR)
	$(CC) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/*/%.$(C)
	@mkdir -p $(BUILD_DIR) # Create build directory if not exists
	@echo "[make] Compiling $< -> $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.$(C)
	@mkdir -p $(BUILD_DIR) # Create build directory if not exists
	@echo "[make] Compiling main $< -> $@"
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	@echo "[make] Cleaning up..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)