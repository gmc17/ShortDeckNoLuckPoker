#include "info_set.h"

InfoSet::InfoSet(): player(false),
                    cr1(0),
                    cr2(0),
                    fp1(0),
                    fp2(0),
                    fp3(0),
                    trn(0),
                    rvr(0),
                    pfp_history(0),
                    flp_history(0),
                    trn_history(0),
                    rvr_history(0),
                    num_actions(0) {}

std::string InfoSet::to_string() const {
    std::stringstream ss;
    return ss.str();
}

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t InfoSet::hash() const {
    size_t seed = 1234;
    
    if (fp3 == 0) hash_combine(seed, pocket_id(cr1, cr2));
    else { hash_combine(seed, cr1); hash_combine(seed, cr2); }
    
    hash_combine(seed, fp1);
    hash_combine(seed, fp2);
    hash_combine(seed, fp3);
    hash_combine(seed, trn);
    hash_combine(seed, rvr);
    hash_combine(seed, pfp_history);
    hash_combine(seed, flp_history);
    hash_combine(seed, trn_history);
    hash_combine(seed, rvr_history);

    return seed % STRATEGY_ARRAY_SIZE;
}