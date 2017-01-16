#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cuda_runtime_api.h>
#include <cuda.h>

#ifndef __GPU1_HH__
#define __GPU1_HH__

#define CUDA_CHECK(cmd) cuda_check(cmd, __FILE__, __LINE__, 0)
#define CUDA_CHECK_FATAL(cmd) cuda_check(cmd, __FILE__, __LINE__, 1)
#define CUDA_CHECK_KERNEL(fatal) cuda_check(cudaGetLastError(), __FILE__, __LINE__, fatal)

void cuda_check(cudaError_t error, const char* file, int line, int fatal);

namespace own_first_gpu_reference {
	// swaps the endian of any datatype. Source: http://stackoverflow.com/a/4956493
	__device__ template <typename T> T swap_endian(T u);

	// implementation of the s function as described in section 3. of spec
	__device__ std::uint32_t sha1_helper_s(std::uint32_t input, unsigned char offset);

	// implementation of the f function as described in section 5. of spec
	__device__ std::uint32_t sha1_helper_f(unsigned char nr, const std::uint32_t& b, const std::uint32_t& c, const std::uint32_t& d);

	// representation of the K variables as described in section 5. of spec
	__device__ std::uint32_t sha1_helper_K(unsigned char nr);

	// calculates the sha1 sum
	__global__ void d_sha1(const unsigned char* input_buffer, size_t input_buffer_size, unsigned char* output, unsigned int threads);
	
	void sha1_prepare(unsigned char* h_input_buffer, unsigned char** d_input_buffer, size_t input_buffer_size, unsigned char* h_output, unsigned char** d_output, unsigned int threads);
	
	void sha1(const unsigned char* input_buffer, size_t input_buffer_size, unsigned char* output, unsigned int threads);
	
	void sha1_cleanup(unsigned char* h_input_buffer, unsigned char* d_input_buffer, size_t input_buffer_size, unsigned char* h_output, unsigned char* d_output, unsigned int threads);
}

#endif
