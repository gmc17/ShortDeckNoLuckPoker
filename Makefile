.PHONY: all clean test run debug profile quick train play generate-ars sanitize wasm

WASM = poker_solver.js
EMXX = emcc
EMXXFLAGS = --bind -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME="createPokerSolverModule" -I./include

CXX = g++
CXXFLAGS = -Wall -std=c++20 -I./include -Ofast -march=native -mtune=native -flto -ffast-math -funroll-loops -fno-rtti -finline-functions
DEBUGFLAGS = -g -O0 -DDEBUG
PROFILEFLAGS = -g -fprofile-instr-generate -fcoverage-mapping
SANITIZEFLAGS = -fsanitize=address -fno-omit-frame-pointer
SANITIZECXXFLAGS = -Wall -std=c++20 -I./include -O1 -g
LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread
SRCS = src/main.cpp src/game_state.cpp src/info_set.cpp src/best_response.cpp src/tree.cpp src/cfr.cpp src/helpers.cpp src/user_interface.cpp src/constants.cpp
TEST_SRCS = tests/tests.cpp src/main.cpp src/game_state.cpp src/info_set.cpp src/best_response.cpp src/tree.cpp src/cfr.cpp src/helpers.cpp src/user_interface.cpp src/constants.cpp
MAIN = main
TEST = test_executable
DEBUG = main_debug
PROFILE = main_profile
SANITIZE = main_sanitize
SHORTDECK = shortdeck

all: $(MAIN) $(SHORTDECK) $(SANITIZE)

$(MAIN): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(SHORTDECK): src/main.cpp src/game_state.cpp src/info_set.cpp src/cfr.cpp src/best_response.cpp src/tree.cpp src/helpers.cpp src/constants.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(TEST): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(DEBUG): $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^ $(LIBS)

$(PROFILE): $(SRCS)
	$(CXX) $(CXXFLAGS) $(PROFILEFLAGS) -o $@ $^ $(LIBS)

$(SANITIZE): $(SRCS)
	$(CXX) $(SANITIZECXXFLAGS) $(SANITIZEFLAGS) -o $@ $^ $(LIBS)

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

sanitize: $(SANITIZE)
	ASAN_OPTIONS=print_stats=1,detect_stack_use_after_return=1 \
	UBSAN_OPTIONS=print_stacktrace=1 \
	./$(SANITIZE)

quick: $(QUICK)
	./$(QUICK)

train: $(SHORTDECK)
	./$(SHORTDECK) train $(ITERATIONS)

play: $(SHORTDECK)
	./$(SHORTDECK) play

generate-ars: $(SHORTDECK)
	./$(SHORTDECK) generate-ars

exploit: $(SHORTDECK)
	./$(SHORTDECK) exploit $(HANDS)

clean:
	rm -f $(MAIN) $(TEST) $(DEBUG) $(PROFILE) $(SHORTDECK) $(SANITIZE) *.profraw *.profdata gmon.out profile_output.txt *.jar *.dSYM
	find . -name "*.o" -type f -delete
