.PHONY: all clean test run debug

CXX = g++
CXXFLAGS = -Wall -std=c++20 -I./include
DEBUGFLAGS = -g -O0 -DDEBUG
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread

SRCS = src/main.cpp src/game_state.cpp src/cfr.cpp
TEST_SRCS = tests/tests.cpp src/game_state.cpp src/cfr.cpp

MAIN = main
TEST = test_executable
DEBUG = main_debug

all: $(MAIN)

$(MAIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(DEBUG): $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^ $(LIBS)

test: $(TEST)
	./$(TEST)

run: $(MAIN)
	./$(MAIN)

debug: $(DEBUG)
	lldb ./$(DEBUG)

clean:
	rm -f $(MAIN) $(TEST) $(DEBUG)