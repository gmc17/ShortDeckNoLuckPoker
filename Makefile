.PHONY: all clean test run debug profile quick

CXX = g++
CXXFLAGS = -Wall -std=c++20 -I./include -Ofast -march=native -mtune=native -flto -ffast-math -funroll-loops -finline-functions
DEBUGFLAGS = -g -O0 -DDEBUG
PROFILEFLAGS = -g -fprofile-instr-generate -fcoverage-mapping
QUICKFLAGS = -O1 -Wall -std=c++20 -I./include
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread
SRCS = src/main.cpp src/game_state.cpp src/cfr.cpp src/ars_table.cpp src/info_set.cpp src/lbr.cpp
TEST_SRCS = tests/tests.cpp src/game_state.cpp src/cfr.cpp src/ars_table.cpp src/info_set.cpp src/lbr.cpp
MAIN = main
TEST = test_executable
DEBUG = main_debug
PROFILE = main_profile
QUICK = main_quick

all: $(MAIN)

$(MAIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(DEBUG): $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^ $(LIBS)

$(PROFILE): $(SRCS)
	$(CXX) $(CXXFLAGS) $(PROFILEFLAGS) -o $@ $^ $(LIBS)

$(QUICK): $(SRCS)
	$(CXX) $(QUICKFLAGS) -o $@ $^ $(LIBS)

test: $(TEST)
	./$(TEST)

run: $(MAIN)
	./$(MAIN)

debug: $(DEBUG)
	lldb ./$(DEBUG)

profile: $(PROFILE)
	LLVM_PROFILE_FILE="$(PROFILE).profraw" ./$(PROFILE)
	xcrun llvm-profdata merge -sparse $(PROFILE).profraw -o $(PROFILE).profdata
	xcrun llvm-cov show ./$(PROFILE) -instr-profile=$(PROFILE).profdata > profile_output.txt
	@echo "Profiling complete. Results are in profile_output.txt"

quick: $(QUICK)
	./$(QUICK)

clean:
	rm -f $(MAIN) $(TEST) $(DEBUG) $(PROFILE) $(QUICK) *.profraw *.profdata gmon.out profile_output.txt *.jar *.dSYM