#pragma once

#include <vector>

namespace Tokeniser {

enum class Error {
	NoLineNumber,
	BadLineNumber,
	LineTooLong,
	BadStringLiteral,
};

std::vector<uint8_t> import(FILE *input);	// TODO: use C++ iostream here.

}