#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>
#include <string>

namespace spdlog::sinks
{
	// NOLINTBEGIN
	/**
	 * A sink for sending log messages to a Sentry server.
	 *
	 * This sink is used to send log messages to a Sentry server for error tracking and monitoring.
	 * It provides an interface for sending log messages and flushing the sink.
	 *
	 * @tparam Mutex The type of mutex to use for thread safety.
	 */
	template <typename Mutex> class sentry_api_sink : public base_sink<Mutex> {
	  public:
		/**
		 * Constructs a Sentry API sink with the specified Sentry server address.
		 *
		 * @param sentryAddress The address of the Sentry server.
		 */
		explicit sentry_api_sink(const std::string &sentryAddress);

		/**
		 * Destroys the Sentry API sink.
		 */
		~sentry_api_sink();

	  protected:
		/**
		 * Sends the log message to the Sentry server.
		 *
		 * This function is called for each log message that needs to be sent to the Sentry server.
		 *
		 * @param msg The log message to be sent.
		 */
		void sink_it_(const details::log_msg &msg) override;

		/**
		 * Flushes the sink.
		 *
		 * This function is called to flush any buffered log messages to the Sentry server.
		 */
		void flush_() override;

	  private:
		bool _sentryAvailable{false}; /**< Flag indicating if the Sentry server is available. */
	};

	using sentry_api_sink_mt = sentry_api_sink<std::mutex>; /**< Type alias for Sentry API sink with mutex. */
	using sentry_api_sink_st =
		sentry_api_sink<details::null_mutex>; /**< Type alias for Sentry API sink without mutex. */

	// NOLINTEND
} // namespace spdlog::sinks
