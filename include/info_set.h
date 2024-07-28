#include "constants.h"

class InfoSet {

public:
	InfoSet(int flop_bucket,
			int turn_bucket,
			int rivr_bucket,
            uint8_t flop_history, 
            uint8_t turn_history, 
            uint8_t rivr_history,
            bool call_preflop);

	// Utility methods
	std::string to_string() const;
	bool operator==(const InfoSet& other) const;

	int flop_bucket;
	int turn_bucket;
	int rivr_bucket;
	uint8_t flop_history;
	uint8_t turn_history;
	uint8_t rivr_history;
	bool call_preflop;
};

size_t hash_info_set(const InfoSet& is);