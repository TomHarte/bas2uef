#include "tokeniser.hpp"
#include "CRC.hpp"

#include <cstdint>
#include <iostream>

namespace {

struct UEFWriter {
	
};

}

int main(int argc, char *argv[]) try {
	const auto result = Tokeniser::import(stdin);

//	CRC::Generator<uint16_t, 0x0000, 0x0000, false, false> crc_(0x1021);
	FILE *const target = fopen("out.uef", "wb");

	// UEF: write header.
	fputs("UEF File!", target);
	fputc(0, target);
	fputc(10, target);
	fputc(0, target);
	fclose(target);

	// Write lead-in tone.
	fputc(0x11, target);	fputc(0x01, target);

	std::cout << "Produced " << result.size() << " bytes:" << std::endl;
	for(const auto &byte: result) {
		printf("%02x ", byte);
	}
	std::cout << std::endl;

	return 0;
} catch(const Tokeniser::Error &error) {
	std::cout << "ERROR: " << error.to_string() << std::endl;
}
