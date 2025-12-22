# Compiler and flags
CXX := clang++
CXXFLAGS := -std=c++20 \
	-Wall \
	-Wextra \
	-rdynamic \
	-fno-exceptions \
	-fno-rtti\
	-Wno-missing-designated-field-initializers\
	-Wno-multichar \
	-Wno-reorder-init-list

# Directories
SRC_DIR := src
BIN_DIR := bin

# Source files
SRCS := $(wildcard $(SRC_DIR)/**/*.cpp) $(wildcard $(SRC_DIR)/*.cpp)

# Object files
# OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

test: $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$@ entry/$@.cpp $(SRCS)
	$(BIN_DIR)/$@

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
