#include "logging/Logger.hpp"
#include "metrics/ProcessMetrics.hpp"
#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "utils/ConfigParser.hpp"
#include "utils/ErrorHelpers.hpp"
#include "utils/InputParser.hpp"
#include "utils/Tracer.hpp"
#include "zeromq/ZeroMQServer.hpp"

#include <curl/curl.h>
#include <spdlog/spdlog.h>

#include <csignal>

// SIGALRM interval in seconds
constexpr uintmax_t alarmInterval = 30;

namespace
{
	// Interruption flag
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	volatile sig_atomic_t interruptFlag = 0;

	// Default SIGALRM function
	void alarmFunc(int /*unused*/)
	{
		alarm(alarmInterval);

		// Clear all flags
		std::for_each(vCheckFlag.begin(), vCheckFlag.end(), [](auto &entry) { entry.second->clear(); });
	}

	void interruptFunc(int /*unused*/) { interruptFlag = 1; }
} // namespace

int main(int argc, char **argv)
{
	const InputParser input(argc, argv);
	const ConfigParser config(input.cmdOptionExists("--config") ? input.getCmdOption("--config") : "config.json");
	const MainLogger logger(config.get("LOKI_ADDRESS"), config.get("SENTRY_ADDRESS"));

	// Initialize curl as soon as possible
	if (curl_global_init(CURL_GLOBAL_DEFAULT) < 0)
	{
		spdlog::critical("Can't init curl");
		return EXIT_FAILURE;
	}

	// Adjust log level
	if (input.cmdOptionExists("-v"))
	{
		spdlog::set_level(spdlog::level::info);
	}
	if (input.cmdOptionExists("-vv"))
	{
		spdlog::set_level(spdlog::level::debug);
	}
	if (input.cmdOptionExists("-vvv"))
	{
		spdlog::set_level(spdlog::level::trace);
	}

	// Log input arguments
	spdlog::debug("======== Detected input arguments ========");
	for (const auto &entry : input.getCmdOptions())
	{
		spdlog::debug("{} = {}", entry.first, entry.second);
	}

	// Log detected configuration to debug
	spdlog::debug("======== Detected configuration ========");
	for (const auto &entry : config.getConfigMap())
	{
		spdlog::debug("{} = {}", entry.first, entry.second);
	}

	// Register interrupt signal handler
	if (std::signal(SIGINT, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGINT): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}

	// Register termination signal handler
	if (std::signal(SIGTERM, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGTERM): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}

	// Register alarm signal handler
	if (std::signal(SIGALRM, alarmFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGALRM): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}
	alarm(alarmInterval);

	// Initialize Crashpad handler
	std::unique_ptr<Tracer> crashpadController(nullptr);
	vCheckFlag.emplace_back("Crashpad Handler", std::make_shared<std::atomic_flag>(false));
	crashpadController = std::make_unique<Tracer>(
		vCheckFlag[vCheckFlag.size() - 1].second, config.get("CRASHPAD_REMOTE"), config.get("CRASHPAD_PROXY"),
		config.get("CRASHPAD_EXECUTABLE_PATH"), config.get("CRASHPAD_REPORT_DIR"));

	// Initialize Prometheus server
	std::unique_ptr<PrometheusServer> mainPrometheusServer(nullptr);
	const std::string prometheusAddr = input.getCmdOption("--enable-prometheus");
	if (!prometheusAddr.empty())
	{
		try
		{
			mainPrometheusServer = std::make_unique<PrometheusServer>(prometheusAddr);
			spdlog::info("Prometheus server start at {}", prometheusAddr);
		}
		catch (const std::exception &e)
		{
			spdlog::error("Can't start Prometheus Server: {}", e.what());
			return EXIT_FAILURE;
		}
	}

	// Initialize self monitoring
	std::unique_ptr<ProcessMetrics> selfMonitor(nullptr);
	vCheckFlag.emplace_back("Self Monitor", std::make_shared<std::atomic_flag>(false));
	if (mainPrometheusServer)
	{
		selfMonitor = std::make_unique<ProcessMetrics>(vCheckFlag[vCheckFlag.size() - 1].second,
													   mainPrometheusServer->createNewRegistry());
	}

	// Initialize ZeroMQ server
	std::unique_ptr<ZeroMQServer> zmqController(nullptr);
	vCheckFlag.emplace_back("ZeroMQ Server", std::make_shared<std::atomic_flag>(false));
	const std::string zeromqServerAddr = input.getCmdOption("--enable-zeromq");
	if (!zeromqServerAddr.empty())
	{
		try
		{
			zmqController = std::make_unique<ZeroMQServer>(
				zeromqServerAddr, vCheckFlag[vCheckFlag.size() - 1].second,
				mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);
			zmqController->messageCallback(ZeroMQServerMessageCallback);
			zmqController->initialise();
		}
		catch (const std::exception &e)
		{
			spdlog::error("Can't start ZeroMQ Server: {}", e.what());
			return EXIT_FAILURE;
		}
	}

	// Initialize Telnet server
	std::shared_ptr<TelnetServer> telnetController(nullptr);
	vCheckFlag.emplace_back("Telnet Server", std::make_shared<std::atomic_flag>(false));
	const unsigned long telnetPort =
		input.cmdOptionExists("--enable-telnet") ? std::stoul(input.getCmdOption("--enable-telnet")) : 0;
	if (telnetPort > 0 && telnetPort < 65536)
	{
		try
		{
			telnetController = std::make_shared<TelnetServer>();
			telnetController->connectedCallback(TelnetConnectedCallback);
			telnetController->newLineCallback(TelnetMessageCallback);
			telnetController->tabCallback(TelnetTabCallback);
			telnetController->initialise(telnetPort, vCheckFlag[vCheckFlag.size() - 1].second, "> ",
										 mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);
		}
		catch (const std::exception &e)
		{
			spdlog::error("Can't start Telnet Server: {}", e.what());
			return EXIT_FAILURE;
		}
	}
	else if (telnetPort != 0)
	{
		spdlog::error("Invalid Telnet port: {}", telnetPort);
		return EXIT_FAILURE;
	}

	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	while (interruptFlag == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	curl_global_cleanup();

	return EXIT_SUCCESS;
}
