#pragma once
#include <string>
#include <array>
#include <cstddef>
#include <unordered_map>
#include <chrono>
#include <random>
#define FIXED_FLOAT(x) std::fixed << std::setprecision(2) << (x)

inline std::mt19937& get_random_generator() {
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}
inline std::uniform_real_distribution<float>& get_uniform_distribution() {
    static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis;
}
inline std::uniform_int_distribution<> get_card_distribution() {
    static thread_local std::uniform_int_distribution<> dis(0, 35);
    return dis;
}
constexpr std::array<int, 36 * 36> generate_pocket_id_table() {
    std::array<int, 36 * 36> table{};
    for (int p1 = 0; p1 < 36; ++p1) {
        for (int p2 = 0; p2 < 36; ++p2) {
        	bool suited = ((p1/9)==(p2/9));
            table[p1 * 36 + p2] = (suited) ? std::max((p1%9)*9 + p2%9, (p2%9)*9 + p1%9)
                    					   : std::min((p1%9)*9 + p2%9, (p2%9)*9 + p1%9);
        }
    }
    return table;
}
inline constexpr auto POCKET_ID_TABLE = generate_pocket_id_table();
inline constexpr int pocket_id(int p1, int p2) noexcept {
    return POCKET_ID_TABLE[p1 * 36 + p2];
}

//****************************** CFR Parameters ******************************//
static const size_t STRATEGY_ARRAY_SIZE = 10000007;
static const float EPSILON = 0.05;
static const float TAU = 1000;
static const float BETA = 1000000;
static const uint8_t FLOP_BUCKETS = 20;
static const uint8_t TURN_BUCKETS = 10;
static const uint8_t RIVR_BUCKETS = 10;
static const std::array<float, FLOP_BUCKETS> FLOP_BUCKETS_ARR = {0.277263f, 0.318031f, 0.345064f, 0.366392f, 0.386034f, 0.406617f, 0.42208f, 0.440187f, 0.456866f, 0.470568f, 0.485192f, 0.506205f, 0.536178f, 0.571026f, 0.60727f, 0.642738f, 0.676258f, 0.717659f, 0.820755f, 1.0f};
static const std::array<float, TURN_BUCKETS> TURN_BUCKETS_ARR = {0.258352f, 0.309773f, 0.353553f, 0.40008f, 0.457418f, 0.538388f, 0.613087f, 0.696023f, 0.815798f, 1.0f};
static const std::array<float, RIVR_BUCKETS> RIVR_BUCKETS_ARR = {0.0969943f, 0.202133f, 0.310652f, 0.408409f, 0.503422f, 0.611624f, 0.705702f, 0.798761f, 0.952471f, 1.0f};

//****************************** Utility Constants ******************************//
static const size_t NUM_ROUNDS = 3;
static const size_t NUM_RANKS = 9900;
static const size_t NUM_POCKET_PAIRS = 81;
static const uint32_t STACK_SIZE = 100;
static const std::array<std::string,  4> SUIT_NAMES = {"♠", "♥", "♦", "♣"};
static const std::array<std::string, 13> CARD_NAMES = {"6", "7", "8", "9", "T", "J", "Q", "K", "A"};
static const std::array<int, 6> STRAIGHT_MASKS = {0b111110000, 0b011111000, 0b001111100, 0b000111110, 0b000011111, 0b100001111};
static const std::array<int, 9> SINGLE_MASKS = {0b100000000, 0b010000000, 0b001000000, 0b000100000, 0b000010000, 0b000001000, 0b000000100, 0b000000010, 0b000000001};
static const std::array<int, 9> OCTAL_MASKS = {0b111, 0b111000, 0b111000000, 0b111000000000, 0b111000000000000, 0b111000000000000000}; // currently allowing for only 3 bets per round
static const std::unordered_map<std::string, int> STRING_TO_ACTION = {{"fold", 0}, {"check", 0}, {"call", 1}, {"bet", 1}, {"raise", 2}};
static const std::array<float, 4> BET_SIZES = {0.5f, 0.75f, 1.0f, 2.0f};
static const std::array<float, 4> RAISE_SIZES = {2.0f, 2.75f, 3.5f, 5.0f};
static const std::array<std::string, 7> BET_ACTION_NAMES = {"Check", "Bet", "Bet", "Bet", "Bet", "All in", "Call"};
static const std::array<std::string, 7> RAISE_ACTION_NAMES = {"Fold", "Raise", "Raise", "Raise", "Raise", "All in", "Call"};