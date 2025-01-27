#include "tokeniser.hpp"
#include "CRC.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iostream>

namespace {

class UEFWriter {
private:
	using CRCGenerator = CRC::Generator<uint16_t, 0x0000, 0x0000, false, false>;

public:
	UEFWriter(const std::string &file_name) {
		file_ = fopen(file_name.c_str(), "wb");
		if(!file_) {
			throw std::runtime_error(std::string("Unable to open for output: ") + file_name);
		}

		// Write header.
		fputs("UEF File!", file_);
		fputc(0, file_);
		fputc(10, file_);
		fputc(0, file_);
	}

	struct Chunk {
	public:
		Chunk(uint16_t id, FILE *file, CRCGenerator &crc_generator) : id_(id), file_(file), crc_generator_(crc_generator) {}

		~Chunk() {
			// Write chunk ID.
			fputc(uint8_t(id_ >> 0), file_);
			fputc(uint8_t(id_ >> 8), file_);

			// Write chunk length.
			const auto length = uint32_t(chunk_contents_.size());
			fputc(uint8_t(length >> 0), file_);
			fputc(uint8_t(length >> 8), file_);
			fputc(uint8_t(length >> 16), file_);
			fputc(uint8_t(length >> 24), file_);

			// Write chunk data.
			fwrite(chunk_contents_.data(), 1, chunk_contents_.size(), file_);
		}

		template <typename IteratorT>
		void append(IteratorT begin, IteratorT end, bool append_crc = false) {
			// Write data. Could do a better job by type inspection but
			// quantities of data in this program are so small that there's
			// virtue in simplicity.
			chunk_contents_.reserve(chunk_contents_.size() + std::distance(begin, end) + (append_crc ? 2 : 0));
			for(auto it = begin; it != end; it++) {
				chunk_contents_.push_back(*it);
			}
			if(append_crc) {
				const uint16_t crc = crc_generator_.compute_crc(begin, end);
				chunk_contents_.push_back(uint8_t(crc >> 8));
				chunk_contents_.push_back(uint8_t(crc >> 0));
			}
		}

		template <typename CollectionT>
		void append(const CollectionT &data, bool append_crc = false) {
			append(std::begin(data), std::end(data), append_crc);
		}

	private:
		uint16_t id_;
		FILE *file_;
		CRCGenerator &crc_generator_;
		std::vector<uint8_t> chunk_contents_;
	};

	Chunk chunk(const uint16_t id) {
		return Chunk(id, file_, crc_generator_);
	}

	~UEFWriter() {
		fclose(file_);
	}

private:
	FILE *file_ = nullptr;
	inline static CRCGenerator crc_generator_{0x1021};
};

void print_help() {
	std::cout << "usage: bas2uef [-i input file] [-o output file]" << std::endl;
}

}

int main(int argc, char *argv[]) try {
	std::string output = "out.uef";
	std::string input = "";

	// Do a negligible parsing of command-line options.
	for(int c = 1; c < argc; c++) {
		if(c == argc - 1) {
			print_help();
			return -1;
		}

		if(std::string("-o") == argv[c]) {
			output = argv[c + 1];
			++c;
			continue;
		}

		if(std::string("-i") == argv[c]) {
			input = argv[c + 1];
			++c;
			continue;
		}

		print_help();
		return -1;
	}

	// Read from file or from stdin if none was specified.
	FILE *const in = input.empty() ? stdin : fopen(input.c_str(), "rt");
	if(!in) {
		std::cerr << "Couldn't open " << input << std::endl;
		return -1;
	}
	const auto result = Tokeniser::import(in);
	if(in != stdin) {
		fclose(in);
	}

	// Always output to a file as this is primarily binary data.
	UEFWriter writer(output);
	writer.chunk(0x0000).append("bas2uef v1.0");

	// Write high tone with a dummy byte.
	writer.chunk(0x0111).append(std::vector<uint8_t>{0xdc, 0x05, 0xdc, 0x05});

	// Divide data into max-256-byte segments.
	auto begin = std::begin(result);
	const auto end = std::end(result);
	uint16_t block_number = 0;
	while(begin != end) {
		const auto remaining = int(std::distance(begin, end));
		const auto length = std::min(256, remaining);

		const auto block_begin = begin;
		std::advance(begin, length);

		auto block = writer.chunk(0x0100);
		block.append(std::vector<uint8_t>{ 0x2a });					// Synchronisation byte.
		block.append(std::vector<uint8_t>{
			'B', 'A', 'S', 'I', 'C', 0x00,							// File name, with terminator.
			0x00, 0x19, 0x00, 0x00,									// Load address.
			0x23, 0x80, 0x00, 0x00,									// Execution address.
			uint8_t(block_number >> 0), uint8_t(block_number >> 8),	// Block number.
			uint8_t(length >> 0), uint8_t(length >> 8),				// Block length.
			uint8_t(begin == end ? 0x80 : 0x00),					// Block flag.
			0x00, 0x00, 0x00, 0x00,									// Four unused bytes.
		}, true);
		block.append(block_begin, begin, true);
		writer.chunk(0x0110).append(std::vector{0x58, 0x02});
		++block_number;
	}

	return 0;
} catch(const Tokeniser::Error &error) {
	std::cout << "ERROR: " << error.to_string() << std::endl;
} catch(std::exception &error) {
	std::cout << "ERROR: " << error.what() << std::endl;
}
