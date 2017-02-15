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

	void allocate_resources(unsigned int total_input_size, unsigned int total_output_size, unsigned char** h_input_buffer, unsigned char** h_output_buffer, unsigned char** d_input_buffer, unsigned char** d_output_buffer) {
		*h_input_buffer = (unsigned char*) malloc(total_input_size);
		*h_output_buffer = (unsigned char*) malloc(total_output_size);
		own_first_gpu_reference::sha1_allocate(total_input_size, total_output_size, d_input_buffer, d_output_buffer);
	}

	void free_resources(unsigned int total_input_size, unsigned int total_output_size, unsigned char** h_input_buffer, unsigned char** h_output_buffer, unsigned char** d_input_buffer, unsigned char** d_output_buffer) {
		free(*h_input_buffer);
		free(*h_output_buffer);
		own_first_gpu_reference::sha1_free(total_input_size, total_output_size, d_input_buffer, d_output_buffer);
	}

	//this function does the benchmark and prints out its results
	void benchmark_comparison(std::string value_separator, std::string line_separator, std::ostream& output_stream, unsigned int input_buffer_size, unsigned int amount_inputs, unsigned char* input_buffer, unsigned char* output, unsigned char* d_input_buffer, unsigned char* d_output) {
		std::cout << "\n\n";
		const unsigned int input_size = (input_buffer_size + 72) & 0xFFFFFFC0;
		value_separator = "\t";
		
		// output_stream << input_buffer_size << value_separator;
		// output_stream << amount_inputs << value_separator;

		// initializing variables for benchmarks
		std::chrono::time_point<std::chrono::system_clock> start_wall_time;
		std::clock_t start_cpu_time;
		std::chrono::time_point<std::chrono::system_clock> stop_wall_time;
		std::clock_t stop_cpu_time;
		double total_wall_time = 0;

		// initializing memory for CPU implementation
		unsigned char* h_output = (unsigned char*) malloc(sizeof(unsigned char) * 20 * amount_inputs);
		unsigned char* tmp_input = (unsigned char*) malloc(sizeof(unsigned char) * input_size);

		// "starting" stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// initializing memory for CPU implementation and running benchmark for own CPU implementation for comparison
		for (unsigned int i = 0; i < amount_inputs; ++i) {
			//std::cerr << "C";
			memset(tmp_input, 0, sizeof(unsigned char) * input_size);
			//std::cerr << "D";
			for (unsigned int j = 0; j < input_size; j += 64) {
				//std::cerr << "E" << j;
				memcpy(tmp_input + j, input_buffer + j * amount_inputs + 64 * i, 64 * sizeof(unsigned char));
			}
			//std::cerr << "F";
			own_first_cpu_reference::sha1(tmp_input, input_buffer_size, h_output + 20 * i);
		}

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();

		// printing results
		// output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		// output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;

		// "starting" stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// init gpu memory and copy stuff to gpu
		own_first_gpu_reference::sha1_prepare(input_buffer, d_input_buffer, input_buffer_size, output, d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		// output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		// output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;
		total_wall_time += ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count();

		// "starting" stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// run kernels
		own_first_gpu_reference::sha1(d_input_buffer, input_buffer_size, d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		// output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		// output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;
		total_wall_time += ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count();

		// starting stopwatch
		start_wall_time = std::chrono::system_clock::now();
		start_cpu_time = std::clock();

		// copy back from and free gpu memory
		own_first_gpu_reference::sha1_cleanup(input_buffer, d_input_buffer, input_buffer_size, output, d_output, amount_inputs);

		// "stopping" stopwatch
		stop_wall_time = std::chrono::system_clock::now();
		stop_cpu_time = std::clock();
		
		// printing results
		// output_stream << ((double) stop_cpu_time - start_cpu_time) / CLOCKS_PER_SEC << value_separator;
		// output_stream << ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count() << value_separator;
		total_wall_time += ((std::chrono::duration<double>) (stop_wall_time - start_wall_time)).count();
		// output_stream << total_wall_time << line_separator;
		
		if (memcmp(output, h_output, sizeof(unsigned char) * 20 * amount_inputs)) {
			std::cerr << "Inconsistent output!\n";
			unsigned int errcount = 0;
			for (unsigned int i = 0; i < amount_inputs && errcount < 100; ++i) {
				if (memcmp(output + 20 * i, h_output + 20 * i, sizeof(unsigned char) * 20)) {
					std::cerr << i << ": ";
					for (unsigned int j = 0; j < input_size; j += 64) {
						std::cerr << std::string((const char*) (input_buffer + j * amount_inputs + 64 * i), std::min<unsigned int>(64, input_buffer_size - j));
					}
					std::cerr << " does " << helpers::base16(output + 20 * i, 20) << " vs " << helpers::base16(h_output + 20 * i, 20) << "\n";
					++errcount;
				}
			}
			exit(0);
		}
	}
}
