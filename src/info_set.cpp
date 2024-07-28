#include "info_set.h"

InfoSet::InfoSet(int p,
                 bool call_preflop,
                 uint8_t flop_history, 
                 uint8_t turn_history, 
                 uint8_t rivr_history,
                 int flop_bucket,
                 int turn_bucket,
                 int rivr_bucket,
                 bool player) 
                 :
                 p(p),
                 call_preflop(call_preflop),
                 flop_history(flop_history),
                 turn_history(turn_history),
                 rivr_history(rivr_history),
                 flop_bucket(flop_bucket),
                 turn_bucket(turn_bucket),
                 rivr_bucket(rivr_bucket),
                 player(player) {}

std::string InfoSet::to_string() const {
    std::stringstream ss;

    std::bitset<8> binary_flop_history(flop_history);
    std::bitset<8> binary_turn_history(turn_history);
    std::bitset<8> binary_rivr_history(rivr_history);

    if (!call_preflop) ss << "p_id: " << p << "\n";
    ss << "Player: " << player << "\n";
    ss << "Call preflop: " << call_preflop << "\n";
    ss << "Flop history: " << binary_flop_history << "\n";
    ss << "Turn history: " << binary_turn_history << "\n";
    ss << "Rivr history: " << binary_rivr_history << "\n";
    ss << "Flop bucket:  " << flop_bucket << "\n";
    ss << "Turn bucket:  " << turn_bucket << "\n";
    ss << "River bucket: " << rivr_bucket << "\n";
    ss << "Hash: " << hash() << "\n";
    
    return ss.str();
}

int InfoSet::num_actions() const {
    
    /************************* Preflop *************************/
    if (call_preflop == 0 && 
        flop_history == 0) return 2;

    /************************* Flop *************************/
    if (flop_history == 0b1) return 2; // first flop action
    if (flop_history == 0b11) return 2; // check
    if (flop_history == 0b101) return 3; // bet
    if (flop_history == 0b10101) return 2; // bet raise
    if (flop_history == 0b10011) return 3; // check bet
    if (flop_history == 0b1010011) return 2; // check bet raise

    /************************* Turn *************************/
    if (turn_history == 0b1) return 2; // first turn action
    if (turn_history == 0b11) return 2; // check
    if (turn_history == 0b101) return 2; // bet
    if (turn_history == 0b10011) return 2; // check bet

    /************************* River *************************/
    if (rivr_history == 0b1) return 2; // first river action
    if (rivr_history == 0b11) return 2; // check
    if (rivr_history == 0b101) return 3; // bet
    if (rivr_history == 0b10101) return 2; // bet raise
    if (rivr_history == 0b10011) return 3; // check bet
    if (rivr_history == 0b1010011) return 2; // check bet raise

    std::ostringstream oss;
        oss << "Tried to find number of actions for non-action node. "
            << "flop_history: " << static_cast<int>(flop_history)
            << ", turn_history: " << static_cast<int>(turn_history)
            << ", rivr_history: " << static_cast<int>(rivr_history);

    throw std::invalid_argument(oss.str());
    
    return -1;
}

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t InfoSet::hash() const {
    size_t seed = 0;
    
    if (!call_preflop) {
        hash_combine(seed, p);
    }
    hash_combine(seed, flop_bucket);
    hash_combine(seed, turn_bucket << 4);
    hash_combine(seed, rivr_bucket << 8);
    hash_combine(seed, flop_history);
    hash_combine(seed, turn_history << 1);
    hash_combine(seed, rivr_history << 2);
    hash_combine(seed, call_preflop);

    return seed % STRATEGY_ARRAY_SIZE;
}