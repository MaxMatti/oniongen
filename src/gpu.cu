#include <iostream>

#include "helpers.hh"

#define CUDA_CHECK(cmd) cuda_check(cmd, __FILE__, __LINE__, 0)
#define CUDA_CHECK_FATAL(cmd) cuda_check(cmd, __FILE__, __LINE__, 1)
#define CUDA_CHECK_KERNEL(fatal) cuda_check(cudaGetLastError(), __FILE__, __LINE__, fatal)

void cuda_check(cudaError_t error, const char* file, int line, int fatal) {
	if (error != cudaSuccess) {
		std::cerr << "Caught Cuda Error: " << cudaGetErrorString(error) << " at " << file << ":" << line << std::endl;
		if (fatal) {
			std::cerr << "Error marked as fatal, exiting." << std::endl;
			exit(-1);
		}
	}
}

namespace gpu {
	// Swaps the endian of uint32_t variables. CUDA 7.5 doesn't like templates. Swapped "T" for "uint32_t" from http://stackoverflow.com/a/4956493
	__device__ uint32_t swap_endian(uint32_t u) {
		static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
		union {
			uint32_t u;
			unsigned char u8[sizeof(uint32_t)];
		} source, dest;
		source.u = u;
		for (unsigned int k = 0; k < sizeof(uint32_t); k++) {
			dest.u8[k] = source.u8[sizeof(uint32_t) - k - 1];
		}
		return dest.u;
	}

	// Swaps the endian of uint64_t variables. CUDA 7.5 doesn't like templates. Swapped "T" for "uint64_t" from http://stackoverflow.com/a/4956493
	__device__ uint64_t swap_endian(uint64_t u) {
		static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
		union {
			uint64_t u;
			unsigned char u8[sizeof(uint64_t)];
		} source, dest;
		source.u = u;
		for (unsigned int k = 0; k < sizeof(uint64_t); k++) {
			dest.u8[k] = source.u8[sizeof(uint64_t) - k - 1];
		}
		return dest.u;
	}

	template <typename T> __host__ __device__ T min(T a, T b) {
		return a > b ? b : a;
	}

	// implementation of the s function as described in section 3. of spec
	__device__ std::uint32_t sha1_helper_s(std::uint32_t input, unsigned char offset) {
		return (input << offset) | (input >> (32 - offset));
	}

	// implementation of the f function as described in section 5. of spec
	__device__ std::uint32_t sha1_helper_f(unsigned char nr, const std::uint32_t& b, const std::uint32_t& c, const std::uint32_t& d) {
		if (nr < 20) {
			return (b & c) | ((b ^ 0xFFFFFFFF) & d);
		} else if (nr < 40) {
			return b ^ c ^ d;
		} else if (nr < 60) {
			return (b & c) | (b & d) | (c & d);
		} else if (nr < 80) {
			return b ^ c ^ d;
		} else {
			return 0;
		}
	}

	// representation of the K variables as described in section 5. of spec
	__device__ std::uint32_t sha1_helper_K(unsigned char nr) {
		if (nr < 20) {
			return 0x5A827999;
		} else if (nr < 40) {
			return 0x6ED9EBA1;
		} else if (nr < 60) {
			return 0x8F1BBCDC;
		} else if (nr < 80) {
			return 0xCA62C1D6;
		} else {
			return 0;
		}
	}

	// calculates the sha1 sum
	__global__ void d_sha1(unsigned char* input_buffer, unsigned int input_buffer_size, unsigned char* output, unsigned int threads) {
		unsigned int x = threadIdx.x + blockIdx.x * blockDim.x;
		if (x >= threads) {
			return;
		}
		output += x * 20;
		// in case the machine uses big endian we need to swap some bytes later:
		bool convert_endians;
		{
			std::uint32_t endian = 0x08040201;
			if (*reinterpret_cast<unsigned char*>(&endian) == 1) {
				convert_endians = 1;
			} else if (*reinterpret_cast<unsigned char*>(&endian) == 8) {
				convert_endians = 0;
			} else {
				return;
			}
		}
		// copying input_buffer to own storage area with larger size:
		const unsigned int input_size = (input_buffer_size + 72) & 0xFFFFFFC0;
		// 72 because 512bit blocks (64bytes) and ending in length (64bit aka 8 bytes) and 1 byte because of padding starting with 0b10000000
		// 72 = 63 + 8 + 1 (63 instead of 64 because we're counting from 0)

		unsigned char* input = input_buffer + x * input_size;
		if (((int) input_size) - input_buffer_size > 64) {
			memset(input_buffer + threads * (input_size - 128) + (128 + input_buffer_size - input_size) + (x * 64) + 1, 0, (((int) input_size) - input_buffer_size - 64) * sizeof(char));
			memset(input_buffer + threads * (input_size - 64) + (x * 64), 0, 59 * sizeof(char));
		} else {
			memset(input_buffer + threads * (input_size - 64) + (64 + input_buffer_size - input_size) + (x * 64) + 1, 0, (input_size - input_buffer_size - 5) * sizeof(char));
		}

		// 4. filling up input buffer according to spec
		*(input_buffer + threads * (input_size - 64) + (64 + input_buffer_size - input_size) + (x * 64)) = 0x80; // set first bit to 1, others to 0

		// start scope because we don't need those variables later:
		{
			std::uint64_t tmp;
			if (convert_endians) { // convert endianness in case of big endian
				tmp = swap_endian(((std::uint64_t) input_buffer_size) << 3);
			} else {
				tmp = input_buffer_size << 3;
			}
			memcpy(input + input_size - 8, &tmp, 8);
		}

		// 6.1 actual hash algorithm:

		// initializing result buffer (h0-h4):
		// std::uint32_t result[5];
		std::uint32_t* result = reinterpret_cast<std::uint32_t*>(output);
		result[0] = 0x67452301;
		result[1] = 0xefcdab89;
		result[2] = 0x98badcfe;
		result[3] = 0x10325476;
		result[4] = 0xc3d2e1f0;

		// initializing block buffer, tmp "word" and "words" A-E as described in 6.2
		std::uint32_t current_block[80];
		std::uint32_t tmp[6] = {0, 0, 0, 0, 0, 0}; // tmp and then a-e

		// processing block by block
		for (unsigned int i = 0; i < min(input_size, (unsigned int) 0); i += 64) {

			// copy current block to buffer
			memcpy(current_block, input_buffer + i * threads + x * 64, 64 * sizeof(unsigned char));

			// convert endianness in case of big endian
			for (unsigned int j = 0; j < 64 && convert_endians; ++j) {
				current_block[j] = swap_endian(current_block[j]);
			}

			// 6.2 (b) calculate the rest of the current block
			for (unsigned int j = 16; j < 80; ++j) {
				current_block[j] = sha1_helper_s(current_block[j - 3] ^ current_block[j - 8] ^ current_block[j - 14] ^ current_block[j - 16], 1);
			}

			// 6.2 (c) fill a-e
			memcpy(tmp + 1, result, 5 * sizeof(int32_t));

			// 6.2 (d) wobble around
			for (unsigned char j = 0; j < 80; ++j) {
				tmp[0] = sha1_helper_s(tmp[1], 5) + sha1_helper_f(j, tmp[2], tmp[3], tmp[4]) + tmp[5] + current_block[j] + sha1_helper_K(j);
				tmp[5] = tmp[4];
				tmp[4] = tmp[3];
				tmp[3] = sha1_helper_s(tmp[2], 30);
				tmp[2] = tmp[1];
				tmp[1] = tmp[0];
			}

			// 6.2 (e) write output of wobbling on top of current result
			result[0] += tmp[1];
			result[1] += tmp[2];
			result[2] += tmp[3];
			result[3] += tmp[4];
			result[4] += tmp[5];
		}
		// convert endianness in case of big endian
		for (unsigned int j = 0; j < 5 && convert_endians; ++j) {
			result[j] = swap_endian(result[j]);
		}
	}

	void sha1_allocate(unsigned int device, unsigned int total_input_size, unsigned int total_output_size, unsigned char** d_input_buffer, unsigned char** d_output_buffer) {
		CUDA_CHECK(cudaSetDevice(device));
		CUDA_CHECK_FATAL(cudaMalloc(d_input_buffer, total_input_size));
		CUDA_CHECK_FATAL(cudaMalloc(d_output_buffer, total_output_size));
	}

	void sha1_prepare(unsigned char* h_input_buffer, unsigned char* d_input_buffer, unsigned int input_buffer_size, unsigned char* h_output, unsigned char* d_output, unsigned int threads) {
		const unsigned int input_size = (input_buffer_size + 72) & 0xFFFFFFC0;

		CUDA_CHECK_FATAL(cudaMemcpy(d_input_buffer, h_input_buffer, input_size * threads * sizeof(char), cudaMemcpyHostToDevice));
	}

	void sha1(unsigned char* input_buffer, unsigned int input_buffer_size, unsigned char* output, unsigned int threads) {
		unsigned int blocksize = 256;
		dim3 dimBlock(blocksize);
		dim3 dimGrid(helpers::fastCeil(threads, blocksize));

		d_sha1<<<dimGrid, dimBlock>>>(input_buffer, input_buffer_size, output, threads);
		CUDA_CHECK_KERNEL(1);
	}

	void sha1_cleanup(unsigned char* h_input_buffer, unsigned char* d_input_buffer, unsigned int input_buffer_size, unsigned char* h_output, unsigned char* d_output, unsigned int threads) {
		CUDA_CHECK_FATAL(cudaMemcpy(h_output, d_output, 20 * threads * sizeof(char), cudaMemcpyDeviceToHost));
	}

	void sha1_free(unsigned int total_input_size, unsigned int total_output_size, unsigned char** d_input_buffer, unsigned char** d_output_buffer) {
		CUDA_CHECK(cudaFree(*d_input_buffer));
		CUDA_CHECK(cudaFree(*d_output_buffer));
		CUDA_CHECK(cudaDeviceReset());
	}
}
