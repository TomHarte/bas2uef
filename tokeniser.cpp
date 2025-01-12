#include "tokeniser.h"

#include <cctype>
#include <cstdio>
#include <map>

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

/*
	Cf. pages 41-43 of the BASIC ROM User Guide by Mark Plumbley.
*/
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

std::vector<uint8_t> import() {
	struct Importer {
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
				auto num = next();
				if(!isnumber(num)) {
					throw Error::NoLineNumber;
				}
				int line_number = 0;
				do {
					line_number = (line_number * 10) + (num - '0');
					if(line_number > 0xfeff) {
						throw Error::BadLineNumber;
					}
					num = next();
				} while(isnumber(num));
				replace(num);

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
				if(line_length > 255) throw Error::LineTooLong;
				result[size_position] = uint8_t(line_length);
			}
		}

		int line = 1;
		std::vector<uint8_t> result;

	private:
		void tokenise_line() {
			bool statement_start = true;
			bool conditional = false;

			while(!feof(input_)) {
				// If conditional and alphanumeric, write a single character and continue.
				if(conditional) {
					const auto ch = next();
					if(isalnum(ch)) {
						conditional = false;
						result.push_back(ch);
						continue;
					} else {
						replace(ch);
					}
				}

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

				if(node && node->value()) {
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
				} else {
					const auto ch = next();
					if(ch == '\n') return;

					result.push_back(ch);
					switch(ch) {
						case ':':
							statement_start = true;
						break;

						case '*':
							if(statement_start) {
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
							while(true) {
								const auto ch = next();
								result.push_back(ch);
								if(ch == '\n') throw Error::BadStringLiteral;
								if(ch == '"') break;
								if(feof(input_)) throw Error::BadStringLiteral;
							}
						break;

						default:
							// Tokenise entire variable names in one go.
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
					statement_start = false;
				}
			}
		}
		
		void tokenise_line_number() {
			while(!feof(input_)) {
				
			}
			//	while(!ErrorNum && !EndOfFile)
			//	{
			//		if(NumberStart)
			//		{
			//			// tokenise line number
			//			Uint16 LineNumber = NumberValue ^ 0x4040;
			//
			//			WriteByte(0x8d);
			//
			//			WriteByte(((LineNumber&0xc0) >> 2) | ((LineNumber&0xc000) >> 12) | 0x40);
			//			WriteByte((LineNumber&0x3f) | 0x40);
			//			WriteByte(((LineNumber >> 8)&0x3f) | 0x40);
			//
			//			EatCharacters(NumberLength);
			//		}
			//		else
			//			switch(Token)
			//			{
			//				// whitespace and commas do not cause this mode to exit
			//				case ' ':
			//				case ',':
			//					WriteByte(Token);
			//					EatCharacters(1);
			//				break;
			//
			//				// hex numbers get through unscathed too
			//				case '&':
			//					WriteByte(Token);
			//					EatCharacters(1);
			//
			//					while(
			//						!ErrorNum &&
			//						!EndOfFile &&
			//						(
			//							(IncomingBuffer[0] >= '0' && IncomingBuffer[0] <= '9') ||
			//							(IncomingBuffer[0] >= 'A' && IncomingBuffer[0] <= 'F')
			//						)
			//					)
			//					{
			//						WriteByte(IncomingBuffer[0]);
			//						EatCharacters(1);
			//					}
			//				break;
			//
			//				/* grab strings without tokenising numbers */
			//				case '"':
			//					if(!CopyStringLiteral())
			//						return false;
			//				break;
			//
			//				/* default action is to turn off line number tokenising and get back to normal */
			//				default: return true;
			//			}
			//	}
		}

		void replace(const std::string &n) {
			next_.insert(next_.end(), n.rbegin(), n.rend());
		}

		void replace(char n) {
			next_.push_back(n);
		}

		char next() {
			if(!next_.empty()) {
				const auto n = next_.back();
				next_.pop_back();
				return n;
			}

			// Read a character from the file, ignoring \r and
			// keeping track of the current line.
			int next = 0;
			do {				next = fgetc(input_);
			} while(next == '\r');
			if(feof(input_)) return 0;

			if(next == '\n') ++line;
			return static_cast<char>(next);
		}

	private:
		FILE *input_ = stdin;
		std::string next_;
	} importer;
	importer.tokenise();

	// Append "end of program".
	importer.result.push_back(0x0d);
	importer.result.push_back(0xff);
	return importer.result;
}

}
	