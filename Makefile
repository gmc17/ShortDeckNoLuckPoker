.PHONY: clean test run

# Define the compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++14 -I./include

# Define the target executable names
MAIN_TARGET = main
TEST_TARGET = test_executable

# Define the source files
MAIN_SRCS = main.cpp src/game_state.cpp
TEST_SRCS = tests/tests.cpp src/game_state.cpp

# Define the library paths
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread

# The default target to build the main executable
$(MAIN_TARGET): $(MAIN_SRCS)
	$(CXX) $(CXXFLAGS) -o $(MAIN_TARGET) $(MAIN_SRCS) $(LIBS)

# Target to build the test executable
$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SRCS) $(LIBS)

# Rule to run the tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Rule to run the main executable
run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

# Rule to clean the build files
clean:
	rm -f $(MAIN_TARGET) $(TEST_TARGET)