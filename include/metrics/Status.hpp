#pragma once

#include <prometheus/registry.h>

/**
 * @class StatusTracker
 * Class for tracking the status of events and counting successes and failures.
 */
class StatusTracker {
  private:
	prometheus::Gauge *_activeCtr;	  ///< Active number of events
	prometheus::Counter *_totalCtr;	  ///< Total number of counters
	prometheus::Counter *_successCtr; ///< Number of success
	prometheus::Counter *_failedCtr;  ///< Number of fail

  public:
	/**
	 * Construct a new Status Tracker
	 * @param[in] reg Registry to prometheus
	 * @param[in] name Name of the metric
	 * @param[in] metricID ID to append to metric names
	 */
	StatusTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name, uint64_t metricID = 0);

	/**
	 * Increment number of current events
	 */
	void incrementActive();

	/**
	 * Decrement number of current events
	 */
	void decrementActive();

	/**
	 * Increment number of success
	 */
	void incrementSuccess();

	/**
	 * Increment number of fail
	 */
	void incrementFail();
};

/**
 * RAII style wrapper for StatusTracker
 */
class TrackStatus {
  private:
	StatusTracker &_tracker;

  public:
	explicit TrackStatus(StatusTracker &tracker) : _tracker(tracker) { _tracker.incrementActive(); }
	~TrackStatus() { _tracker.decrementActive(); }

	/// Copy constructor
	TrackStatus(const TrackStatus & /*unused*/) = delete;

	/// Move constructor
	TrackStatus(TrackStatus && /*unused*/) = delete;

	/// Copy assignment operator
	TrackStatus &operator=(TrackStatus /*unused*/) = delete;

	/// Move assignment operator
	TrackStatus &operator=(TrackStatus && /*unused*/) = delete;
};
