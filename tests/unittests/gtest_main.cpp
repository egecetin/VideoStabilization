#include "test-static-definitions.h"

#include "utils/ErrorHelpers.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#ifdef XXX_ENABLE_MEMLEAK_CHECK
#include <memplumber.h>
#endif

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int retval = EXIT_FAILURE;

	spdlog::set_level(spdlog::level::off);

	// Internal variables
	vCheckFlag.emplace_back("Test Disabled Info", std::make_unique<std::atomic_flag>(false));
	vCheckFlag.emplace_back("Test Enabled Info", std::make_unique<std::atomic_flag>(true));

#ifdef XXX_ENABLE_MEMLEAK_CHECK
	MemPlumber::start();
#endif
	retval = RUN_ALL_TESTS();
	spdlog::shutdown();
#ifdef XXX_ENABLE_MEMLEAK_CHECK
	MemPlumber::stop();

	// Check memory leak
	size_t memLeakCtr = 0;
	uint64_t memLeakSz = 0;
	MemPlumber::memLeakCheck(memLeakCtr, memLeakSz, true);

	// Comes from gtest itself
	if (memLeakCtr == 1 && memLeakSz == 8)
	{
		std::cout << "No memory leak detected!" << std::endl;
	}
	else if (memLeakCtr > 0 || memLeakSz > 0)
	{
		std::cout << "Number of leaked objects: " << memLeakCtr << std::endl;
		std::cout << "Total amount of memory leaked: " << memLeakSz << "[bytes]" << std::endl;
		retval = EXIT_FAILURE;
	}
#endif

	return retval;
}
