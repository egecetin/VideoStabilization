#include "telnet/TelnetServer.hpp"

#include "Version.h"
#include "utils/ErrorHelpers.hpp"
#include "utils/Hasher.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

#include <sys/time.h>

// Invalid socket identifier for readability
constexpr int INVALID_SOCKET = -1;
// Receive buffer length
constexpr int DEFAULT_BUFLEN = 512;
// Timeout to automatic close session
constexpr int TELNET_TIMEOUT = 120;
// Maximum number of concurrent sessions
constexpr int MAX_AVAILABLE_SESSION = 5;
// History limit for Telnet session
constexpr int TELNET_HISTORY_LIMIT = 50;
// Sleep interval for the server (to control rate limiting)
constexpr int SLEEP_INTERVAL_MS = 50;

// Status table widths
constexpr int KEY_WIDTH = 30;
constexpr int VAL_WIDTH = 15;

// Telnet ASCII constants
constexpr int ASCII_LF = 0x0A;
constexpr int ASCII_NULL = 0x00;
constexpr int ASCII_NBSP = 0xFF;

// NOLINTBEGIN
const std::vector<std::pair<std::string, std::string>> telnetCommands = {
	{"clear", "Clears the terminal screen"},
	{"disable log", "Resets logger level"},
	{"enable log", R"(Enable specified logger level. Level can be "v" (info), "vv" (debug) and "vvv" (trace))"},
	{"help", "Prints available commands"},
	{"ping", "Pings the server"},
	{"status", "Checks the internal status"},
	{"version", "Displays the current version"},
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	{"quit", "Ends the connection"}};

const std::string ANSI_FG_BLACK("\x1b[30m");
const std::string ANSI_FG_RED("\x1b[31m");
const std::string ANSI_FG_GREEN("\x1b[32m");
const std::string ANSI_FG_YELLOW("\x1b[33m");
const std::string ANSI_FG_BLUE("\x1b[34m");
const std::string ANSI_FG_MAGENTA("\x1b[35m");
const std::string ANSI_FG_CYAN("\x1b[36m");
const std::string ANSI_FG_WHITE("\x1b[37m");
const std::string ANSI_FG_DEFAULT("\x1b[39m");

const std::string ANSI_BG_BLACK("\x1b[40m");
const std::string ANSI_BG_RED("\x1b[41m");
const std::string ANSI_BG_GREEN("\x1b[42m");
const std::string ANSI_BG_YELLOW("\x1b[43m");
const std::string ANSI_BG_BLUE("\x1b[44m");
const std::string ANSI_BG_MAGENTA("\x1b[45m");
const std::string ANSI_BG_CYAN("\x1b[46m");
const std::string ANSI_BG_WHITE("\x1b[47m");
const std::string ANSI_BG_DEFAULT("\x1b[49m");

const std::string ANSI_BOLD_ON("\x1b[1m");
const std::string ANSI_BOLD_OFF("\x1b[22m");

const std::string ANSI_ITALICS_ON("\x1b[3m");
const std::string ANSI_ITALCIS_OFF("\x1b[23m");

const std::string ANSI_UNDERLINE_ON("\x1b[4m");
const std::string ANSI_UNDERLINE_OFF("\x1b[24m");

const std::string ANSI_INVERSE_ON("\x1b[7m");
const std::string ANSI_INVERSE_OFF("\x1b[27m");

const std::string ANSI_STRIKETHROUGH_ON("\x1b[9m");
const std::string ANSI_STRIKETHROUGH_OFF("\x1b[29m");

const std::string ANSI_ERASE_LINE("\x1b[2K");
const std::string ANSI_ERASE_SCREEN("\x1b[2J");

const std::string ANSI_ARROW_UP("\x1b\x5b\x41");
const std::string ANSI_ARROW_DOWN("\x1b\x5b\x42");
const std::string ANSI_ARROW_RIGHT("\x1b\x5b\x43");
const std::string ANSI_ARROW_LEFT("\x1b\x5b\x44");

const std::string ANSI_DOUBLE_HORIZONTAL_TAB("\t\t");
const std::string ANSI_HORIZONTAL_TAB("\t");

const std::string TELNET_ERASE_LINE("\xff\xf8");
const std::string TELNET_CLEAR_SCREEN("\033[2J");
// NOLINTEND

std::string TelnetSession::getPeerIP() const
{
	sockaddr_in client_info{};
	memset(&client_info, 0, sizeof(client_info));
	socklen_t addrsize = sizeof(client_info);
	getpeername(m_socket, reinterpret_cast<sockaddr *>(&client_info), &addrsize);

	std::array<char, INET_ADDRSTRLEN> ipAddr{};
	inet_ntop(AF_INET, &client_info.sin_addr, ipAddr.data(), INET_ADDRSTRLEN);

	return ipAddr.data();
}

void TelnetSession::sendPromptAndBuffer()
{
	// Output the prompt
	ssize_t sendBytes =
		send(m_socket, m_telnetServer->promptString().c_str(), m_telnetServer->promptString().length(), 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	// Resend the buffer
	if (!m_buffer.empty())
	{
		sendBytes = send(m_socket, m_buffer.c_str(), m_buffer.length(), 0);
		if (sendBytes > 0)
		{
			stats.uploadBytes += static_cast<size_t>(sendBytes);
		}
	}
}

void TelnetSession::eraseLine()
{
	// Send an erase line
	ssize_t sendBytes = send(m_socket, ANSI_ERASE_LINE.c_str(), ANSI_ERASE_LINE.length(), 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	// Move the cursor to the beginning of the line
	const std::string moveBack = "\x1b[80D";
	sendBytes = send(m_socket, moveBack.c_str(), moveBack.length(), 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}
}

void TelnetSession::sendLine(std::string data)
{
	// If is something is on the prompt, wipe it off
	if (m_telnetServer->interactivePrompt() || !m_buffer.empty())
	{
		eraseLine();
	}

	data.append("\r\n");
	if (auto sendBytes = send(m_socket, data.c_str(), data.length(), 0) > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	if (m_telnetServer->interactivePrompt())
	{
		sendPromptAndBuffer();
	}
}

void TelnetSession::closeClient()
{
	spdlog::info("Telnet connection to {} closed", getPeerIP());

	// Attempt to cleanly shutdown the connection since we're done
	shutdown(m_socket, SHUT_WR);

	// Cleanup
	close(m_socket);

	// Set disconnect time
	stats.disconnectTime = std::chrono::high_resolution_clock::now();
}

bool TelnetSession::checkTimeout() const
{
	return (llabs(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastSeenTime)
					  .count()) > TELNET_TIMEOUT);
}

void TelnetSession::markTimeout()
{
	lastSeenTime = std::chrono::system_clock::time_point(std::chrono::duration<int>(0));
}

void TelnetSession::echoBack(const char *buffer, unsigned long length)
{
	// If you are an NVT command (i.e. first it of data is 255) then ignore the echo back
	if (static_cast<uint8_t>(*buffer) == ASCII_NBSP)
	{
		return;
	}

	const ssize_t sendBytes = send(m_socket, buffer, length, 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}
}

void TelnetSession::initialise()
{
	// Get details of connection
	spdlog::info("Telnet connection received from {}", getPeerIP());

	stats.connectTime = std::chrono::high_resolution_clock::now();

	// Set the connection to be non-blocking
	unsigned long iMode = 1;
	ioctl(m_socket, FIONBIO, &iMode);

	// Set NVT mode to say that I will echo back characters.
	const std::array<uint8_t, 3> willEcho{0xff, 0xfb, 0x01};
	ssize_t sendBytes = send(m_socket, willEcho.data(), 3, 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	// Set NVT requesting that the remote system not/dont echo back characters
	const std::array<uint8_t, 3> dontEcho{0xff, 0xfe, 0x01};
	sendBytes = send(m_socket, dontEcho.data(), 3, 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	// Set NVT mode to say that I will suppress go-ahead. Stops remote clients from doing local linemode.
	const std::array<uint8_t, 3> willSGA{0xff, 0xfb, 0x03};
	sendBytes = send(m_socket, willSGA.data(), 3, 0);
	if (sendBytes > 0)
	{
		stats.uploadBytes += static_cast<size_t>(sendBytes);
	}

	if (m_telnetServer->connectedCallback())
	{
		m_telnetServer->connectedCallback()(shared_from_this());
	}

	// Set last seen
	lastSeenTime = std::chrono::system_clock::now();
}

void TelnetSession::stripNVT(std::string &buffer)
{
	size_t found = 0;
	do
	{
		found = buffer.find_first_of(static_cast<char>(ASCII_NBSP));
		if (found != std::string::npos && (found + 2) <= buffer.length() - 1)
		{
			buffer.erase(found, 3);
		}
		else if ((found + 2) >= buffer.length())
		{
			break;
		}
	} while (found != std::string::npos);
}

void TelnetSession::stripEscapeCharacters(std::string &buffer)
{
	size_t found = 0;
	const std::array<std::string, 4> cursors = {ANSI_ARROW_UP, ANSI_ARROW_DOWN, ANSI_ARROW_RIGHT, ANSI_ARROW_LEFT};

	for (const auto &cursor : cursors)
	{
		do
		{
			found = buffer.find(cursor);
			if (found != std::string::npos)
			{
				buffer.erase(found, cursor.length());
			}
		} while (found != std::string::npos);
	}
}

bool TelnetSession::processBackspace(std::string &buffer)
{
	bool foundBackspaces = false;

	size_t found = 0;
	do
	{
		// Need to handle both \x7f and \b backspaces
		found = buffer.find_first_of('\x7f');
		if (found == std::string::npos)
		{
			found = buffer.find_first_of('\b');
		}

		if (found != std::string::npos)
		{
			if (buffer.length() > 1 && (found > 0))
			{
				buffer.erase(found - 1, 2);
			}
			else
			{
				buffer = "";
			}
			foundBackspaces = true;
		}
	} while (found != std::string::npos);
	return foundBackspaces;
}

bool TelnetSession::processTab(std::string &buffer)
{
	bool foundTabs = false;

	size_t found = 0;
	do
	{
		found = buffer.find_first_of('\t');
		if (found == std::string::npos)
		{
			continue;
		}

		// Remove single tab
		if (!buffer.empty())
		{
			buffer.erase(found, 1);
		}
		foundTabs = true;

		// Process
		if (m_telnetServer->tabCallback())
		{
			const std::string retCommand = m_telnetServer->tabCallback()(shared_from_this(), buffer.substr(0, found));
			if (!retCommand.empty())
			{
				buffer.erase(0, found);
				buffer.insert(0, retCommand);
			}
		}
	} while (found != std::string::npos);
	return foundTabs;
}

void TelnetSession::addToHistory(const std::string &line)
{
	// Add it to the history
	if (line != (!m_history.empty() ? m_history.back() : "") && !line.empty())
	{
		m_history.push_back(line);
		if (m_history.size() > TELNET_HISTORY_LIMIT)
		{
			m_history.pop_front();
		}
	}
	m_historyCursor = m_history.end();
}

bool TelnetSession::processCommandHistory(std::string &buffer)
{
	// Handle up and down arrow actions
	if (m_telnetServer->interactivePrompt())
	{
		if (buffer.find(ANSI_ARROW_UP) != std::string::npos && !m_history.empty())
		{
			if (m_historyCursor != m_history.begin())
			{
				--m_historyCursor;
			}
			buffer = *m_historyCursor;

			// Issue a cursor command to counter it
			ssize_t sendBytes = 0;
			if ((sendBytes = send(m_socket, ANSI_ARROW_DOWN.c_str(), ANSI_ARROW_DOWN.length(), 0)) < 0)
			{
				return false;
			}
			stats.uploadBytes += static_cast<size_t>(sendBytes);
			return true;
		}
		if (buffer.find(ANSI_ARROW_DOWN) != std::string::npos && !m_history.empty())
		{
			if (next(m_historyCursor) != m_history.end())
			{
				++m_historyCursor;
			}
			buffer = *m_historyCursor;

			// Issue a cursor command to counter it
			ssize_t sendBytes = 0;
			if ((sendBytes = send(m_socket, ANSI_ARROW_UP.c_str(), ANSI_ARROW_UP.length(), 0)) < 0)
			{
				return false;
			}

			stats.uploadBytes += static_cast<size_t>(sendBytes);
			return true;
		}

		// Ignore left and right and just reprint buffer
		if (buffer.find(ANSI_ARROW_LEFT) != std::string::npos || buffer.find(ANSI_ARROW_RIGHT) != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

std::vector<std::string> TelnetSession::getCompleteLines(std::string &buffer)
{
	// Now find all new lines (<CR><LF>) and place in a vector and delete from buffer
	std::vector<std::string> lines;

	size_t found = 0;
	do
	{
		found = buffer.find("\r\n");
		if (found != std::string::npos)
		{
			lines.push_back(buffer.substr(0, found));
			buffer.erase(0, found + 2);
		}
	} while (found != std::string::npos);

	return lines;
}

void TelnetSession::update()
{
	ssize_t readBytes = 0;
	std::array<char, DEFAULT_BUFLEN> recvbuf{};

	// Reset stats
	stats.uploadBytes = 0;
	stats.downloadBytes = 0;
	stats.successCmdCtr = 0;
	stats.failCmdCtr = 0;

	// Receive
	readBytes = recv(m_socket, recvbuf.data(), DEFAULT_BUFLEN, 0);

	// Check for errors from the read
	if (readBytes < 0 && errno != EAGAIN)
	{
		close(m_socket);
	}
	else if (readBytes > 0)
	{
		stats.downloadBytes += static_cast<size_t>(readBytes);

		// Update last seen
		lastSeenTime = std::chrono::system_clock::now();

		// Echo it back to the sender
		echoBack(recvbuf.data(), static_cast<size_t>(readBytes));

		// we've got to be careful here. Telnet client might send null characters for New Lines mid-data block. We need
		// to swap these out. recv is not null terminated, so its cool
		std::replace_if(
			recvbuf.begin(), recvbuf.begin() + readBytes, [](char chr) { return chr == ASCII_NULL; }, ASCII_LF);

		// Add it to the received buffer
		m_buffer.append(recvbuf.data(), static_cast<unsigned int>(readBytes));
		// Remove telnet negotiation sequences
		stripNVT(m_buffer);

		bool requirePromptReprint = false;
		if (m_telnetServer->interactivePrompt())
		{
			// Read up and down arrow keys and scroll through history
			if (processCommandHistory(m_buffer))
			{
				requirePromptReprint = true;
			}
			stripEscapeCharacters(m_buffer);

			// Remove characters
			if (processBackspace(m_buffer))
			{
				requirePromptReprint = true;
			}
			// Complete commands
			if (processTab(m_buffer))
			{
				requirePromptReprint = true;
			}
		}

		// Process commands
		auto lines = getCompleteLines(m_buffer);
		for (const auto &line : lines)
		{
			if (!m_telnetServer->newLineCallBack())
			{
				break;
			}

			m_telnetServer->newLineCallBack()(shared_from_this(), line) ? ++stats.successCmdCtr : ++stats.failCmdCtr;
			addToHistory(line);
		}

		if (requirePromptReprint && m_telnetServer->interactivePrompt())
		{
			eraseLine();
			sendPromptAndBuffer();
		}
	}
}

/* ------------------ Telnet Server -------------------*/

TelnetServer::~TelnetServer()
{
	try
	{
		shutdown();
	}
	catch (const std::exception &e)
	{
		try
		{
			spdlog::error("Telnet server destructor thrown an exception: {}", e.what());
		}
		catch (const std::exception &e2)
		{
			std::cerr << "Telnet server destructor and also logger thrown an exception: " << e.what() << '\n'
					  << e2.what() << '\n';
		}
	}
}

bool TelnetServer::initialise(unsigned long listenPort, const std::shared_ptr<std::atomic_flag> &checkFlag,
							  std::string promptString, const std::shared_ptr<prometheus::Registry> &reg,
							  const std::string &prependName)
{
	if (m_initialised)
	{
		return false;
	}

	m_checkFlag = checkFlag;
	m_listenPort = listenPort;
	m_promptString = std::move(promptString);

	addrinfo hints{};
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	addrinfo *result = nullptr;
	if (getaddrinfo(nullptr, std::to_string(m_listenPort).c_str(), &hints, &result) != 0)
	{
		return false;
	}

	// Create a SOCKET for connecting to server
	m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_listenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		return false;
	}

	if (int optionVal = 1; setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &optionVal, sizeof(optionVal)) < 0)
	{
		freeaddrinfo(result);
		close(m_listenSocket);
		return false;
	}

	// Setup the TCP listening socket
	if (bind(m_listenSocket, result->ai_addr, result->ai_addrlen) < 0)
	{
		freeaddrinfo(result);
		close(m_listenSocket);
		return false;
	}
	freeaddrinfo(result);

	if (listen(m_listenSocket, SOMAXCONN) < 0)
	{
		close(m_listenSocket);
		return false;
	}

	// If prometheus registry is provided prepare statistics
	if (reg)
	{
		m_stats = std::make_unique<TelnetStats>(reg, listenPort, prependName);
	}

	m_shouldStop.clear();
	m_serverThread = std::make_unique<std::thread>(&TelnetServer::threadFunc, this);

	m_initialised = true;
	return true;
}

bool TelnetServer::acceptConnection()
{
	const Socket ClientSocket = accept(m_listenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET)
	{
		return false;
	}
	if (m_sessions.size() >= MAX_AVAILABLE_SESSION)
	{
		// Create for only sending error
		const auto session = std::make_shared<TelnetSession>(ClientSocket, shared_from_this());
		session->initialise();

		session->sendLine("Too many active connections. Please try again later. \r\nClosing...");
		session->closeClient();
		return false;
	}

	const auto session = std::make_shared<TelnetSession>(ClientSocket, shared_from_this());
	m_sessions.push_back(session);
	session->initialise();
	return true;
}

void TelnetServer::threadFunc() noexcept
{
	spdlog::info("Telnet server started");
	while (!m_shouldStop._M_i)
	{
		try
		{
			update();
			if (m_checkFlag)
			{
				m_checkFlag->test_and_set();
			}
		}
		catch (const std::exception &e)
		{
			spdlog::error("Telnet server failed: {}", e.what());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_INTERVAL_MS));
	}
	spdlog::info("Telnet server stopped");
}

void TelnetServer::update()
{
	// See if connection pending on the listening socket
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(m_listenSocket, &readSet);
	timeval timeout{};
	timeout.tv_sec = 0; // Zero timeout (poll)
	timeout.tv_usec = 0;

	bool newConnectionAccept = false;
	bool newConnectionRefused = false;

	TelnetServerStats serverStats;
	serverStats.processingTimeStart = std::chrono::high_resolution_clock::now();

	// If there is a connection pending, accept it.
	if (select(m_listenSocket + 1, &readSet, nullptr, nullptr, &timeout) > 0)
	{
		if (acceptConnection())
		{
			newConnectionAccept = true;
		}
		else
		{
			newConnectionRefused = true;
		}
	}

	// Update all the telnet Sessions that are currently in flight.
	for (size_t idx = 0; idx < m_sessions.size(); ++idx)
	{
		// Update session
		m_sessions[idx]->update();

		// Close session if needed
		if (m_sessions[idx]->checkTimeout())
		{
			m_sessions[idx]->closeClient();
			if (m_stats)
			{
				m_stats->consumeStats(m_sessions[idx]->stats, true);
			}
			m_sessions.erase(m_sessions.begin() + static_cast<int>(idx));
			--idx;
		}
		else
		{
			if (m_stats)
			{
				m_stats->consumeStats(m_sessions[idx]->stats, false);
			}
		}
	}

	serverStats.activeConnectionCtr = m_sessions.size();
	serverStats.acceptedConnectionCtr = static_cast<uint64_t>(newConnectionAccept);
	serverStats.refusedConnectionCtr = static_cast<uint64_t>(newConnectionRefused);
	serverStats.processingTimeEnd = std::chrono::high_resolution_clock::now();
	if (m_stats)
	{
		m_stats->consumeStats(serverStats);
	}
}

void TelnetServer::shutdown()
{
	// Attempt to cleanly close every telnet session in flight.
	for (const SP_TelnetSession &tSession : m_sessions)
	{
		tSession->closeClient();
	}
	m_sessions.clear();

	// No longer need server socket so close it.
	close(m_listenSocket);
	m_listenSocket = INVALID_SOCKET;
	m_initialised = false;

	m_shouldStop.test_and_set();
	if (m_serverThread && m_serverThread->joinable())
	{
		m_serverThread->join();
		m_serverThread.reset();
	}
}

void TelnetPrintAvailableCommands(const SP_TelnetSession &session)
{
	// Print available commands
	session->sendLine("");
	session->sendLine("Available commands:");
	session->sendLine("");
	for (const auto &[command, info] : telnetCommands)
	{
		std::array<char, BUFSIZ> buffer{'\0'};
		if (snprintf(buffer.data(), BUFSIZ, "%-25s : %s", command.c_str(), info.c_str()) > 0)
		{
			session->sendLine(buffer.data());
		}
	}
}

void TelnetConnectedCallback(const SP_TelnetSession &session)
{
	session->sendLine("\r\n"
					  "𝑲𝒆𝒆𝒑 𝒚𝒐𝒖𝒓 𝒆𝒚𝒆𝒔 𝒐𝒏 𝒕𝒉𝒆 𝒔𝒕𝒂𝒓𝒔 "
					  "𝒂𝒏𝒅 𝒚𝒐𝒖𝒓 𝒇𝒆𝒆𝒕 𝒐𝒏 𝒕𝒉𝒆 𝒈𝒓𝒐𝒖𝒏𝒅 "
					  "\r\n");
	TelnetPrintAvailableCommands(session);
}

bool TelnetMessageCallback(const SP_TelnetSession &session, const std::string &line)
{
	spdlog::trace("Received message {}", line);

	// Send received message for user terminal
	session->sendLine(line);

	if (line.empty())
	{
		return true;
	}

	// Process received message
	switch (constHasher(line.c_str()))
	{
	case constHasher("Test Message"):
		session->sendLine("OK");
		return true;
	case constHasher("help"):
		TelnetPrintAvailableCommands(session);
		return true;
	case constHasher("disable log"):
		session->sendLine("Default log mode enabled");
		spdlog::set_level(spdlog::level::info);
		return true;
	case constHasher("disable log all"): // Internal use only
		session->sendLine("Disabling all logs");
		spdlog::set_level(spdlog::level::off);
		return true;
	case constHasher("enable log v"):
		session->sendLine("Info log mode enabled");
		spdlog::set_level(spdlog::level::info);
		return true;
	case constHasher("enable log vv"):
		session->sendLine("Debug log mode enabled");
		spdlog::set_level(spdlog::level::debug);
		return true;
	case constHasher("enable log vvv"):
		session->sendLine("Trace log mode enabled");
		spdlog::set_level(spdlog::level::trace);
		return true;
	case constHasher("ping"):
		session->sendLine("pong");
		return true;
	case constHasher("version"):
		session->sendLine(PROJECT_FULL_VERSION_STRING);
		return true;
	case constHasher("clear"):
		session->sendLine(TELNET_CLEAR_SCREEN);
		return true;
	case constHasher("status"):
		for (const auto &[service, statusFlag] : vCheckFlag)
		{
			std::ostringstream oss;
			oss << std::left << std::setfill('.') << std::setw(KEY_WIDTH) << service + " " << std::setw(VAL_WIDTH)
				<< std::right << (statusFlag->_M_i ? " OK" : " Not Active");
			session->sendLine(oss.str());
		}
		return true;
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	case constHasher("quit"):
		session->sendLine("Closing connection");
		session->sendLine("Goodbye!");
		session->markTimeout();
		return true;
	default:
		session->sendLine("Unknown command received");
		return false;
	}
}

std::string TelnetTabCallback(const SP_TelnetSession &session, std::string_view line)
{
	std::string retval;

	size_t ctr = 0;
	std::ostringstream sStream;
	for (const auto &[command, info] : telnetCommands)
	{
		if (command.rfind(line, 0) == 0)
		{
			++ctr;
			retval = command;
			sStream << command << std::setw(KEY_WIDTH);
		}
	}
	// Send suggestions if found any. If there is only one command retval will invoke completion
	if (ctr != 1 && (!sStream.str().empty()))
	{
		session->sendLine(sStream.str());
		retval = "";
	}

	return retval;
}
