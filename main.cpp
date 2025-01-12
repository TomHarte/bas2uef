#include "tokeniser.h"

#include <iostream>

int main(int argc, char *argv[]) {
	const auto result = Tokeniser::import();
	
	FILE *target = fopen("out.uef", "wb");

	// UEF: write header.
	fputs("UEF File!", target);
	fputc(0, target);
	fputc(10, target);
	fputc(0, target);
	fclose(target);

	// Write lead-in tone. 2/4
	fputc(0x11, target);	fputc(0x01, target);

	std::cout << "Produced " << result.size() << " bytes:" << std::endl;
	for(const auto &byte: result) {
		printf("%02x ", byte);
	}
	std::cout << std::endl;

	return 0;
}