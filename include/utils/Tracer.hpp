#pragma once

#include "client/crashpad_client.h"

#include <thread>

/**
 * Tracer class to handle operations of Crashpad
 */
class Tracer {
  private:
	std::unique_ptr<std::thread> _thread;
	std::atomic_flag _shouldStop{false};
	std::shared_ptr<std::atomic_flag> _checkFlag;

	std::string _serverPath;
	std::string _serverProxy;
	std::string _handlerPath;
	std::map<std::string, std::string> _annotations;
	std::vector<base::FilePath> _attachments;
	std::string _reportPath;
	std::unique_ptr<crashpad::CrashpadClient> _clientHandler;

	/**
	 * Start the crashpad handler process
	 */
	void startHandler();

	/**
	 * Thread function to check and restart the crashpad handler process
	 */
	void threadFunc() const noexcept;

	/**
	 * Check if the given process ID is running
	 * @param[in] processId The process ID to check
	 * @return true if the process is running, false otherwise
	 */
	static bool checkPidIsRunning(pid_t processId);

	/**
	 * Check if the given socket ID is running
	 * @param[in] sockId The socket ID to check
	 * @return true if the socket is running, false otherwise
	 */
	static bool checkSocketIsRunning(int sockId);

	/**
	 * Get the executable directory of the current application
	 * @return The executable directory path
	 */
	static inline std::string getSelfExecutableDir();

	/**
	 * Dump shared library information to a file
	 * @param[in] filePath File path to dump the information
	 */
	static void dumpSharedLibraryInfo(const std::string &filePath);

  public:
	/**
	 * Construct a new Tracer object
	 * @param[in] checkFlag Flag to check if the process is running
	 * @param[in] serverPath Remote server address
	 * @param[in] serverProxy Remote server proxy
	 * @param[in] reportPath Path to where dump minidump files
	 * @param[in] crashpadHandlerPath Path to crashpad_handler executable
	 * @param[in] attachments Attachments to add to the minidump
	 */
	explicit Tracer(std::shared_ptr<std::atomic_flag> checkFlag, std::string serverPath = "",
					std::string serverProxy = "", const std::string &crashpadHandlerPath = "",
					const std::string &reportPath = "", std::vector<base::FilePath> attachments = {});

	/// Copy constructor
	Tracer(const Tracer & /*unused*/) = delete;

	/// Move constructor
	Tracer(Tracer && /*unused*/) = delete;

	/// Copy assignment operator
	Tracer &operator=(Tracer /*unused*/) = delete;

	/// Move assignment operator
	Tracer &operator=(Tracer && /*unused*/) = delete;

	/**
	 * Destroy the Tracer object
	 */
	~Tracer();

	/**
	 * Check if the crashpad_handler process is running
	 * @return true if the crashpad_handler is running, false otherwise
	 */
	[[nodiscard]] static bool isRunning();
};
