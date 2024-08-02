#include "info_set.h"

InfoSet::InfoSet(): player(false),
                    pocket_id(0),
                    pflp_history(0),
                    flop_history(0),
                    turn_history(0),
                    rivr_history(0),
                    flop_seen(false),
                    turn_seen(false),
                    rivr_seen(false),
                    flop_bucket(0),
                    turn_bucket(0),
                    rivr_bucket(0),
                    num_actions(0) {}

std::string InfoSet::to_string() const {
    std::stringstream ss;

    std::bitset<32> binary_pflp_history(pflp_history);
    std::bitset<32> binary_flop_history(flop_history);
    std::bitset<32> binary_turn_history(turn_history);
    std::bitset<32> binary_rivr_history(rivr_history);

    if (!flop_seen) ss << "Pocket id: " << pocket_id << "\n";
    ss << "Pflp history: " << binary_pflp_history << "\n";
    if (flop_seen) ss << "Flop history: " << binary_flop_history << "\n";
    if (turn_seen) ss << "Turn history: " << binary_turn_history << "\n";
    if (rivr_seen) ss << "Rivr history: " << binary_rivr_history << "\n";
    if (flop_seen) ss << "Flop bucket:  " << flop_bucket << "\n";
    if (turn_seen) ss << "Turn bucket:  " << turn_bucket << "\n";
    if (rivr_seen) ss << "River bucket: " << rivr_bucket << "\n";
    ss << "Hash: " << hash() << "\n";
    
    return ss.str();
}

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t InfoSet::hash() const {
    size_t seed = 1234;
    
    if (!flop_seen) hash_combine(seed, pocket_id);
    hash_combine(seed, pflp_history);
    hash_combine(seed, flop_history);
    hash_combine(seed, turn_history << 1);
    hash_combine(seed, rivr_history << 2);
    hash_combine(seed, flop_seen << 1);
    hash_combine(seed, turn_seen << 2);
    hash_combine(seed, rivr_seen << 3);
    hash_combine(seed, flop_bucket);
    hash_combine(seed, turn_bucket << 4);
    hash_combine(seed, rivr_bucket << 8);

    return seed % STRATEGY_ARRAY_SIZE;
}