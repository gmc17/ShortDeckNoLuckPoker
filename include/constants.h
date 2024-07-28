#pragma once
#include <string>
#include <array>
#include <cstddef>

//****************************** CFR Parameters ******************************//
static const size_t STRATEGY_ARRAY_SIZE = 10000007;
static const int NUM_CHANCE_SAMPLES = 3;
static const float EPSILON = 0.05;
static const float TAU = 1000;
static const float BETA = 1000000;
static const uint8_t FLOP_BUCKETS = 12;
static const uint8_t TURN_BUCKETS = 12;
static const uint8_t RIVR_BUCKETS = 10;
static const std::array<float, FLOP_BUCKETS> FLOP_BUCKETS_arr = {0.2f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f, 0.65f, 0.7f, 0.8f, 0.9f};
static const std::array<float, TURN_BUCKETS> TURN_BUCKETS_arr = {0.2f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f, 0.65f, 0.7f, 0.8f, 0.9f};
static const std::array<float, RIVR_BUCKETS> RIVR_BUCKETS_arr = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};

//****************************** Utility Constants ******************************//
static const size_t NUM_ROUNDS = 3;
static const size_t NUM_RANKS = 9900;
static const size_t NUM_POCKET_PAIRS = 81;
static const std::array<std::string,  4> SUIT_NAMES = {"♠", "♥", "♦", "♣"};
static const std::array<std::string, 13> CARD_NAMES = {"6", "7", "8", "9",
                                              		   "T", "J", "Q", "K",
                                              		   "A"};
static const std::array<int, 6> STRAIGHT_MASKS = {0b111110000, 0b011111000, 0b001111100, 0b000111110, 0b000011111, 0b100001111};
static const std::array<int, 9> SINGLE_MASKS = {0b100000000, 0b010000000, 0b001000000, 0b000100000, 0b000010000, 0b000001000, 0b000000100, 0b000000010, 0b000000001};