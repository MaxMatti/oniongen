#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
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

#include "helpers.hh"
#include "cpu1.hh"
#include "gpu1.hh"
#include "controls.hh"

#define RAND_INIT 1000
#define MAX_LEN 300
#define LEN_STEPS 32
#define TRIES_PER_LEN 1000000

int main() {
	// init
	unsigned char* h_input_buffer;
	unsigned char* h_output;
	unsigned char* d_input_buffer;
	unsigned char* d_output;
	unsigned int amount_inputs = TRIES_PER_LEN;
	srand(RAND_INIT);

	// allocate resources
	control_structure::allocate_resources(amount_inputs * ((MAX_LEN + 72) & 0xFFFFFFC0), amount_inputs * 20, &h_input_buffer, &h_output, &d_input_buffer, &d_output);

	// iterate over input lengths, init input with random values and run benchmark for each
	for (unsigned int i = LEN_STEPS; i < MAX_LEN; i += LEN_STEPS) {
		const unsigned int input_size = (i + 72) & 0xFFFFFFC0;
		// h_input_buffer = (unsigned char*) realloc(h_input_buffer, sizeof(unsigned char) * input_size * amount_inputs);
		// output = (unsigned char*) realloc(output, sizeof(unsigned char) * 20 * amount_inputs);
		for (unsigned int j = 0; j < input_size * amount_inputs; ++j) {
			h_input_buffer[j] = ' ' + random() % 94;
			// h_input_buffer[j] = (unsigned char) (j / (i << 1)) + '0';
		}
		control_structure::benchmark_comparison("\t", "\n", std::cout, i, amount_inputs, h_input_buffer, h_output, d_input_buffer, d_output);
	}
	
	control_structure::free_resources(amount_inputs * ((MAX_LEN + 72) & 0xFFFFFFC0), amount_inputs * 20, &h_input_buffer, &h_output, &d_input_buffer, &d_output);
}
