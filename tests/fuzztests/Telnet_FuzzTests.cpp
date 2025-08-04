#include "telnet/TelnetServer.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <thread>

#define TELNET_SERVER_PORT 9001

class SocketWrapper {
  private:
	int sockFd{-1};

  public:
	explicit SocketWrapper(uint16_t port)
	{
		struct hostent *host = gethostbyname("localhost");
		if (host == nullptr)
		{
			throw std::runtime_error("Gethostbyname failed");
		}

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = *((unsigned long *)host->h_addr);

		sockFd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockFd < 0)
		{
			throw std::runtime_error("Socket failed");
		}

		if (connect(sockFd, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			throw std::runtime_error("Connect failed");
		}
	}

	int sendMessage(const uint8_t *data, size_t size) { return send(sockFd, data, size, 0) < 0 ? -1 : 0; }

	~SocketWrapper()
	{
		if (sockFd >= 0)
		{
			close(sockFd);
		}
	}
};

class TelnetWrapper {
  private:
	std::shared_ptr<TelnetServer> server;

  public:
	explicit TelnetWrapper(uint16_t port)
	{
		server = std::make_shared<TelnetServer>();
		if (!server || !(server->initialise(port, nullptr)))
		{
			throw std::runtime_error("Can't init telnet");
		}

		server->connectedCallback(TelnetConnectedCallback);
		server->newLineCallback(TelnetMessageCallback);
		server->tabCallback(TelnetTabCallback);

	}
};

extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
	spdlog::set_level(spdlog::level::off);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static TelnetWrapper telnetWrap(TELNET_SERVER_PORT);
	static SocketWrapper socketWrap(TELNET_SERVER_PORT);

	return socketWrap.sendMessage(data, size);
}
