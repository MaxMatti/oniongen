#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <random>
#include <string>
#include <algorithm>

#include "helpers.hh"

namespace helpers {

	// calculates the ceiling of a division in a fast way.
	template <typename T> T fastCeil(T denominator, T divisor) {
		return (denominator + divisor - 1) / divisor;
	}

	// calculates the ceiling of a division in a fast way.
	unsigned int fastCeil(unsigned int denominator, unsigned int divisor) {
		return (denominator + divisor - 1) / divisor;
	}

	// returns a string with length random characters
	std::string getRandomStr(size_t length) {
		static auto randchar = []() -> char {
			return ' ' + rand() % 94;
		};
		std::string result(length, 0);
		std::generate_n(result.begin(), length, randchar);
		return result;
	}

	// fills input with a random string of length length, requires allocated space of length + 1
	void getRandomStr(size_t length, unsigned char* input) {
		static auto randchar = []() -> char {
			return ' ' + rand() % 94;
		};
		for (size_t i = 0; i < length; ++i) {
			input[i] = randchar();
		}
		input[length] = 0;
	}

	// converts byte-string to base16-string
	std::string base16(std::string input) {
		std::string result(fastCeil((size_t) input.size() * 8, (size_t) 4), 0); // 8 bytes in a "normal" string, 4 bytes in a base16-encoded (hex) string
		char characters[] = "0123456789abcdef";
		for (size_t i = 0; i < input.size(); ++i) {
			result[i * 2] = characters[(input[i] >> 4) & 15];
			result[i * 2 + 1] = characters[input[i] & 15];
		}
		return result;
	}

	// converts byte-string to base16-string
	std::string base16(const char* input) {
		std::string input_str(input);
		return base16(input_str);
	}

	// converts byte-string to base16-string
	std::string base16(const unsigned char* input) {
		std::string input_str(reinterpret_cast<const char*>(input));
		return base16(input_str);
	}

	// converts byte-string to base32-string
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

	// converts byte-string to base32-string
	std::string base32(std::string input) {
		return base32(input, '=');
	}

	// converts byte-string to base32-string
	std::string base32(const char* input) {
		std::string input_str(input);
		return base32(input_str, '=');
	}

	// converts byte-string to base32-string
	std::string base32(const unsigned char* input) {
		std::string input_str(reinterpret_cast<const char*>(input));
		return base32(input_str, '=');
	}
}

