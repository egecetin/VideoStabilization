#include "zeromq/ZeroMQServer.hpp"

#include "Version.h"
#include "utils/ErrorHelpers.hpp"
#include "utils/Hasher.hpp"

#include <spdlog/spdlog.h>

constexpr uint32_t LOG_LEVEL_ID = (static_cast<uint32_t>('L') | (static_cast<uint32_t>('O') << 8) |
								   (static_cast<uint32_t>('G') << 16) | (static_cast<uint32_t>('L') << 24));
constexpr uint32_t VERSION_INFO_ID = (static_cast<uint32_t>('V') | (static_cast<uint32_t>('E') << 8) |
									  (static_cast<uint32_t>('R') << 16) | (static_cast<uint32_t>('I') << 24));
constexpr uint32_t PING_PONG_ID = (static_cast<uint32_t>('P') | (static_cast<uint32_t>('I') << 8) |
								   (static_cast<uint32_t>('N') << 16) | (static_cast<uint32_t>('G') << 24));
constexpr uint32_t STATUS_CHECK_ID = (static_cast<uint32_t>('S') | (static_cast<uint32_t>('C') << 8) |
									  (static_cast<uint32_t>('H') << 16) | (static_cast<uint32_t>('K') << 24));
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

void ZeroMQServer::update()
{
	auto recvMsgs = recvMessages();

	if (!recvMsgs.empty())
	{
		std::vector<zmq::message_t> replyMsgs;

		ZeroMQServerStats serverStats;
		serverStats.processingTimeStart = std::chrono::high_resolution_clock::now();
		serverStats.isSuccessful = messageCallback() && messageCallback()(recvMsgs, replyMsgs);

		if (size_t nSentMsg = sendMessages(replyMsgs); nSentMsg != replyMsgs.size())
		{
			spdlog::warn("Can't send whole reply: Sent messages {} / {}", nSentMsg, replyMsgs.size());
		}
		serverStats.processingTimeEnd = std::chrono::high_resolution_clock::now();

		if (_stats)
		{
			_stats->consumeStats(recvMsgs, replyMsgs, serverStats);
		}
	}
}

void ZeroMQServer::threadFunc() noexcept
{
	spdlog::info("ZeroMQ server started");
	while (!_shouldStop._M_i)
	{
		try
		{
			update();
			if (_checkFlag)
			{
				_checkFlag->test_and_set();
			}
		}
		catch (const std::exception &e)
		{
			spdlog::error("ZeroMQ server failed: {}", e.what());
		}
	}
	spdlog::info("ZeroMQ server stopped");
}

ZeroMQServer::ZeroMQServer(const std::string &hostAddr, std::shared_ptr<std::atomic_flag> checkFlag,
						   const std::shared_ptr<prometheus::Registry> &reg, const std::string &prependName)
	: ZeroMQ(zmq::socket_type::rep, hostAddr, true), _checkFlag(std::move(checkFlag))
{
	if (reg)
	{
		_stats = std::make_unique<ZeroMQStats>(reg, prependName);
	}

	startMonitoring(getSocket().get(), "inproc://" + std::to_string(constHasher(hostAddr.c_str())) + ".rep");
}

bool ZeroMQServer::initialise()
{
	_shouldStop.clear();

	if (start())
	{
		_serverThread = std::make_unique<std::thread>(&ZeroMQServer::threadFunc, this);
		return true;
	}
	return false;
}

void ZeroMQServer::shutdown()
{
	_shouldStop.test_and_set();
	if (_serverThread && _serverThread->joinable())
	{
		_serverThread->join();
		_serverThread.reset();
	}

	stop();
}

bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::message_t> &replyMsgs)
{
	spdlog::trace("Received {} messages", recvMsgs.size());
	replyMsgs.clear();

	std::string replyBody;
	int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
	switch (*(static_cast<const uint64_t *>(recvMsgs[0].data())))
	{
	case LOG_LEVEL_ID: {
		if (recvMsgs.size() != 2)
		{
			spdlog::error("Received unknown number of messages for log level change");
			break;
		}

		spdlog::warn("Log level change request received");
		const auto receivedMsg = std::string(static_cast<const char *>(recvMsgs[1].data()), recvMsgs[1].size());

		if (receivedMsg == "v")
		{
			spdlog::set_level(spdlog::level::info);
		}
		if (receivedMsg == "vv")
		{
			spdlog::set_level(spdlog::level::debug);
		}
		if (receivedMsg == "vvv")
		{
			spdlog::set_level(spdlog::level::trace);
		}
		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		break;
	}
	case VERSION_INFO_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Received unknown number of messages for version information");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		replyBody = PROJECT_FULL_VERSION_STRING;
		break;
	}
	case PING_PONG_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Received unknown number of messages for ping");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		replyBody = "PONG";
		break;
	}
	case STATUS_CHECK_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Received unknown number of messages for status check");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;

		std::ostringstream oss;
		oss << "{";
		for (const auto &[process, statusFlag] : vCheckFlag)
		{
			oss << "\"" << process << "\":" << (statusFlag->_M_i ? "1," : "0,");
		}
		replyBody = oss.str();
		replyBody.replace(replyBody.size() - 1, 1, "}");
		break;
	}
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	default:
		spdlog::error("Unknown command received from control");
		break;
	}

	// Prepare reply
	replyMsgs.emplace_back(&reply, sizeof(reply));
	replyMsgs.emplace_back(replyBody.c_str(), replyBody.size());

	return reply == ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
}
