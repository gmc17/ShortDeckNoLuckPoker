#pragma once
#include <string>
#include <bitset>
#include <sstream>
#include <cstdint>

#include "constants.h"

class InfoSet {

public:
	InfoSet();

	std::string to_string() const;
	bool operator==(const InfoSet& other) const;
	size_t hash() const;

	bool player;
	int pocket_id;
	uint32_t pflp_history;
	uint32_t flop_history;
	uint32_t turn_history;
	uint32_t rivr_history;
	bool flop_seen;
	bool turn_seen;
    bool rivr_seen;
	int flop_bucket;
	int turn_bucket;
	int rivr_bucket;
	int num_actions;
};