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
#define MAX_LEN 1000
#define LEN_STEPS 32
#define TRIES_PER_LEN 100

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
	
	unsigned char* input_buffer;
	unsigned char* output;
	size_t amount_inputs = 256;
	
	srand(RAND_INIT);
	std::cout << std::setprecision(5) << std::fixed;
	std::cout << "CPU\t\tGPU preparation\tGPU running\tGPU cleaning\n";
	std::cout << "CPU\tWall\tCPU\tWall\tCPU\tWall\tCPU\tWall\n";
	for (size_t i = 32; i < 200; i += 32) {
		input_buffer = (unsigned char*) malloc(sizeof(unsigned char) * i * amount_inputs);
		output = (unsigned char*) malloc(sizeof(unsigned char) * 20 * amount_inputs);
		for (size_t j = 0; j < i; ++j) {
			input_buffer[j] = ' ' + random() % 94;
		}
		control_structure::benchmark_comparison("\t", "\n", std::cout, input_buffer, i, amount_inputs, output);
	}
}
