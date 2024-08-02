#define CATCH_CONFIG_MAIN
#include <gtest/gtest.h>
#include "game_state.h"
#include "constants.h"

/********************* HELPERS *********************/

const double TOL = 0.01;

int within(double a, double b)
{
  return fabs(a-b)<=TOL;
}

/********************** TESTS **********************/

TEST(ith_action, all_in_call) {
    uint32_t pflp_history = 0b110111;

    ASSERT_EQ(ith_action(pflp_history, 1), 6);
    ASSERT_EQ(ith_action(pflp_history, 0), 7);
}

TEST(pot_size, preflop_all_in_call) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(5);
    gs.apply_index(6);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);

    ASSERT_EQ(gs.pot_size, STACK_SIZE * 2);
}

TEST(pot_size, preflop_call_check) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(0);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);

    ASSERT_EQ(gs.pot_size, 4.0f);
}

TEST(pot_size, preflop_call_minraise_fold) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(3);
    gs.apply_index(0);

    ASSERT_EQ(gs.pot_size, 4.0f);
}

TEST(pot_size, preflop_call_minraise_call) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(1);
    gs.apply_index(6);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);

    ASSERT_EQ(gs.pot_size, 8.0f);
}

TEST(pot_size, pot_call_to_turn) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(1);
    gs.apply_index(6);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));
    gs.apply_chance_action(29);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));

    ASSERT_EQ(gs.pot_size, 72.0f);
}

TEST(pot_size, pot_call_to_river) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(1);
    gs.apply_index(6);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));
    gs.apply_chance_action(29);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));
    gs.apply_chance_action(28);
    gs.apply_index(gs.action_to_index(6)); // must go all-in by this point
    gs.apply_index(gs.action_to_index(7));

    ASSERT_EQ(gs.pot_size, 200.0f);
}

TEST(pot_size, pot_call_to_river02) {
    GameState gs = generate_random_initial_state();
    gs.apply_index(6);
    gs.apply_index(0);
    gs.apply_chance_action(32);
    gs.apply_chance_action(31);
    gs.apply_chance_action(30);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));
    gs.apply_chance_action(29);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));
    gs.apply_chance_action(28);
    gs.apply_index(gs.action_to_index(4));
    gs.apply_index(gs.action_to_index(7));

    ASSERT_EQ(gs.pot_size, 108.0f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}