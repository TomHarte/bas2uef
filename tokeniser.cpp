#include "tokeniser.hpp"
#include "tokeniser.hpp"

#include <cctype>
#include <cstdio>
#include <map>

/*
	Implements parsing of BBC BASIC v2.

	Heavily based on the descriptions provided by Mark Plumbley in
	BASIC ROM User Guide, ISBN 0 947929 04 5, section 2.3.
*/


namespace {

template <typename NodeKeyT, typename ValueT>
struct Trie {
	Trie() = default;
	constexpr Trie(std::initializer_list<std::pair<const NodeKeyT *, ValueT>> values) noexcept {
		for(const auto &value: values) {
			insert(value.first, value.second);
		}
	}

	const Trie *find(NodeKeyT key) const {
		const auto it = children_.find(key);
		if(it == children_.end()) return nullptr;
		return &it->second;
	}

	const std::optional<ValueT> &value() const {
		return value_;
	}

private:
	constexpr void insert(const NodeKeyT *key, const ValueT &value) {
		if(*key) {
			children_[*key].insert(key + 1, value);
		} else {
			value_ = value;
		}
	}

	std::unordered_map<NodeKeyT, Trie> children_;
	std::optional<ValueT> value_;
};

enum Flags: uint8_t {
	PseudoVariable = 0x40,
	REM = 0x20,
	LineNumber = 0x10,
	FNProc = 0x08,
	Start = 0x04,
	Middle = 0x02,
	Conditional = 0x01
};
struct Keyword {
	uint8_t token = 0;
	uint8_t flags = 0;
};

const Trie<char, Keyword> tokens = {
	{"AND",			{0x80}},
	{"DIV",			{0x81}},
	{"EOR",			{0x82}},
	{"MOD",			{0x83}},
	{"OR",			{0x84}},
	{"ERROR",		{0x85, Start}},
	{"LINE",		{0x86}},
	{"OFF",			{0x87}},
	{"STEP",		{0x88}},
	{"SPC",			{0x89}},
	{"TAB(",		{0x8a}},
	{"ELSE",		{0x8b, LineNumber | Start}},
	{"THEN",		{0x8c, LineNumber | Start}},
	{"OPENIN",		{0x8e}},
	{"PTR",			{0x8f, PseudoVariable | Middle | Conditional}},
	{"PAGE",		{0x90, PseudoVariable | Middle | Conditional}},
	{"TIME",		{0x91, PseudoVariable | Middle | Conditional}},
	{"LOMEM",		{0x92, PseudoVariable | Middle | Conditional}},
	{"HIMEM",		{0x93, PseudoVariable | Middle | Conditional}},
	{"ABS",			{0x94}},
	{"ACS",			{0x95}},
	{"ADVAL",		{0x96}},
	{"ASC",			{0x97}},
	{"ASN",			{0x98}},
	{"ATN",			{0x99}},
	{"BGET",		{0x9a, Conditional}},
	{"COS",			{0x9b}},
	{"COUNT",		{0x9c, Conditional}},
	{"DEG",			{0x9d}},
	{"ERL",			{0x9e, Conditional}},
	{"ERR",			{0x9f, Conditional}},
	{"EVAL",		{0xa0}},
	{"EXP",			{0xa1}},
	{"EXT",			{0xa2, Conditional}},
	{"FALSE",		{0xa3, Conditional}},
	{"FN",			{0xa4, FNProc}},
	{"GET",			{0xa5}},
	{"INKEY",		{0xa6}},
	{"INSTR(",		{0xa7}},
	{"INT",			{0xa8}},
	{"LEN",			{0xa9}},
	{"LN",			{0xaa}},
	{"LOG",			{0xab}},
	{"NOT",			{0xac}},
	{"OPENUP",		{0xad}},
	{"OPENOUT",		{0xae}},
	{"PI",			{0xaf, Conditional}},
	{"POINT(",		{0xb0}},
	{"POS",			{0xb1, Conditional}},
	{"RAD",			{0xb2}},
	{"RND",			{0xb3, Conditional}},
	{"SGN",			{0xb4}},
	{"SIN",			{0xb5}},
	{"SQR",			{0xb6}},
	{"TAN",			{0xb7}},
	{"TO",			{0xb8}},
	{"TRUE",		{0xb9, Conditional}},
	{"USR",			{0xba}},
	{"VAL",			{0xbb}},
	{"VPOS",		{0xbc, Conditional}},
	{"CHR$",		{0xbd}},
	{"GET$",		{0xbe}},
	{"INKEY$",		{0xbf}},
	{"LEFT$(",		{0xc0}},
	{"MID$(",		{0xc1}},
	{"RIGHT$(",		{0xc2}},
	{"STR$",		{0xc3}},
	{"STRING$(",	{0xc4}},
	{"EOF",			{0xc5, Conditional}},
	{"AUTO",		{0xc6, LineNumber}},
	{"DELETE",		{0xc7, LineNumber}},
	{"LOAD",		{0xc8, Middle}},
	{"LIST",		{0xc9, LineNumber}},
	{"NEW",			{0xca, Conditional}},
	{"OLD",			{0xcb, Conditional}},
	{"RENUMBER",	{0xcc, LineNumber}},
	{"SAVE",		{0xcd, Middle}},
	{"PTR",			{0xcf}},
	{"PAGE",		{0xd0}},
	{"TIME",		{0xd1}},
	{"LOMEM",		{0xd2}},
	{"HIMEM",		{0xd3}},
	{"SOUND",		{0xd4, Middle}},
	{"BPUT",		{0xd5, Middle | Conditional}},
	{"CALL",		{0xd6, Middle}},
	{"CHAIN",		{0xd7, Middle}},
	{"CLEAR",		{0xd8, Conditional}},
	{"CLOSE",		{0xd9, Middle | Conditional}},
	{"CLG",			{0xda, Conditional}},
	{"CLS",			{0xdb, Conditional}},
	{"DATA",		{0xdc, REM}},
	{"DEF",			{0xdd}},
	{"DIM",			{0xde, Middle}},
	{"DRAW",		{0xdf, Middle}},
	{"END",			{0xe0, Conditional}},
	{"ENDPROC",		{0xe1, Conditional}},
	{"ENVELOPE",	{0xe2, Middle}},
	{"ENVELOPE",	{0xe2, Middle}},
	{"FOR",			{0xe3, Middle}},
	{"GOSUB",		{0xe4, LineNumber | Middle}},
	{"GOTO",		{0xe5, LineNumber | Middle}},
	{"GCOL",		{0xe6, Middle}},
	{"IF",			{0xe7, Middle}},
	{"INPUT",		{0xe8, Middle}},
	{"LET",			{0xe9, Start}},
	{"LOCAL",		{0xea, Middle}},
	{"MODE",		{0xeb, Middle}},
	{"MOVE",		{0xec, Middle}},
	{"NEXT",		{0xed, Middle}},
	{"ON",			{0xee, Middle}},
	{"VDU",			{0xef, Middle}},
	{"PLOT",		{0xf0, Middle}},
	{"PRINT",		{0xf1, Middle}},
	{"PROC",		{0xf2, FNProc | Middle}},
	{"READ",		{0xf3, Middle}},
	{"REM",			{0xf4, REM}},
	{"REPEAT",		{0xf5}},
	{"REPORT",		{0xf6, Conditional}},
	{"RESTORE",		{0xf7, LineNumber | Middle}},
	{"RETURN",		{0xf8, Conditional}},
	{"RUN",			{0xf9, Conditional}},
	{"STOP",		{0xfa, Conditional}},
	{"COLOUR",		{0xfb, Middle}},
	{"TRACE",		{0xfc, LineNumber | Middle}},
	{"UNTIL",		{0xfd, Middle}},
	{"WIDTH",		{0xfe, Middle}},
	{"OSCLI",		{0xff, Middle}},
};

}

namespace Tokeniser {

std::vector<uint8_t> import(FILE *input) {
	struct Importer {
		Importer(FILE *input) : input_(input) {}

		void tokenise() {
			while(true) {
				// Consume whitespace.
				while(true) {
					const auto ch = next();
					if(!isspace(ch)) {
						replace(ch);
						break;
					}
				}
				if(feof(input_)) break;

				// Get line number.
				const auto line_number = read_line_number();

				// Write start of line, including line number.
				result.push_back(0x0d);
				result.push_back(uint8_t(line_number >> 8));
				result.push_back(uint8_t(line_number));

				// Reserve a spot for line length.
				const auto size_position = result.size();
				result.push_back(0);

				// Encode line.
				tokenise_line();
				
				// Set line length.
				const auto line_length = 3 + result.size() - size_position;
				if(line_length > 255) throw_error(Error::Type::LineTooLong);
				result[size_position] = uint8_t(line_length);
			}
		}

		std::vector<uint8_t> result;

	private:
		void tokenise_line() {
			bool statement_start = true;

			while(!feof(input_)) {
				// Check for a new token.
				std::string input_text;
				auto node = &tokens;
				while(true) {
					const auto ch = next();
					input_text.push_back(ch);
					node = node->find(ch);
					if(!node) {
						replace(input_text);
						break;
					}
					if(node->value()) {
						break;
					}
				}

				// If a token was found and is conditional, check whether to tokenise.
				if(node && node->value()->flags & Flags::Conditional) {
					auto ch = next();
					if(isalnum(ch)) {
						// Don't treat as a token then.
						std::copy(input_text.begin(), input_text.end(), std::back_inserter(result));
						while(isalnum(ch)) {
							result.push_back(ch);
							ch = next();
						}
						replace(ch);
						continue;
					}
				}

				if(node) {
					const auto &keyword = *node->value();
					result.push_back(keyword.token);

					if(keyword.flags & Flags::FNProc) {
						// Copy all alphanumerics (and underscores?)
						while(true) {
							const auto ch = next();
							if(!isalnum(ch) && ch != '_') {
								break;
							}
							if(ch == '\n' || feof(input_)) return;
							result.push_back(ch);
						}
					}

					if(keyword.flags & Flags::LineNumber) {
						tokenise_line_number();
					}

					if(keyword.flags & Flags::REM) {
						// Copy rest of line without tokenisation.
						while(true) {
							const auto ch = next();
							if(ch == '\n' || feof(input_)) return;
							result.push_back(ch);
						}
					}

					if(statement_start && (keyword.flags & Flags::PseudoVariable)) {
						// Adjust token.
						result.back() += 0x40;
					}

					statement_start &= !(keyword.flags & Flags::Middle);
					statement_start |= keyword.flags & Flags::Start;
					continue;
				}

				// If here: no token was found. So copy at least one character
				// from the input and possibly more.
				const auto ch = next();
				if(ch == '\n') return;

				const bool was_start = statement_start;
				statement_start = false;
				result.push_back(ch);
				switch(ch) {
					case ':':
						// Go back into start mode after each colon.
						statement_start = true;
					break;

					case '*':
						// If a * is encountered while in start mode, blindly copy from it to
						// the end of the line.
						if(was_start) {
							while(true) {
								const auto ch = next();
								if(ch == '\n' || feof(input_)) {
									return;
								}
								result.push_back(ch);
							}
						}
					break;

					case '"':
						// Copy an entire string.
						while(true) {
							const auto ch = next();
							result.push_back(ch);
							if(ch == '\n') throw_error(Error::Type::BadStringLiteral);
							if(ch == '"') break;	// TODO: is this written out?
							if(feof(input_)) throw_error(Error::Type::BadStringLiteral);
						}
					break;

					case '&':
						// Copy an entire hex number.
						while(true) {
							const auto ch = next();
							const bool is_hex =
								(ch >= '0' && ch <= '9') ||
								(ch >= 'A' && ch <= 'F');
							if(!is_hex) {
								replace(ch);
								break;
							}
							result.push_back(ch);
						}
					break;

					default:
						// This is a variable name or number, copy it all.
						if(isalpha(ch)) {
							while(true) {
								const auto ch = next();
								if(!isalnum(ch)) {
									replace(ch);
									break;
								}
								result.push_back(ch);
							}
						}
					break;
				}
			}
		}
		
		int read_line_number() {
			while(true) {
				const auto num = next();
				if(isdigit(num)) {
					replace(num);
					break;
				}
				if(!isspace(num)) {
					throw_error(Error::Type::NoLineNumber);
				}
			}

			int line_number = 0;
			while(true) {
				const auto num = next();
				if(!isdigit(num)) {
					replace(num);
					break;
				}
				line_number = (line_number * 10) + (num - '0');
				if(line_number > 32767) {
					throw_error(Error::Type::BadLineNumber);
				}
			};
			return line_number;
		}

		void tokenise_line_number() {
			// PrŽcis on format:
			//
			// $8d is the token for a line number; the three subsequent bytes all have
			// 01 as their top two bits and some other portion of the original bits beneath.
			// Bit 6 of both bytes of the target line number is inverted.

			const int number = read_line_number() ^ 0b0100'0000'0100'0000;
			const auto high = uint8_t(number >> 8);
			const auto low = uint8_t(number);

			result.push_back(0x8d);
			result.push_back(0b0100'0000 | ((high & 0b1100'0000) >> 2) | ((low & 0b1100'0000) >> 4));
			result.push_back(0b0100'0000 | (high & 0b0011'1111));
			result.push_back(0b0100'0000 | (low & 0b0011'1111));
		}

		void replace(const std::string &n) {
			for(const auto c: n) source_line_ -= c == '\n';
			next_.insert(next_.end(), n.rbegin(), n.rend());
		}

		void replace(char n) {
			source_line_ -= n == '\n';
			next_.push_back(n);
		}

		char next() {
			if(!next_.empty()) {
				const auto n = next_.back();
				next_.pop_back();
				source_line_ += n == '\n';
				return n;
			}

			// Read a character from the file, ignoring \r and
			// keeping track of the current line.
			int next = 0;
			do {				next = fgetc(input_);
			} while(next == '\r');
			if(feof(input_)) return 0;

			source_line_ += next == '\n';
			return static_cast<char>(next);
		}

	private:
		FILE *input_ = stdin;
		std::string next_;
		int source_line_ = 1;

		void throw_error(Error::Type type) const {
			throw Error{type, source_line_};
		}
	} importer(input);
	importer.tokenise();

	// Append "end of program".
	importer.result.push_back(0x0d);
	importer.result.push_back(0xff);
	return importer.result;
}

}
