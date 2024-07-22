.PHONY: clean test run

# Define the compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++20 -I./include

# Define the target executable names
MAIN_TARGET = main
TEST_TARGET = test_executable

# Define the source files
MAIN_SRCS = src/main.cpp src/game_state.cpp src/cfr.cpp
TEST_SRCS = tests/tests.cpp src/game_state.cpp src/cfr.cpp

# Define the library paths
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread

# Define object files
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# The default target to build the main executable
$(MAIN_TARGET): $(MAIN_OBJS)
	$(CXX) $(CXXFLAGS) -o $(MAIN_TARGET) $(MAIN_OBJS) $(LIBS)

# Target to build the test executable
$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS) $(LIBS)

# Rule to compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to run the tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Rule to run the main executable
run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

# Rule to clean the build files
clean:
	rm -f $(MAIN_TARGET) $(TEST_TARGET) $(MAIN_OBJS) $(TEST_OBJS)