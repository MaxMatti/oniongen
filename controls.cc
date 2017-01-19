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

#include "controls.hh"
#include "cpu1.hh"
#include "gpu1.hh"
#include "helpers.hh"

#define RAND_INIT 1000

namespace control_structure {

	//this function does the benchmark and prints out its results
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::ostream& output_stream, unsigned char* input_buffer, size_t input_size, size_t amount_inputs, unsigned char* output) {
		value_separator = "\t";
		
		// initializing variables for benchmarks
		std::chrono::time_point<std::chrono::system_clock> start_wall_time;
		std::clock_t start_cpu_time;
		std::chrono::time_point<std::chrono::system_clock> stop_wall_time;
		std::clock_t stop_cpu_time;

		// starting stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		unsigned char* h_output = (unsigned char*) malloc(sizeof(unsigned char) * 20 * amount_inputs);

		// run benchmark for own CPU implementation for comparison
		for (size_t i = 0; i < amount_inputs; ++i) {
			own_first_cpu_reference::sha1(input_buffer + input_size * i, input_size, h_output + 20 * i);
		}

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;

		// starting stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		unsigned char* d_input_buffer;
		unsigned char* d_output;

		// init gpu memory and copy stuff to gpu
		own_first_gpu_reference::sha1_prepare(input_buffer, &d_input_buffer, input_size, output, &d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;

		// starting stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// run kernels
		own_first_gpu_reference::sha1(d_input_buffer, input_size, d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;

		// starting stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// copy back from and free gpu memory
		own_first_gpu_reference::sha1_cleanup(input_buffer, d_input_buffer, input_size, output, d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;

		output_stream << line_separator;
		
		if (memcmp(output, h_output, sizeof(unsigned char) * 20 * amount_inputs)) {
			std::cerr << "Inconsistent output!\n";
			for (size_t i = 0; i < amount_inputs; ++i) {
				if (memcmp(output + 20 * i, h_output + 20 * i, sizeof(unsigned char) * 20)) {
					std::cerr << std::string((const char*) (input_buffer + input_size * i), input_size) << " does " << helpers::base32(output + 20 * i, 20) << " vs " << helpers::base32(h_output + 20 * i, 20) << "\n";
				}
			}
			exit(0);
		}
	}

	//this function does the benchmark and prints out its results
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::unordered_map<std::string, std::function<void(const unsigned char*, const size_t, unsigned char*)>> hash_functions, std::map<size_t, size_t> input_lengths, std::ostream& output_stream, std::chrono::duration<double> time_penalty) {
		// init variables and values
		srand(RAND_INIT);
		unsigned char input[input_lengths.rbegin()->first + 1];
		unsigned char correct_output[21];
		unsigned char current_output[21];
		correct_output[20] = 0;
		current_output[20] = 0;
		std::chrono::time_point<std::chrono::system_clock> start_wall_time;
		std::clock_t start_cpu_time;
		std::chrono::time_point<std::chrono::system_clock> stop_wall_time;
		std::clock_t stop_cpu_time;
		size_t penalty_counter;

		// print header
		output_stream << "Input Size" << value_separator << "Tries" << value_separator;
		for (auto current_function : hash_functions) {
			output_stream << current_function.first << " CPU-Time" << value_separator << current_function.first << " Wall-Time" << value_separator;
		}
		output_stream << "hash" << value_separator << "input" << line_separator;

		// run through input lengths and print the results
		for (auto current_input_length : input_lengths) {

			// output beginning of line and generate random string
			output_stream << current_input_length.first << value_separator << current_input_length.second << value_separator;
			helpers::getRandomStr(current_input_length.first, input);

			// calculate reference for results
			hash_functions.begin()->second(input, current_input_length.first, correct_output);

			for (auto current_function : hash_functions) {
				// "starting" stopwatch
				penalty_counter = 0;
				start_wall_time = std::chrono::system_clock::now();
				start_cpu_time = std::clock();

				// starting actual benchmark
				for (size_t i = 0; i < current_input_length.second; ++i) {
					current_function.second(input, current_input_length.first, current_output);
					if (memcmp(current_output, correct_output, 20) != 0) {
						++penalty_counter;
					}
				}

				// "stopping" stopwatch
				stop_wall_time = std::chrono::system_clock::now();
				stop_cpu_time = std::clock();

				// printing results
				output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
				output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time) + time_penalty * penalty_counter).count() << value_separator;
			}
			//output_stream << helpers::base32(correct_output) << value_separator << helpers::base32(input) << line_separator;
			output_stream << line_separator;
		}
	}
}
