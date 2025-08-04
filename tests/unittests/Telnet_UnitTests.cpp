#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

constexpr int TELNET_PORT = 23000;

TEST(Telnet_Tests, TelnetServerUnitTests)
{
	std::string promServerAddr = "localhost:8200";

	std::future<int> shResult;

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	std::shared_ptr<std::atomic_flag> checkFlag;
	ASSERT_TRUE(telnetServerPtr->initialise(TELNET_PORT, checkFlag, "", reporter.createNewRegistry()));
	ASSERT_FALSE(telnetServerPtr->initialise(TELNET_PORT, checkFlag));
	ASSERT_NO_THROW(telnetServerPtr->shutdown());

	ASSERT_TRUE(telnetServerPtr->initialise(TELNET_PORT, checkFlag, "> "));
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);
	telnetServerPtr->tabCallback(TelnetTabCallback);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("expect " + std::string(TEST_TELNET_SH_PATH) + " >/dev/null").c_str());
	});

	shResult.wait();
	ASSERT_NO_THROW(telnetServerPtr->shutdown());
	ASSERT_NO_THROW(telnetServerPtr->shutdown());
	ASSERT_EQ(0, shResult.get());
}
