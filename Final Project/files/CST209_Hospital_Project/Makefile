CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

# Find all .cpp files in the src directory
SRCS = $(wildcard src/*.cpp)
# Replace src/ with obj/ and .cpp with .o
OBJS = $(SRCS:src/%.cpp=obj/%.o)

ifeq ($(OS),Windows_NT)
    TARGET = HospitalSystem.exe
    MKDIR_OBJ = if not exist obj mkdir obj
    MKDIR_DATA = if not exist data mkdir data
    CLEAN_CMD = rmdir /s /q obj 2>nul & del /q /f $(TARGET) 2>nul
else
    TARGET = HospitalSystem
    MKDIR_OBJ = mkdir -p obj
    MKDIR_DATA = mkdir -p data
    CLEAN_CMD = rm -rf obj $(TARGET)
endif

# Default target
all: obj_dir $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create obj directory if it doesn't exist
obj_dir:
	@$(MKDIR_OBJ)
	@$(MKDIR_DATA)

# Clean up build files
clean:
	@$(CLEAN_CMD)

.PHONY: all clean obj_dir

