#include "utils/Hasher.hpp"

#include <cstring>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char buffer[4096] = {'\0'};

	memcpy(buffer, data, size > 4095 ? 4095 : size);
	constHasher(buffer);

	memcpy(buffer, data, size > 4096 ? 4096 : size);
	constSeeder(buffer);

	return 0;
}
