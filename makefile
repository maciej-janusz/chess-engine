# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
LDFLAGS = 

# Program name
TARGET = chess_engine

# Source files
SRCS = main.cpp board.cpp engine.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Header files
HEADERS = board.h engine.h

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Pattern rule for compiling .cpp files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)

# Phony targets
.PHONY: all clean run debug