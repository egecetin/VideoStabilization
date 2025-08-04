#include "logging/Logger.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Logger_Tests, LoggingUnitTests)
{
	std::future<int> shResult;

	const MainLogger logger("http://localhost:8400",
							"http://username:password@localhost:8400/foo"); // pragma: allowlist secret

	ASSERT_NE(logger.getLogger(), nullptr);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_LOG_SERVER_PY_PATH) + " --port=8400 >/dev/null").c_str());
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	ASSERT_NO_THROW(spdlog::trace("Trace message"));
	ASSERT_NO_THROW(spdlog::debug("Debug message"));
	ASSERT_NO_THROW(spdlog::warn("Warning message"));
	ASSERT_NO_THROW(spdlog::error("Error message"));
	ASSERT_NO_THROW(spdlog::critical("Critical message"));

	shResult.wait();
	ASSERT_EQ(0, shResult.get());

	const MainLogger logger2("", "");
	ASSERT_NE(logger2.getLogger(), nullptr);

	ASSERT_NO_THROW(spdlog::trace("Trace message"));
	ASSERT_NO_THROW(spdlog::debug("Debug message"));
	ASSERT_NO_THROW(spdlog::warn("Warning message"));
	ASSERT_NO_THROW(spdlog::error("Error message"));
	ASSERT_NO_THROW(spdlog::critical("Critical message"));
}
