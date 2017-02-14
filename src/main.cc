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
/*
	// functions to compare in benchmark
	std::unordered_map<std::string, std::tuple<std::function<void(unsigned char*, unsigned char**, size_t, unsigned char*, unsigned char**, unsigned int)>, std::function<void(const unsigned char*, const size_t, unsigned char*)>, std::function<void(unsigned char*, unsigned char*, size_t, unsigned char*, unsigned char*, unsigned int)>>> functions;
	// functions["OpenSSL"] = openssl::SHA1;
	functions["Own (CPU, first try)"] = {own_first_cpu_reference::sha1, own_first_cpu_reference::sha1, own_first_cpu_reference::sha1};
	functions["Own (GPU, first try)"] = {own_first_gpu_reference::sha1_prepare, own_first_gpu_reference::sha1, own_first_gpu_reference::sha1_cleanup};

	// inputs lengths in benchmark
	std::map<size_t, size_t> lengths;
	for (size_t i = 0; i < MAX_LEN; i += LEN_STEPS) {
		lengths[i] = TRIES_PER_LEN;
	}

	// start benchmark
	control_structure::benchmark_comparison("\t", "\n", functions, lengths, std::cout, std::chrono::duration<double>(0.1));
	*/

	// init
	unsigned char* h_input_buffer;
	unsigned char* h_output;
	unsigned char* d_input_buffer;
	unsigned char* d_output;
	size_t amount_inputs = TRIES_PER_LEN;
	srand(RAND_INIT);

	// print header output
	std::cout << std::setprecision(12) << std::fixed;
	std::cout << "Input\t\tCPU\t\t\t\tGPU preparation\t\t\tGPU running\t\t\tGPU cleaning\t\t\tGPU total\n";
	std::cout << "Size\tTries\tCPU\t\tWall\t\tCPU\t\tWall\t\tCPU\t\tWall\t\tCPU\t\tWall\t\tWall\n";

	// allocate resources
	control_structure::allocate_resources(amount_inputs * ((MAX_LEN + 72) & 0xFFFFFFC0), amount_inputs * 20, &h_input_buffer, &h_output, &d_input_buffer, &d_output);

	// iterate over input lengths, init input with random values and run benchmark for each
	for (size_t i = 32; i < MAX_LEN; i += 32) {
		const size_t input_size = (i + 72) & 0xFFFFFFC0;
		// h_input_buffer = (unsigned char*) realloc(h_input_buffer, sizeof(unsigned char) * input_size * amount_inputs);
		// output = (unsigned char*) realloc(output, sizeof(unsigned char) * 20 * amount_inputs);
		for (size_t j = 0; j < input_size * amount_inputs; ++j) {
			h_input_buffer[j] = ' ' + random() % 94;
			// h_input_buffer[j] = (unsigned char) (j / (i << 1)) + ' ';
		}
		control_structure::benchmark_comparison("\t", "\n", std::cout, i, amount_inputs, h_input_buffer, h_output, d_input_buffer, d_output);
	}
	
	control_structure::free_resources(amount_inputs * ((MAX_LEN + 72) & 0xFFFFFFC0), amount_inputs * 20, &h_input_buffer, &h_output, &d_input_buffer, &d_output);
}
