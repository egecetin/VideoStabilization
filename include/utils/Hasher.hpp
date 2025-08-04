#pragma once

#include <cstdint>
#include <cstdio>

// NOLINTBEGIN

/**
 * Calculates the compile-time hash value of a string.
 * @param[in] s The input string. Must be null-terminated.
 * @return The hash value of the input string.
 */
constexpr size_t constHasher(const char *s) { return (s && *s) ? (*s + 33 * constHasher(s + 1)) : 5381; }

/**
 * Generates entropy using the input string and an initial value.
 * @param[in] entropy The input string used for generating entropy.
 * @param[in] iv The initial value for the entropy generation.
 * @return The generated entropy value.
 */
template <size_t N> constexpr uint64_t constSeeder(const char (&entropy)[N], const uint64_t iv = 0)
{
	auto value{iv};
	for (size_t i{0}; i < N; i++)
	{
		// Xor 1st byte of seed with input byte
		value = (value & (unsigned(~0) << 8)) | ((value & 0xFF) ^ entropy[i]);
		// Rotl 1 byte
		value = value << 8 | value >> ((sizeof(value) * 8) - 8);
	}
	return value ^ iv;
}

// NOLINTEND
