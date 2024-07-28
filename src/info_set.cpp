#include "info_set.h"

InfoSet::InfoSet(int flop_bucket,
                 int turn_bucket,
                 int rivr_bucket,
                 uint8_t flop_history, 
                 uint8_t turn_history, 
                 uint8_t rivr_history,
                 bool call_preflop) 
                 :
                 flop_bucket(flop_bucket),
                 turn_bucket(turn_bucket),
                 rivr_bucket(rivr_bucket),
                 flop_history(flop_history),
                 turn_history(turn_history),
                 rivr_history(rivr_history),
                 call_preflop(call_preflop) {}

size_t hash_info_set(const InfoSet& is) {
    size_t seed = 0x9e3779b9;
    std::hash<int> hash_int;
    std::hash<uint8_t> hash_uint8;
    std::hash<bool> hash_bool;

    seed ^= hash_int(is.flop_bucket);
    seed ^= hash_int((is.turn_bucket)<<4);
    seed ^= hash_int((is.rivr_bucket)<<8);
    seed ^= hash_uint8(is.flop_history);
    seed ^= hash_uint8(is.turn_history) + (seed >> 2);
    seed ^= hash_uint8(is.rivr_history) + (seed >> 2);
    seed ^= hash_bool(is.call_preflop) + (seed >> 2);

    return seed % STRATEGY_ARRAY_SIZE;
}