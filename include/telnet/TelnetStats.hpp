#pragma once

#include "utils/BaseServerStats.hpp"

#include <prometheus/registry.h>

/**
 * Telnet session statistics
 */
struct TelnetSessionStats {
	std::chrono::high_resolution_clock::time_point connectTime;	   ///< Connection start time
	std::chrono::high_resolution_clock::time_point disconnectTime; ///< Connection end time
	size_t uploadBytes{};										   ///< Uploaded bytes
	size_t downloadBytes{};										   ///< Downloaded bytes
	uint64_t successCmdCtr{};									   ///< Successful commands
	uint64_t failCmdCtr{};										   ///< Failed commands
};

/**
 * Telnet server statistics
 */
struct TelnetServerStats {
	std::chrono::high_resolution_clock::time_point processingTimeStart; ///< Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeEnd;	///< Processing time end
	uint64_t activeConnectionCtr{};										///< Number of active connections
	uint64_t acceptedConnectionCtr{};									///< Number of accepted connections
	uint64_t refusedConnectionCtr{};									///< Number of refused connections
};

/**
 * Prometheus statistics for Telnet server
 */
class TelnetStats : private BaseServerStats {
  private:
	prometheus::Family<prometheus::Info> *_infoFamily; ///< Information metric family
	prometheus::Gauge *_activeConnection;			   ///< Number of active connections
	prometheus::Counter *_refusedConnection;		   ///< Number of refused connections
	prometheus::Counter *_totalConnection;			   ///< Number of total received connections
	prometheus::Counter *_totalUploadBytes;			   ///< Total uploaded bytes
	prometheus::Counter *_totalDownloadBytes;		   ///< Total downloaded bytes
	prometheus::Summary *_sessionDuration;			   ///< Value of the duration of sessions
	prometheus::Gauge *_maxSessionDuration;			   ///< Maximum duration of sessions
	prometheus::Gauge *_minSessionDuration;			   ///< Minimum duration of sessions

  public:
	/**
	 * Construct a new Telnet statistics
	 * @param[in] reg Prometheus registry
	 * @param[in] portNumber Telnet server port
	 * @param[in] prependName Prefix for Prometheus stats
	 */
	TelnetStats(const std::shared_ptr<prometheus::Registry> &reg, uint16_t portNumber,
				const std::string &prependName = "");

	/**
	 * Updates statistics with session values
	 * @param[in] stat Statistics values from session
	 * @param[in] sessionClosed True if the provided session is closed
	 */
	void consumeStats(const TelnetSessionStats &stat, bool sessionClosed);

	/**
	 * Updates statistics with server values
	 * @param[in] stat Statistics values from server
	 */
	void consumeStats(const TelnetServerStats &stat);
};
