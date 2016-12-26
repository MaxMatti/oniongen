#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <string>
#include <cstring>
#include <cstdint>
#include <climits>
#include <functional>
#include <algorithm>

#include <openssl/sha.h>

#define RAND_INIT 1000
#define MAX_LEN 100000
#define LEN_STEPS 32
#define TRIES_PER_LEN 1000

template <typename T> T fastCeil(T denominator, T divisor) {
	return (denominator + divisor - 1) / divisor;
}

template <typename T> T swap_endian(T u) {
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union {
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++) {
		dest.u8[k] = source.u8[sizeof(T) - k - 1];
	}

	return dest.u;
}

std::string getRandomStr(size_t length) {
	static auto randchar = []() -> char {
		static const char charset[] = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" ",.-;:_<>|äöüßÄÖÜ#+'*`´^°!\"§$%&/()=?\\{[]}";
		return charset[rand() % sizeof(charset)];
	};
	std::string result(length, 0);
	std::generate_n(result.begin(), length, randchar);
	return result;
}

std::string base16(std::string input) {
	std::string result(fastCeil((size_t) input.size() * 8, (size_t) 4), 0); // 8 bytes in a "normal" string, 4 bytes in a base16-encoded (hex) string
	char characters[] = "0123456789abcdef";
	for (size_t i = 0; i < input.size(); ++i) {
		result[i * 2] = characters[(input[i] >> 4) & 15];
		result[i * 2 + 1] = characters[input[i] & 15];
	}
	return result;
}

std::string base16(const char* input) {
	std::string input_str(input, 20);
	return base16(input_str);
}

std::string base16(const unsigned char* input) {
	std::string input_str(reinterpret_cast<const char*>(input), 20);
	return base16(input_str);
}

std::string base32(std::string input, char padding) {
	size_t final_output_size = fastCeil((size_t) input.size() * 8, (size_t) 5);
	input.append(5 - input.size() % 5, 0);
	size_t estimated_output_size = input.size() * 8 / 5;
	std::string result(estimated_output_size, 0); // 8 bytes in a "normal" string, 5 bytes in a base32-encoded string
	char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ23456789";
	size_t output_pos = 0;
	for (size_t i = 0; i < input.size(); i += 5, output_pos += 8) {
		result[output_pos    ] = characters[(input[i] >> 3) & 31]; // 31 base10 = 11111 base2
		result[output_pos + 1] = characters[((input[i] << 2) & 28) | ((input[i + 1] >> 6) & 3)]; // 28 base10 = 11100 base2, 3 base10 = 11 base2
		result[output_pos + 2] = characters[(input[i + 1] >> 1) & 31]; // 31 base10 = 11111 base2
		result[output_pos + 3] = characters[((input[i + 1] << 4) & 16) | ((input[i + 2] >> 4) & 15)]; // 16 base10 = 10000 base2, 15 base10 = 01111 base2
		result[output_pos + 4] = characters[((input[i + 2] << 1) & 30) | ((input[i + 3] >> 7) & 1)]; // 30 base10 = 11110 base2, 1 base10 = 00001 base2
		result[output_pos + 5] = characters[(input[i + 3] >> 2) & 31]; // 31 base10 = 11111 base2
		result[output_pos + 6] = characters[((input[i + 3] << 3) & 24) | ((input[i + 4] >> 5) & 7)]; // 24 base10 = 11000 base2, 7 base10 = 00111 base2
		result[output_pos + 7] = characters[input[i + 4] & 31]; // 31 base10 = 11111 base2
	}
	result.resize(final_output_size);
	result.append(8 - result.size() % 8, padding);
	return result;
}

std::string base32(std::string input) {
	return base32(input, '=');
}

std::string base32(const char* input) {
	std::string input_str(input);
	return base32(input_str, '=');
}

std::string base32(const unsigned char* input) {
	std::string input_str(reinterpret_cast<const char*>(input));
	return base32(input_str, '=');
}

// implementation of the s function as described in section 3. of spec
std::uint32_t sha1_helper_s(std::uint32_t input, unsigned char offset) {
	return (input << offset) | (input >> (32 - offset));
}

// implementation of the f function as described in section 5. of spec
std::uint32_t sha1_helper_f(unsigned char nr, const std::uint32_t& b, const std::uint32_t& c, const std::uint32_t& d) {
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
std::uint32_t sha1_helper_K(unsigned char nr) {
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

// calculates the sha1 sum, allocates 160bit (aka 20 byte) buffer for result (that needs to be freed!)
// returns nullptr on failure.
unsigned char* sha1(const char* input_buffer, const size_t& input_buffer_size) {
	// in case the machine uses big endian we need to swap some bytes later:
	bool convert_endians;
	{
		std::uint32_t endian = 0x08040201;
		if (*reinterpret_cast<unsigned char*>(&endian) == 1) {
			convert_endians = 1;
		} else if (*reinterpret_cast<unsigned char*>(&endian) == 8) {
			convert_endians = 0;
		} else {
			return nullptr;
		}
	}
	// copying input_buffer to own storage area with larger size:
	const size_t input_size = (input_buffer_size + 72) & 0xFFFFFFC0;
	// 73 because 512bit blocks (64bytes) and ending in length (64bit aka 8 bytes) and 1 byte because of padding starting with 0b10000000
	// 73 = 64 + 8 + 1
	// but for some reason when using 73 some results differ from OpenSSLs implementation, fixed by using 72. TODO: investigate this.

	// then applying floor function
	unsigned char* input = (unsigned char*) calloc(input_size, sizeof(unsigned char));
	if (!input) {
		return input;
	} // return nullptr if calloc failed.
	std::memcpy(input, input_buffer, input_buffer_size);
	
	// 4. filling up input buffer according to spec
	*(input + input_buffer_size) = 0x80; // set first bit to 1, others to 0
	{
		std::uint64_t tmp;
		if (convert_endians) { // convert endianness in case of big endian
			tmp = swap_endian<std::uint64_t>(input_buffer_size << 3);
		} else {
			tmp = input_buffer_size << 3;
		}
		memcpy(input + input_size - 8, &tmp, 8);
		// These are to check wether the input string was corrupted:
		// std::string a(reinterpret_cast<char*>(input), input_size);
		// std::cerr << base16(a) << std::endl;
	}
	
	// 6.1 actual hash algorithm:
	
	// initializing result buffer (h0-h4):
	std::uint32_t* result = (std::uint32_t*) malloc(5 * sizeof(uint32_t));
	result[0] = 0x67452301;
	result[1] = 0xefcdab89;
	result[2] = 0x98badcfe;
	result[3] = 0x10325476;
	result[4] = 0xc3d2e1f0;
	
	// initializing block buffer, tmp "word" and "words" A-E as described in 6.2
	std::uint32_t* current_block = (std::uint32_t*) calloc(80, sizeof(std::uint32_t));
	if (!current_block) {
		return reinterpret_cast<unsigned char*>(current_block);
	} // return nullptr if calloc failed.
	std::uint32_t tmp[] = {0, 0, 0, 0, 0, 0}; // tmp and then a-e
	
	// processing block by block
	for (size_t i = 0; i < input_size; i += 64) {
		
		// copy current block to buffer
		memcpy(current_block, input + i, 64);

		// convert endianness in case of big endian
		for (size_t j = 0; j < 64 && convert_endians; ++j) {
			current_block[j] = swap_endian<std::uint32_t>(current_block[j]);
		}
		
		// 6.2 (b) calculate the rest of the current block
		for (size_t j = 16; j < 80; ++j) {
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

		// 6.2 (e) wobble around a little bit more
		result[0] += tmp[1];
		result[1] += tmp[2];
		result[2] += tmp[3];
		result[3] += tmp[4];
		result[4] += tmp[5];
	}
	// convert endianness in case of big endian
	for (size_t j = 0; j < 5 && convert_endians; ++j) {
		result[j] = swap_endian<std::uint32_t>(result[j]);
	}
	// free memory
	free(input);
	free(current_block);
	return reinterpret_cast<unsigned char*>(result);
}

std::string sha1(const std::string& input) {
	unsigned char* ptr = sha1(input.c_str(), input.size());
	std::string result(reinterpret_cast<char*>(ptr), 20);
	free(ptr);
	return result;
}

int main() {
	srand(RAND_INIT);
	std::string tmp;
	unsigned char hash[20];
	unsigned char* own_hash;
	for (size_t i = 0; i < MAX_LEN; i += LEN_STEPS) {
		tmp = getRandomStr(i);
		// Time measuring code comes from http://stackoverflow.com/a/17440673
		clock_t openssl_time = clock();
		for (size_t j = 0; j < TRIES_PER_LEN; ++j) {
			SHA1(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.size(), hash);
		}
		openssl_time = clock() - openssl_time;
		clock_t own_time = clock();
		for (size_t j = 0; j < TRIES_PER_LEN; ++j) {
			own_hash = sha1(tmp.c_str(), tmp.size());
			free(own_hash);
		}
		own_time = clock() - own_time;
		// std::cout << "Time for length " << i << ":\nOpenSSL: " << ((float) openssl_time) / CLOCKS_PER_SEC << ", Own: " << ((float) own_time) / CLOCKS_PER_SEC << std::endl;
		std::cout << i << "\t" << ((float) openssl_time) / CLOCKS_PER_SEC << "\t" << ((float) own_time) / CLOCKS_PER_SEC << std::endl;
		if (i % (20 * LEN_STEPS) == 0) {
			std::cerr << i << "\r";
		}
		/*if (memcmp(hash, own_hash, 20) != 0) {
			std::cerr << "Hash error at length " << i << ": \"" << base16(hash) << "\" - \"" << base16(own_hash) << "\": \"" << base16(tmp) << "\"" << std::endl;
		}*/
	}
}
