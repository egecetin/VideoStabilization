#pragma once

#include <atomic>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <sys/inotify.h>
#include <thread>
#include <vector>

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @param[out] lastWord Last word (space delimiter) of the first found line
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern,
											 std::string &lastWord)
{
	const std::regex regExp(pattern);
	std::ifstream inFile(filePath);
	std::vector<std::string> matchedLines;

	std::string readLine;
	while (getline(inFile, readLine))
	{
		if (std::regex_search(readLine, regExp))
		{
			matchedLines.push_back(readLine);
		}
	}

	if (!matchedLines.empty())
	{
		auto pos = matchedLines.front().find_last_of(' ');
		if (pos != std::string::npos && pos != matchedLines.front().size())
		{
			lastWord = matchedLines.front().substr(pos + 1);
		}
	}

	return matchedLines;
}

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern)
{
	std::string lastWord;
	return findFromFile(filePath, pattern, lastWord);
}

/// Callback function for file notifications
using FNotifyCallback = std::function<void(const void *)>;

/**
 * Invokes functions for a file for given notify events
 */
class FileMonitor {
  private:
	/// File descriptor
	int _fDescriptor{-1};
	/// Watch descriptor
	int _wDescriptor{-1};
	/// File path
	std::filesystem::path _filePath;
	/// Callback function
	FNotifyCallback _notifyCallback;
	/// Notify types
	uint32_t _notifyEvents;
	/// User pointer
	const void *_userPtr = nullptr;

	/// Thread
	std::unique_ptr<std::thread> _thread;
	/// Flag to stop monitoring
	std::atomic_flag _shouldStop{false};

	void threadFunc() const noexcept;

  public:
	/**
	 * Constructor
	 * @param[in] filePath Path to the file
	 * @param[in] notifyEvents Events to notify
	 */
	explicit FileMonitor(std::filesystem::path filePath, uint32_t notifyEvents = IN_MODIFY);

	/// @brief Copy constructor
	FileMonitor(const FileMonitor & /*unused*/) = delete;

	/// @brief Move constructor
	FileMonitor(FileMonitor && /*unused*/) = delete;

	/// @brief Copy assignment operator
	FileMonitor &operator=(FileMonitor /*unused*/) = delete;

	/// @brief Move assignment operator
	FileMonitor &operator=(FileMonitor && /*unused*/) = delete;

	[[nodiscard]] FNotifyCallback notifyCallback() const { return _notifyCallback; }
	void notifyCallback(FNotifyCallback func) { _notifyCallback = std::move(func); }

	/**
	 * Sets user pointer
	 * @param[in] ptr User pointer
	 */
	void userPtr(const void *ptr) { _userPtr = ptr; }

	/**
	 * Destructor
	 */
	~FileMonitor();
};
