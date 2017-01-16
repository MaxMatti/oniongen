#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <random>
#include <string>

#ifndef __HELPERS_HH__
#define __HELPERS_HH__

namespace helpers {

	// calculates the ceiling of a division in a fast way.
	template <typename T> T fastCeil(T denominator, T divisor);
	unsigned int fastCeil(unsigned int denominator, unsigned int divisor);

	// returns a string with length random characters
	std::string getRandomStr(size_t length);

	// fills input with a random string of length length, requires allocated space of length + 1
	void getRandomStr(size_t length, unsigned char* input);

	// converts byte-string to base16-string
	std::string base16(std::string input);

	// converts byte-string to base16-string
	std::string base16(const char* input);

	// converts byte-string to base16-string
	std::string base16(const unsigned char* input);

	// converts byte-string to base32-string
	std::string base32(std::string input, char padding);

	// converts byte-string to base32-string
	std::string base32(std::string input);

	// converts byte-string to base32-string
	std::string base32(const char* input);

	// converts byte-string to base32-string
	std::string base32(const unsigned char* input);
}

#endif
