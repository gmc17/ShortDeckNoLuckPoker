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
	uint8_t cr1;
	uint8_t cr2;
	uint8_t fp1;
	uint8_t fp2;
	uint8_t fp3;
	uint8_t trn;
	uint8_t rvr;
	uint32_t pfp_history;
	uint32_t flp_history;
	uint32_t trn_history;
	uint32_t rvr_history;
	int num_actions;
};