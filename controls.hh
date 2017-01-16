#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <random>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <cstdint>
#include <climits>
#include <functional>
#include <algorithm>
#include <chrono>
#include <tuple>

#ifndef __CONTROLS_HH__
#define __CONTROLS_HH__

namespace control_structure {
	//this function does the benchmark and prints out its results
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::ostream& output_stream, unsigned char* input_buffer, size_t input_size, size_t amount_inputs, unsigned char* output);
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::unordered_map<std::string, std::function<void(const unsigned char*, const size_t, unsigned char*)>> hash_functions, std::map<size_t, size_t> input_lengths, std::ostream& output_stream, std::chrono::duration<double> time_penalty);
}

#endif
