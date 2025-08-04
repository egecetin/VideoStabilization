#include "zeromq/ZeroMQ.hpp"

#include <iostream>
#include <optional>

#include <spdlog/spdlog.h>
#include <zmq_addon.hpp>

// ZeroMQ send/receive timeouts in milliseconds
constexpr int ZEROMQ_MSG_TIMEOUT_MS = 1000;
// ZeroMQ heartbeat timeout in milliseconds
constexpr int ZEROMQ_HEARTBEAT_TIMEOUT_MS = 1000;

void ZeroMQ::init(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type,
				  const std::string_view &addr, bool isBind)
{
	_contextPtr = ctx;
	_socketAddr = addr;
	_isBinded = isBind;
	_isActive = false;

	// Init ZMQ connection
	_socketPtr = std::make_unique<zmq::socket_t>(*_contextPtr, type);
	_socketPtr->set(zmq::sockopt::linger, 0);
	_socketPtr->set(zmq::sockopt::sndtimeo, ZEROMQ_MSG_TIMEOUT_MS);
	_socketPtr->set(zmq::sockopt::rcvtimeo, ZEROMQ_MSG_TIMEOUT_MS);
	_socketPtr->set(zmq::sockopt::heartbeat_ivl, ZEROMQ_HEARTBEAT_TIMEOUT_MS);
	_socketPtr->set(zmq::sockopt::heartbeat_ttl, ZEROMQ_HEARTBEAT_TIMEOUT_MS * 3);
	_socketPtr->set(zmq::sockopt::heartbeat_timeout, ZEROMQ_HEARTBEAT_TIMEOUT_MS);
}

ZeroMQ::ZeroMQ(const zmq::socket_type &type, const std::string &addr, bool isBind)
{
	init(std::make_shared<zmq::context_t>(1), type, addr, isBind);
}

ZeroMQ::ZeroMQ(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
			   bool isBind)
{
	init(ctx, type, addr, isBind);
}

bool ZeroMQ::start()
{
	if (_isActive)
	{
		return false;
	}

	if (_isBinded)
	{
		_socketPtr->bind(_socketAddr);
	}
	else
	{
		_socketPtr->connect(_socketAddr);
	}
	_isActive = true;

	return true;
}

void ZeroMQ::stop()
{
	if (!_isActive)
	{
		return;
	}

	if (_isBinded)
	{
		_socketPtr->unbind(_socketAddr);
	}
	else
	{
		_socketPtr->disconnect(_socketAddr);
	}
	_isActive = false;
}

std::vector<zmq::message_t> ZeroMQ::recvMessages()
{
	std::vector<zmq::message_t> recvMsgs;
	if (!_isActive)
	{
		spdlog::warn("Connection needs to starting");
	}
	else
	{
		auto nMsgs = zmq::recv_multipart(*_socketPtr, std::back_inserter(recvMsgs));
		spdlog::trace("Received {} messages", nMsgs.value_or(0));
	}
	return recvMsgs;
}

size_t ZeroMQ::sendMessages(std::vector<zmq::message_t> &msg)
{
	zmq::send_result_t res;
	if (!_isActive)
	{
		spdlog::warn("Connection needs to starting");
	}
	else
	{
		res = zmq::send_multipart(*_socketPtr, msg);
	}

	return res.value_or(0);
}

ZeroMQ::~ZeroMQ()
{
	try
	{
		stop();
	}
	catch (const std::exception &e)
	{
		try
		{
			spdlog::error("Error while stopping ZeroMQ connection {} ({})", _socketAddr, e.what());
		}
		catch (const std::exception &e2)
		{
			std::cerr << "Error while stopping ZeroMQ connection and logger for connection " << _socketAddr << " ("
					  << e.what() << ")" << '\n'
					  << e2.what() << '\n';
		}
	}
}
