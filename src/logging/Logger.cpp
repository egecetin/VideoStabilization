#include "logging/Logger.hpp"

#include "Version.h"
#include "logging/Loki.hpp"
#include "logging/Sentry.hpp"

#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/spdlog.h>

#include <unistd.h>

// Log filtering interval for duplicate filtering in seconds
constexpr int LOG_FILTER_SECS = 5;
// Log flush interval in seconds
constexpr int LOG_FLUSH_SECS = 2;

MainLogger::MainLogger(const std::string &lokiAddr, const std::string &sentryAddr)
{
	spdlog::set_level(spdlog::level::off);

	// Prepare spdlog loggers
	auto dupFilter = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(LOG_FILTER_SECS));
	if (getppid() != 1) // Disable stdout output for systemd
	{
		dupFilter->add_sink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	}
	dupFilter->add_sink(std::make_shared<spdlog::sinks::syslog_sink_mt>(PROJECT_NAME, LOG_USER, 0, false));
	dupFilter->add_sink(std::make_shared<spdlog::sinks::loki_api_sink_mt>(lokiAddr));
	dupFilter->add_sink(std::make_shared<spdlog::sinks::sentry_api_sink_mt>(sentryAddr));

	// Register main logger
	_mainLogger = std::make_shared<spdlog::logger>(PROJECT_NAME, dupFilter);

	spdlog::set_default_logger(_mainLogger);
	spdlog::flush_every(std::chrono::seconds(LOG_FLUSH_SECS));

#ifdef NDEBUG
	spdlog::set_level(spdlog::level::warn);
#else
	spdlog::set_level(spdlog::level::info);
#endif

	PRINT_VERSION();
}

MainLogger::~MainLogger()
{
	spdlog::info("Goodbye!");
	_mainLogger->flush();
	spdlog::drop(_mainLogger->name());

	// Set default logger
	spdlog::set_default_logger(
		std::make_shared<spdlog::logger>("default", std::make_shared<spdlog::sinks::stdout_color_sink_mt>()));
}
