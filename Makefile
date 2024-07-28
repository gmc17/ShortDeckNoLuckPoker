.PHONY: all clean test run debug profile

CXX = g++
CXXFLAGS = -Wall -std=c++20 -I./include -Ofast -march=native -mtune=native -flto -ffast-math -funroll-loops -finline-functions
DEBUGFLAGS = -g -O0 -DDEBUG
PROFILEFLAGS = -pg
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread

SRCS = src/main.cpp src/game_state.cpp src/cfr.cpp src/ars_table.cpp src/info_set.cpp
TEST_SRCS = tests/tests.cpp src/game_state.cpp src/cfr.cpp src/ars_table.cpp src/info_set.cpp

MAIN = main
TEST = test_executable
DEBUG = main_debug
PROFILE = main_profile

all: $(MAIN)

$(MAIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(DEBUG): $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^ $(LIBS)

$(PROFILE): $(SRCS)
	$(CXX) $(CXXFLAGS) $(PROFILEFLAGS) -o $@ $^ $(LIBS)

test: $(TEST)
	./$(TEST)

run: $(MAIN)
	./$(MAIN)

debug: $(DEBUG)
	lldb ./$(DEBUG)

profile: $(PROFILE)
	./$(PROFILE)
	gprof $(PROFILE) gmon.out > profile_output.txt

clean:
	rm -f $(MAIN) $(TEST) $(DEBUG) $(PROFILE) gmon.out profile_output.txt