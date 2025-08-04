#include "utils/ErrorHelpers.hpp"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::vector<std::pair<std::string, std::shared_ptr<std::atomic_flag>>> vCheckFlag;

char *checkError(int /*unused*/, char *buffer, int /*unused*/) { return buffer; }

char *checkError(char *result, const char * /*unused*/, int /*unused*/) { return result; }
