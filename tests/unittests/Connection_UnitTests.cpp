#include "connection/Http.hpp"
#include "connection/RawSocket.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

#define RAWSOCKET_BUFFER_SIZE 65536

TEST(Connection_Tests, HttpUnitTests)
{
	int echoServerPort = 8000;

	// Launch echo server
	std::future<int> pyResult;
	pyResult = std::async(std::launch::async, [echoServerPort]() {
		return system(("python3 " + std::string(TEST_REST_ECHO_SERVER_PY_PATH) +
					   " --port=" + std::to_string(echoServerPort) + " --count=4 >/dev/null")
						  .c_str());
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	std::string testHttpServerAddr = "http://localhost:" + std::to_string(echoServerPort);
	HTTP handler(testHttpServerAddr);

	handler.setOption(CURLOPT_SSL_VERIFYPEER, false);
	handler.setOption(CURLOPT_SSL_VERIFYHOST, false);

	ASSERT_EQ(testHttpServerAddr, handler.getHostAddress());

	HttpStatus::Code statusCode = HttpStatus::Code::xxx_max;
	std::string recvData;
	ASSERT_EQ(handler.sendPOSTRequest("", "Test POST Message", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("Test POST Message", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	auto stats = handler.getStats();

	ASSERT_NE(stats.uploadBytes, 0);
	ASSERT_NE(stats.downloadBytes, 0);
	ASSERT_NE(stats.headerBytes, 0);
	ASSERT_NE(stats.requestBytes, 0);
	ASSERT_NE(stats.uploadSpeed, 0);
	ASSERT_NE(stats.downloadSpeed, 0);
	ASSERT_NE(stats.connectionTime, 0);
	ASSERT_NE(stats.nameLookupTime, 0);
	ASSERT_NE(stats.preTransferTime, 0);
	ASSERT_EQ(stats.redirectTime, 0);
	ASSERT_NE(stats.startTransferTime, 0);
	ASSERT_NE(stats.totalTime, 0);

	statusCode = HttpStatus::Code::xxx_max;
	ASSERT_EQ(handler.sendPUTRequest("", "Test PUT Message", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	statusCode = HttpStatus::Code::xxx_max;
	ASSERT_EQ(handler.sendGETRequest("", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	statusCode = HttpStatus::Code::xxx_max;
	ASSERT_EQ(handler.sendHEADRequest("", recvData, statusCode), CURLE_OK);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::OK, statusCode);

	pyResult.wait();
	ASSERT_EQ(0, pyResult.get());

	// Send requests to closed server
	statusCode = HttpStatus::Code::xxx_max;
	ASSERT_EQ(handler.sendPOSTRequest("", "Test POST Message", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);

	stats = handler.getStats();

	ASSERT_EQ(stats.uploadBytes, 0);
	ASSERT_EQ(stats.downloadBytes, 0);
	ASSERT_EQ(stats.headerBytes, 0);
	ASSERT_EQ(stats.requestBytes, 0);
	ASSERT_EQ(stats.uploadSpeed, 0);
	ASSERT_EQ(stats.downloadSpeed, 0);
	ASSERT_EQ(stats.connectionTime, 0);
	ASSERT_NE(stats.nameLookupTime, 0);
	ASSERT_EQ(stats.preTransferTime, 0);
	ASSERT_EQ(stats.redirectTime, 0);
	ASSERT_EQ(stats.startTransferTime, 0);
	ASSERT_NE(stats.totalTime, 0);

	ASSERT_EQ(handler.sendPUTRequest("", "Test PUT Message", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);

	ASSERT_EQ(handler.sendGETRequest("", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);

	ASSERT_EQ(handler.sendHEADRequest("", recvData, statusCode), CURLE_COULDNT_CONNECT);
	ASSERT_EQ("", recvData);
	ASSERT_EQ(HttpStatus::Code::xxx_max, statusCode);
}

TEST(Connection_Tests, RawSocketUnitTests)
{
	if (geteuid() != 0)
	{
		GTEST_SKIP() << "Skipping test due to insufficient permissions.";
	}

	// Launch packet sender
	std::future<int> pyResult;
	pyResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_RAWSOCKET_SENDER_PY_PATH) + " >/dev/null").c_str());
	});

	bool found = false;
	uint8_t data[RAWSOCKET_BUFFER_SIZE];

	// Read tests
	size_t recvSize = 0;
	RawSocket sockRead(TEST_RAWSOCKET_INTERFACE, false);
	for (size_t idx = 0; idx < 1e2; ++idx)
	{
		recvSize = sockRead.readData(data, sizeof(data));
		ASSERT_GT(recvSize, 0);
		if (recvSize == sizeof("I'm a dumb message.") - 1 && !memcmp(data, "I'm a dumb message.", recvSize))
		{
			found = true;
			break;
		}
	}
	ASSERT_TRUE(found);
	ASSERT_LT(sockRead.writeData(data, recvSize), 0);
	ASSERT_EQ(sockRead.getInterfaceName(), TEST_RAWSOCKET_INTERFACE);

	RawSocketStats statsRead = sockRead.getStats(true);
	ASSERT_GT(statsRead.processingTime, 0.0);
	ASSERT_GT(statsRead.receivedBytes, 0);
	ASSERT_EQ(statsRead.sentBytes, 0);

	statsRead = sockRead.getStats();
	ASSERT_DOUBLE_EQ(statsRead.processingTime, 0.0);
	ASSERT_EQ(statsRead.receivedBytes, 0);
	ASSERT_EQ(statsRead.sentBytes, 0);

	// Write tests
	RawSocket sockWrite(TEST_RAWSOCKET_INTERFACE, true);
	ASSERT_GT(sockWrite.writeData(data, recvSize), 0);
	ASSERT_LT(sockWrite.readData(data, sizeof(data)), 0);
	ASSERT_EQ(sockWrite.getInterfaceName(), TEST_RAWSOCKET_INTERFACE);

	RawSocketStats statsWrite = sockWrite.getStats(true);
	ASSERT_GT(statsWrite.processingTime, 0.0);
	ASSERT_EQ(statsWrite.receivedBytes, 0);
	ASSERT_EQ(statsWrite.sentBytes, recvSize);

	statsWrite = sockWrite.getStats();
	ASSERT_DOUBLE_EQ(statsWrite.processingTime, 0.0);
	ASSERT_EQ(statsWrite.receivedBytes, 0);
	ASSERT_EQ(statsWrite.sentBytes, 0);

	pyResult.wait();
	ASSERT_EQ(0, pyResult.get());
}
