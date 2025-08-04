#include "utils/FileHelpers.hpp"

#include "utils/ErrorHelpers.hpp"

#include <fcntl.h>
#include <iostream>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

/// Sleep interval for the file monitor
constexpr int SLEEP_INTERVAL_MS = 50;

void FileMonitor::threadFunc() const noexcept
{
	while (!_shouldStop._M_i)
	{
		// Buffer for reading events
		unsigned int nBytes = 0;
		if (ioctl(_fDescriptor, FIONREAD, &nBytes) < 0)
		{
			spdlog::error("Failed to get available events for file monitoring: {}", getErrnoString(errno));
		}

		auto buffer = std::vector<char>(nBytes + 1, '\0');
		auto nRead = read(_fDescriptor, buffer.data(), nBytes);
		if (nRead < 0 && errno != EAGAIN)
		{
			spdlog::error("Failed to read events for file monitoring: {}", getErrnoString(errno));
		}
		else if (nRead == 0)
		{
			spdlog::debug("No events read for file monitoring");
		}

		ssize_t idx = 0;
		while (_notifyCallback && idx < nRead)
		{
			const auto *event = reinterpret_cast<inotify_event *>(&buffer[static_cast<size_t>(idx)]);

			// Check if file notify type matches
			if ((event->mask & _notifyEvents) != 0)
			{
				_notifyCallback(_userPtr);
				break;
			}

			idx += static_cast<ssize_t>(sizeof(inotify_event) + event->len);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_INTERVAL_MS));
	}
}

FileMonitor::FileMonitor(std::filesystem::path filePath, uint32_t notifyEvents)
	: _fDescriptor(inotify_init()), _filePath(std::move(filePath)), _notifyEvents(notifyEvents)
{
	if (_fDescriptor < 0)
	{
		throw std::ios_base::failure("Failed to initialize inotify");
	}

	_wDescriptor = inotify_add_watch(_fDescriptor, _filePath.c_str(), notifyEvents);
	if (_wDescriptor < 0)
	{
		close(_fDescriptor);
		throw std::ios_base::failure("Failed to add watch descriptor");
	}

	if (fcntl(_fDescriptor, F_SETFL, fcntl(_fDescriptor, F_GETFL) | O_NONBLOCK) < 0)
	{
		close(_fDescriptor);
		throw std::ios_base::failure("Failed to set file descriptor to non-blocking mode");
	}

	_thread = std::make_unique<std::thread>(&FileMonitor::threadFunc, this);
}

FileMonitor::~FileMonitor()
{
	_shouldStop.test_and_set();
	if (_thread && _thread->joinable())
	{
		_thread->join();
		_thread.reset();
	}

	// Remove watch descriptor first
	if (_wDescriptor >= 0)
	{
		if (inotify_rm_watch(_fDescriptor, _wDescriptor) < 0)
		{
			try
			{
				spdlog::error("Failed to remove watch descriptor: {}", getErrnoString(errno));
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to remove watch descriptor and also logger thrown an exception: "
						  << getErrnoString(errno) << " " << e.what() << '\n';
			}
		}
		_wDescriptor = -1;
	}

	// Then close the file descriptor
	if (_fDescriptor >= 0)
	{
		if (close(_fDescriptor) < 0)
		{
			try
			{
				spdlog::error("Failed to close file descriptor: {}", getErrnoString(errno));
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to close file descriptor and also logger thrown an exception: "
						  << getErrnoString(errno) << " " << e.what() << '\n';
			}
		}
		_fDescriptor = -1;
	}
}
