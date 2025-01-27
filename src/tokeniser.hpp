#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Tokeniser {

struct Error {
	enum class Type {
		NoLineNumber,
		BadLineNumber,
		LineTooLong,
		BadStringLiteral,
	};

	Type type;
	int line_number;

	std::string to_string() const {
		return [&]() -> std::string {
			switch(type) {
				case Type::NoLineNumber:		return "NoLineNumber";
				case Type::BadLineNumber:		return "BadLineNumber";
				case Type::LineTooLong:			return "LineTooLong";
				case Type::BadStringLiteral:	return "BadStringLiteral";
			}
			return "InvalidError";
		}() + " at line " + std::to_string(line_number);
	}
};

/// Returns a tokenised version of the textual BASIC program found in the input stream.
///
/// @param source A stream of text describing a BBC BASIC program.
/// @throws An instance of @c Error if any problem is encountered.
std::vector<uint8_t> import(FILE *source);

}