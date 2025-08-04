#include "connection/RawSocket.hpp"

#include "utils/ErrorHelpers.hpp"

#include <chrono>
#include <cstring>
#include <ios>
#include <stdexcept>
#include <utility>

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void RawSocket::init(int domain, int type, int protocol)
{
	_sockFd = socket(domain, type, protocol); // Init socket
	if (_sockFd < 0)
	{
		throw std::ios_base::failure(getErrnoString(errno));
	}
	if (bind(_sockFd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) < 0)
	{
		throw std::ios_base::failure(std::string("Bind failed: ") + getErrnoString(errno));
	}
}

RawSocket::RawSocket(std::string iface, bool isWrite) : _writeMode(isWrite), _iFace(std::move(iface))
{
	// Prepare socket address
	memset(static_cast<void *>(&_addr), 0, sizeof(sockaddr_ll));
	_addr.sll_family = AF_PACKET;
	_addr.sll_protocol = htons(ETH_P_ALL);
	_addr.sll_ifindex = static_cast<int>(if_nametoindex(_iFace.c_str()));
	if (_addr.sll_ifindex == 0)
	{
		throw std::ios_base::failure(std::string("Can't find interface: ") + getErrnoString(errno));
	}

	// Interface request
	ifreq ifr{};
	memset(static_cast<void *>(&ifr), 0, sizeof(ifreq));
	memcpy(std::addressof(ifr.ifr_name), _iFace.c_str(),
		   _iFace.size()); // Size should be sufficient because if_nametoindex not failed

	if (isWrite)
	{
		init(PF_PACKET, SOCK_RAW, IPPROTO_RAW);
		if (setsockopt(_sockFd, SOL_SOCKET, SO_BINDTODEVICE, static_cast<void *>(&ifr), sizeof(ifr)) < 0)
		{
			throw std::ios_base::failure(std::string("Can't set socket options: ") + getErrnoString(errno));
		}
	}
	else
	{
		init(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	}
	_isReady = true;
}

int RawSocket::writeData(const unsigned char *data, size_t dataLen)
{
	if (!_isReady || !_writeMode)
	{
		return -EPERM;
	}

	auto startTime = std::chrono::high_resolution_clock::now();
	const auto retval = static_cast<int>(write(_sockFd, data, dataLen));

	// Update stats
	_stats.processingTime += static_cast<double>((std::chrono::high_resolution_clock::now() - startTime).count());
	_stats.sentBytes += dataLen;

	return retval;
}

int RawSocket::readData(unsigned char *data, size_t dataLen)
{
	if (!_isReady || _writeMode)
	{
		return -EPERM;
	}
	// NOLINTNEXTLINE(cppcoreguidelines-init-variables)
	socklen_t socketLen = sizeof(_addr);

	auto startTime = std::chrono::high_resolution_clock::now();
	const auto retval =
		static_cast<int>(recvfrom(_sockFd, data, dataLen, 0, reinterpret_cast<sockaddr *>(&_addr), &socketLen));

	// Update stats
	_stats.processingTime += static_cast<double>((std::chrono::high_resolution_clock::now() - startTime).count());
	_stats.receivedBytes += dataLen;

	return retval;
}

RawSocketStats RawSocket::getStats(bool resetInternalStats)
{
	if (resetInternalStats)
	{
		RawSocketStats buffer = _stats;
		_stats = {0, 0, 0.0};
		return buffer;
	}
	return _stats;
}

RawSocket::~RawSocket() { close(_sockFd); }
