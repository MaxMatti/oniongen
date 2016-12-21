#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <string>
#include <cstring>
#include <functional>
#include <algorithm>

#define RAND_INIT 1000

template <typename T> T fastCeil(T denominator, T divisor) {
	return (denominator + divisor - 1) / divisor;
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

// implementation of the s function as described in section 3. of spec
int32_t sha1_helper_s(int32_t input, unsigned char offset) {
	return (input << offset) | (input >> (32 - offset));
}

// implementation of the f function as described in section 5. of spec
int32_t sha1_helper_f(unsigned char nr, const int32_t& b, const int32_t& c, const int32_t& d) {
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
int32_t sha1_helper_K(unsigned char nr) {
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

// calculates the sha1 sum, allocates 128bit (aka 16 byte) 0-terminated buffer for result (that needs to be freed!)
// returns nullptr on failure.
char* sha1(char* input_buffer, const size_t& input_buffer_size) {
	// copying input_buffer to own storage area with larger size:
	const size_t input_size = (input_size + 73) & 0xFFFFFFC0;
	// 73 because 512bit blocks (64bytes) and ending in length (64bit aka 8 bytes) and 1 byte because of padding starting with 0b10000000
	// 73 = 64 + 8 + 1
	// then applying floor function
	unsigned char* input = (unsigned char*) calloc(input_size, sizeof(unsigned char));
	if (!input) {
		return input;
	} // return nullptr if calloc failed.
	std::memcpy(input, input_buffer, input_buffer_size);
	
	// 4. filling up input buffer according to spec
	*(input + input_buffer_size) = 0x80; // set first bit to 1, others to 0
	{
		int64_t tmp = input_buffer_size << 3;
		memcpy(input + input_size - 8, &tmp, 8);
	}
	
	// 6.1 actual hash algorithm:
	
	// initializing result buffer (h0-h4):
	unsigned int32_t result[] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
	
	// initializing block buffer, tmp "word" and "words" A-E as described in 6.2
	unsigned int32_t* current_block = (unsigned int32_t*) calloc(80, sizeof(unsigned int32_t));
	if (!current_block) {
		return current_block;
	} // return nullptr if calloc failed.
	unsigned int32_t tmp[] = {0, 0, 0, 0, 0, 0}; // tmp and then a-e
	
	// processing block by block
	for (size_t i = 0; i < input_size; i += 64) {
		
		// copy current block to buffer
		memcpy(current_block, input + i, 64);
		
		// 6.2 (b) calculate the rest of the current block
		for (size_t j = 16; j < 80; ++j) {
			current_block[j] = sha1_helper_s(current_block[j - 3] ^ current_block[j - 8] ^ current_block[j - 14] ^ current_block[j - 16], 1);
		}
		
		// 6.2 (c) fill a-e
		memcpy(tmp + 1, result, 5 * sizeof(int32_t));
		
		// 6.2 (d) wobble around
		for (unsigned char j = 0; j < 80; ++j) {
			tmp[0] = sha1_helper_s(tmp[1], 5) + sha1_helper_f(j, tmp[2], tmp[3], tmp[4]) + tmp[5] + current_block[j] + sha1_helper_K(j);
			// TODO: next line in spec is "E = D;  D = C;  C = S^30(B);  B = A; A = TEMP;" at page 7 under 6.2 (d)
		}
	}
	
	return input;
}

std::string sha1(const std::string& input) {
	char* ptr = sha1(input.c_str(), input.size());
	std::string result(ptr, 20);
	free(ptr);
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
	std::cerr << input.size() << " - " << estimated_output_size << " - " << final_output_size << std::endl;
	result.resize(final_output_size);
	result.append(8 - result.size() % 8, padding);
	return result;
}

std::string base32(std::string input) {
	return base32(input, '=');
}

int main() {
	srand(RAND_INIT);
	std::string tmp;
	for (size_t i = 0; i < 20; ++i) {
		tmp = getRandomStr(i);
		std::cout << "\"" << base32(tmp) << "\": \"" << tmp << "\"" << std::endl;
	}
}
