#include "utils/Tracer.hpp"

#include "Version.h"

#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

constexpr int SLEEP_INTERVAL_MS = 50;

void Tracer::startHandler()
{
	// Path to crashpad executable
	const base::FilePath handler(_handlerPath);

	// Must be writable or crashpad_handler will crash
	const base::FilePath reportsDir(_reportPath);
	const base::FilePath metricsDir(_reportPath);

	// Initialize Crashpad database
	auto database = crashpad::CrashReportDatabase::Initialize(reportsDir);
	if (database == nullptr)
	{
		throw std::ios_base::failure("Can't initialize crash report database");
	}

	// Enable automated crash uploads
	auto *settings = database->GetSettings();
	if (settings == nullptr)
	{
		throw std::ios_base::failure("Can't get crash report database settings");
	}
	settings->SetUploadsEnabled(true);

	// Start crash handler
	if (!_clientHandler->StartHandler(handler, reportsDir, metricsDir, _serverPath, _serverProxy, _annotations,
									  {"--no-rate-limit"}, true, false, _attachments))
	{
		throw std::ios_base::failure("Can't start crash handler");
	}

	_thread = std::make_unique<std::thread>(&Tracer::threadFunc, this);
}

void Tracer::threadFunc() const noexcept
{
	while (!_shouldStop._M_i)
	{
		try
		{
			if (!isRunning())
			{
				spdlog::info("Crashpad handler closed");
				return;
			}
			if (_checkFlag)
			{
				_checkFlag->test_and_set();
			}
		}
		catch (const std::exception &e)
		{
			spdlog::error("Crashpad failed: {}", e.what());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_INTERVAL_MS));
	}
}

bool Tracer::checkPidIsRunning(pid_t processId) { return kill(processId, 0) == 0; }

bool Tracer::checkSocketIsRunning(int sockId)
{
	int error = 0;
	socklen_t len = sizeof(error);

	char buff = 0;
	const int result = getsockopt(sockId, SOL_SOCKET, SO_ERROR, &error, &len);
	return result == 0 && error == 0 && recv(sockId, &buff, 1, MSG_PEEK | MSG_DONTWAIT) != 0;
}

std::string Tracer::getSelfExecutableDir()
{
	std::array<char, FILENAME_MAX> pathBuffer{'\0'};
	auto bytes = readlink("/proc/self/exe", pathBuffer.data(), sizeof(pathBuffer));

	auto path = std::string(pathBuffer.data(), bytes == -1 ? 0 : static_cast<size_t>(bytes));
	auto lastDelimPos = path.find_last_of('/');
	return (lastDelimPos == std::string::npos) ? "" : path.substr(0, lastDelimPos);
}

bool Tracer::isRunning()
{
	int sockId{-1};
	pid_t processId{-1};

	if (!crashpad::CrashpadClient::GetHandlerSocket(&sockId, &processId))
	{
		return false;
	}

	if (sockId >= 0 && !checkSocketIsRunning(sockId))
	{
		return false;
	}
	if (processId > 0 && !checkPidIsRunning(processId))
	{
		return false;
	}
	return true;
}

void Tracer::dumpSharedLibraryInfo(const std::string &filePath)
{
	// Open the output file
	std::ofstream ofile(filePath);
	if (!ofile.is_open())
	{
		throw std::invalid_argument("Can't open file: " + filePath);
	}

	// Get the shared library information
	std::ifstream maps("/proc/self/maps");

	std::string line;
	while (std::getline(maps, line))
	{
		// The format of each line is: address perms offset dev inode pathname
		// We only care about the address and the pathname, which are the first and last fields
		std::istringstream iss(line);
		std::string address;
		std::string perms;
		std::string offset;
		std::string dev;
		std::string inode;
		std::string pathname;
		iss >> address >> perms >> offset >> dev >> inode >> pathname;

		// We only want the shared libraries, which have the .so extension and the read and execute permissions
		if (pathname.find(".so") != std::string::npos && perms.find("r-x") != std::string::npos)
		{
			// The address field is in the form of start-end, we only need the start address
			const std::string start = address.substr(0, address.find('-'));

			// Convert the start address from hexadecimal string to unsigned long
			const unsigned long addr = std::stoul(start, nullptr, 16);

			ofile << pathname << " " << addr << '\n';
		}
	}
}

Tracer::Tracer(std::shared_ptr<std::atomic_flag> checkFlag, std::string serverPath, std::string serverProxy,
			   const std::string &crashpadHandlerPath, const std::string &reportPath,
			   std::vector<base::FilePath> attachments)
	: _checkFlag(std::move(checkFlag)), _serverPath(std::move(serverPath)), _serverProxy(std::move(serverProxy)),
	  _attachments(std::move(attachments))
{
	auto selfDir = getSelfExecutableDir();

	_handlerPath = crashpadHandlerPath.empty() ? selfDir + "/crashpad_handler" : crashpadHandlerPath;
	_reportPath = reportPath.empty() ? selfDir : reportPath;
	_clientHandler = std::make_unique<crashpad::CrashpadClient>();

	_annotations = std::map<std::string, std::string>(
		{{"name", PROJECT_NAME},
		 {"version", PROJECT_FULL_REVISION},
		 {"build_info", PROJECT_BUILD_DATE + std::string(" ") + PROJECT_BUILD_TIME + std::string(" ") + BUILD_TYPE},
		 {"compiler_info", COMPILER_NAME + std::string(" ") + COMPILER_VERSION}});

	// Dump shared library information and add as attachment
	dumpSharedLibraryInfo(_reportPath + "/shared_libs.txt");
	_attachments.emplace_back(_reportPath + "/shared_libs.txt");

	startHandler();
}

Tracer::~Tracer()
{
	_shouldStop._M_i = true;
	if (_thread && _thread->joinable())
	{
		_thread->join();
	}
}
