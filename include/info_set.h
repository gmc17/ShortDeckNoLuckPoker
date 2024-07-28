#pragma once
#include <string>
#include <bitset>
#include <sstream>
#include <cstdint>

#include "constants.h"

class InfoSet {

public:
	InfoSet(int p,
			bool call_preflop,
			uint8_t flop_history, 
            uint8_t turn_history, 
            uint8_t rivr_history,
			int flop_bucket,
			int turn_bucket,
			int rivr_bucket,
			bool player);

	std::string to_string() const;
	bool operator==(const InfoSet& other) const;
	size_t hash() const;
	int num_actions() const;

	int p;
	bool call_preflop;
	uint8_t flop_history;
	uint8_t turn_history;
	uint8_t rivr_history;
	int flop_bucket;
	int turn_bucket;
	int rivr_bucket;
	bool player;
};