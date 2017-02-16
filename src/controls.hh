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
	
	void allocate_resources(unsigned int total_input_size, unsigned int total_output_size, unsigned char** h_input_buffer, unsigned char** h_output_buffer, unsigned char** d_input_buffer, unsigned char** d_output_buffer);
	void free_resources(unsigned int total_input_size, unsigned int total_output_size, unsigned char** h_input_buffer, unsigned char** h_output_buffer, unsigned char** d_input_buffer, unsigned char** d_output_buffer);
	
	//this function does the benchmark and prints out its results
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::ostream& output_stream, unsigned int input_size, unsigned int amount_inputs, unsigned char* input_buffer, unsigned char* output, unsigned char* d_input_buffer, unsigned char* d_output);
}

#endif
