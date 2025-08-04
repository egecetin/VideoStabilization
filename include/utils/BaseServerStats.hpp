#pragma once

#include <prometheus/registry.h>

#define QUANTILE_DEFAULTS                                                                                              \
	prometheus::Summary::Quantiles { {0.5, 0.1}, {0.9, 0.1}, {0.99, 0.1} }

/**
 * @class BaseServerStats
 * Represents the base statistics for a server.
 */
class BaseServerStats {
  private:
	prometheus::Summary *_processingTime{nullptr};	 ///< Value of the command processing performance
	prometheus::Gauge *_maxProcessingTime{nullptr};	 ///< Maximum value of the command processing performance
	prometheus::Gauge *_minProcessingTime{nullptr};	 ///< Minimum value of the command processing performance
	prometheus::Counter *_succeededCommand{nullptr}; ///< Number of succeeded commands
	prometheus::Counter *_failedCommand{nullptr};	 ///< Number of failed commands
	prometheus::Counter *_totalCommand{nullptr};	 ///< Number of total received commands

  protected:
	void initBaseStats(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name);

	void consumeBaseStats(uint64_t succeeded, uint64_t failed, double processingTime);
};
