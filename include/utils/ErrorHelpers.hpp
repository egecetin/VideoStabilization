#pragma once

#include <array>
#include <atomic>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

/// Global variable to check if the servers are running
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern std::vector<std::pair<std::string, std::shared_ptr<std::atomic_flag>>> vCheckFlag;

// Alpine Linux incorrectly declares strerror_r
// https://stackoverflow.com/questions/41953104/strerror-r-is-incorrectly-declared-on-alpine-linux
char *checkError(int /*unused*/, char *buffer, int /*unused*/);
char *checkError(char *result, const char * /*unused*/, int /*unused*/);

/**
 * Converts errno to a readable string
 * @param[in] errVal errno value
 * @return std::string Error message
 */
inline std::string getErrnoString(int errVal)
{
	std::array<char, BUFSIZ> buffer{};
	return checkError(strerror_r(errVal, buffer.data(), BUFSIZ), buffer.data(), errVal);
}
