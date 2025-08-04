#pragma once

#include "utils/BaseServerStats.hpp"

#include <prometheus/registry.h>
#include <zmq.hpp>

/**
 * @struct ZeroMQServerStats
 * Represents the statistics of a ZeroMQ server connection.
 */
struct ZeroMQServerStats {
	std::chrono::high_resolution_clock::time_point processingTimeStart; ///< Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeEnd;	///< Processing time end
	bool isSuccessful{false}; ///< Indicates if processing was successful for this connection
};

/**
 * @class ZeroMQStats
 * Represents the statistics of a ZeroMQ server.
 */
class ZeroMQStats : private BaseServerStats {
  private:
	prometheus::Family<prometheus::Info> *_infoFamily; ///< Information metric family
	prometheus::Counter *_succeededCommandParts;	   ///< Number of received succeeded message parts
	prometheus::Counter *_failedCommandParts;		   ///< Number of received failed message parts
	prometheus::Counter *_totalCommandParts;		   ///< Number of received total message parts
	prometheus::Counter *_totalUploadBytes;			   ///< Total uploaded bytes
	prometheus::Counter *_totalDownloadBytes;		   ///< Total downloaded bytes

  public:
	/**
	 * Construct a new ZeroMQStats object.
	 * @param[in] reg Prometheus registry.
	 * @param[in] prependName Prefix for Prometheus stats.
	 */
	explicit ZeroMQStats(const std::shared_ptr<prometheus::Registry> &reg, const std::string &prependName = "");

	/**
	 * Updates the statistics with messages.
	 * @param[in] recvMsgs Received messages.
	 * @param[in] sendMsgs Send messages.
	 * @param[in] serverStats ZeroMQ server stats.
	 */
	void consumeStats(const std::vector<zmq::message_t> &recvMsgs, const std::vector<zmq::message_t> &sendMsgs,
					  const ZeroMQServerStats &serverStats);
};
