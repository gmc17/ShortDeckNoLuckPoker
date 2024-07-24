#define CATCH_CONFIG_MAIN
#include <gtest/gtest.h>
#include "game_state.h"

TEST(HandEvaluationTest, PotSizeTest01) {
    uint32_t suita = 0b00000000101000000000000000000110;
    uint32_t suitb = 0b00000000000000000000000000000000;
    uint32_t suitc = 0b00000000010000001000000000000000;
    uint32_t suitd = 0b00000000000000000010000000000000;
    uint8_t  turn  =                         0b00000000;
    uint8_t  rivr  =                         0b00000000;
    uint8_t  flop_history =                  0b00110010;
    uint8_t  turn_history =                  0b00110010;
    uint8_t  rivr_history =                  0b00001010;
    bool call_preflop = 0;
    bool player = 0;

    GameState gs(suita,
                 suitb,
                 suitc,
                 suitd,   
                 turn,
                 rivr,
                 flop_history,
                 turn_history,
                 rivr_history,
                 call_preflop,
                 player);

    int result = gs.utility(0);
    ASSERT_EQ(result, 90);
}

TEST(HandEvaluationTest, PotSizeTest02) {
    uint32_t suita = 0b00000000101000000000000000000110;
    uint32_t suitb = 0b00000000000000000000000000000000;
    uint32_t suitc = 0b00000000010000001000000000000000;
    uint32_t suitd = 0b00000000000000000010000000000000;
    uint8_t  turn  =                         0b00000000;
    uint8_t  rivr  =                         0b00000000;
    uint8_t  flop_history =                  0b00110010;
    uint8_t  turn_history =                  0b00110010;
    uint8_t  rivr_history =                  0b00001010;
    bool call_preflop = 0;
    bool player = 0;

    GameState gs(suita,
                 suitb,
                 suitc,
                 suitd,   
                 turn,
                 rivr,
                 flop_history,
                 turn_history,
                 rivr_history,
                 call_preflop,
                 player);

    int result = gs.utility(0);
    ASSERT_EQ(result, 90);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}