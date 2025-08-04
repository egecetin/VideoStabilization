#include "metrics/PrometheusServer.hpp"
#include "test-static-definitions.h"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(ZeroMQ_Tests, ZeroMQUnitTests)
{
	std::string echoServerAddr = "tcp://127.0.0.1:8001";

	// Launch echo server
	std::future<int> pyResult;
	pyResult = std::async(std::launch::async, [echoServerAddr]() {
		return system(("python3 " + std::string(TEST_ZEROMQ_ECHO_SERVER_PY_PATH) + " --address=" + echoServerAddr +
					   " --count=1 >/dev/null")
						  .c_str());
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	{
		std::shared_ptr<zmq::context_t> ctx = std::make_shared<zmq::context_t>(1);
		ZeroMQ handler(ctx, zmq::socket_type::req, echoServerAddr, false);

		ASSERT_EQ(ctx, handler.getContext());

		ASSERT_NO_THROW(handler.getSocket()->set(zmq::sockopt::linger, 1234));
		ASSERT_EQ(handler.getSocket()->get(zmq::sockopt::linger), 1234);

		ASSERT_TRUE(handler.start());
		ASSERT_FALSE(handler.start());

		int testData1 = 15;
		double testData2 = 3.14;
		std::string testData3 = "Test Message";

		std::vector<zmq::message_t> vExpectedMsg;
		vExpectedMsg.push_back(zmq::message_t(&testData1, sizeof(testData1)));
		vExpectedMsg.push_back(zmq::message_t(&testData2, sizeof(testData2)));
		vExpectedMsg.push_back(zmq::message_t(testData3.c_str(), testData3.size()));

		std::vector<zmq::message_t> vSendMsg;
		vSendMsg.push_back(zmq::message_t(&testData1, sizeof(testData1)));
		vSendMsg.push_back(zmq::message_t(&testData2, sizeof(testData2)));
		vSendMsg.push_back(zmq::message_t(testData3.c_str(), testData3.size()));
		ASSERT_EQ(handler.sendMessages(vSendMsg), vSendMsg.size());

		auto vRecvMsgs = handler.recvMessages();
		ASSERT_EQ(vSendMsg.size(), vRecvMsgs.size());
		for (size_t idx = 0; idx < vSendMsg.size(); ++idx)
		{
			ASSERT_TRUE(vSendMsg[idx].empty());

			ASSERT_EQ(vExpectedMsg[idx].size(), vRecvMsgs[idx].size());
			ASSERT_FALSE(memcmp(vExpectedMsg[idx].data(), vRecvMsgs[idx].data(), vExpectedMsg[idx].size()));
		}

		pyResult.wait();
		ASSERT_EQ(0, pyResult.get());
	}

	{
		ZeroMQ handler(zmq::socket_type::rep, echoServerAddr, true);

		std::vector<zmq::message_t> vSendMsg;
		ASSERT_EQ(handler.recvMessages().size(), 0);
		ASSERT_EQ(handler.sendMessages(vSendMsg), 0);

		ASSERT_NO_THROW(handler.stop());
		ASSERT_TRUE(handler.start());
	}
}

TEST(ZeroMQ_Tests, ZeroMQServerUnitTests)
{
	std::string zeromqServerAddr = "tcp://127.0.0.1:8300";
	std::string promServerAddr = "localhost:8301";

	std::future<int> shResult;

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init ZeroMQ Server
	std::shared_ptr<std::atomic_flag> checkFlag;
	auto zeromqServerPtr = std::make_shared<ZeroMQServer>(zeromqServerAddr, checkFlag);
	ASSERT_TRUE(zeromqServerPtr->initialise());
	ASSERT_FALSE(zeromqServerPtr->initialise());
	ASSERT_NO_THROW(zeromqServerPtr->shutdown());

	zeromqServerPtr = std::make_shared<ZeroMQServer>(zeromqServerAddr, checkFlag, reporter.createNewRegistry());
	zeromqServerPtr->messageCallback(ZeroMQServerMessageCallback);
	ASSERT_TRUE(zeromqServerPtr->initialise());

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_ZEROMQ_PY_PATH) + " >/dev/null").c_str());
	});

	shResult.wait();
	ASSERT_NO_THROW(zeromqServerPtr->shutdown());
	ASSERT_NO_THROW(zeromqServerPtr->shutdown());
	ASSERT_EQ(0, shResult.get());
}

TEST(ZeroMQ_Tests, ZeroMQMonitorUnitTests)
{
	ZeroMQMonitor monitor;
	ASSERT_THROW(monitor.startMonitoring(nullptr, ""), std::invalid_argument);
	ASSERT_NO_THROW(monitor.testInternals());
}
