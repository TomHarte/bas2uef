#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>

namespace CRC {
struct ByteSwapped16 {
public:
	explicit constexpr ByteSwapped16(uint16_t original) noexcept {
		value_ = std::rotl(original, 8);
	}
	ByteSwapped16() = default;

	uint16_t raw() const {	return value_;	}
	static ByteSwapped16 from_raw(const uint16_t raw) {
		ByteSwapped16 result{};
		result.value_ = raw;
		return result;
	}

	explicit operator uint16_t() const {	return std::rotl(value_, 8);	}
	uint8_t high() const {	return uint8_t(value_ >> 0);	}
	uint8_t low() const {	return uint8_t(value_ >> 8);	}

private:
	uint16_t value_ = 0;
};

template <uint16_t polynomial = 0x1021, typename IteratorT>
ByteSwapped16 crc16(IteratorT begin, const IteratorT end, const ByteSwapped16 initial = ByteSwapped16{0x0000}) {
	// Generates an at-compile-time table mapping from the top byte of a 16-bit CRC in progress
	// to the net XOR mask that results from bit-by-bit rotates to the left.
	//
	// The final table is byte swapped to simplify the loop below; the compiler proved
	// trustworthy in spotting that these tables are independent of the type of IteratorT,
	// including only one per polynomial in the produced binary.
	static constexpr auto xor_table = [] {
		constexpr uint16_t xor_masks[] = {0, std::rotl(uint16_t(polynomial ^ 1), 8)};
		std::array<uint16_t, 256> table;

		std::iota(table.begin(), table.end(), 0);
		for(auto &value: table) {
			for(int bit = 0; bit < 8; bit++) {
				value = std::rotl(value, 1);
				value ^= xor_masks[(value >> 8) & 1];
			}
		}
		return table;
	} ();

	// Calculate the CRC in byte-swapped form so as slighltly to simplify the inner loop.
	uint16_t crc = initial.raw();
	while(begin != end) {
		crc = xor_table[(*begin ^ crc) & 0xff] ^ (crc >> 8);
		++begin;
	}
	return ByteSwapped16::from_raw(crc);
}
}
