#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include "constants.h"

void read_range_file(const std::string& filename, float range[NUM_RANKS][NUM_RANKS]);