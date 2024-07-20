.PHONY: clean

# Define the compiler and flags
CXX = g++
CXXFLAGS = -Wall

# Define the target executable name
TARGET = a

# Define the source files
SRCS = main.cpp game_state.cpp

# The default target to build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Rule to run the tests
test: $(TARGET)
	./$(TARGET)

# Rule to clean the build files
clean:
	rm -f $(TARGET)