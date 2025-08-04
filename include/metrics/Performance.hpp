#pragma once

#include <prometheus/registry.h>

/**
 * @class PerformanceTracker
 * Measures and calculates performance metrics.
 *
 * The PerformanceTracker class is responsible for measuring and calculating performance metrics.
 * It provides functionality to start and stop a timer, and calculates the elapsed time between the start and stop
 * events. The class uses the prometheus library to store and manage the performance metrics.
 */
class PerformanceTracker {
  private:
	std::chrono::high_resolution_clock::time_point _startTime; ///< Set after startTimer to measure counter difference
	prometheus::Summary *_perfTiming;						   ///< Overall performance
	prometheus::Gauge *_maxTiming;							   ///< Maximum observed value
	prometheus::Gauge *_minTiming;							   ///< Minimum observed value

  public:
	/**
	 * Construct a new PerformanceTracker object.
	 * @param[in] reg The registry to register the performance metrics.
	 * @param[in] name The name of the metric.
	 * @param[in] metricID The ID to append to metric names.
	 */
	PerformanceTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
					   uint64_t metricID = 0);

	/**
	 * Starts the timer.
	 */
	void startTimer();

	/**
	 * Ends the timer and updates internal statistics.
	 * @return The result of the timer in nanoseconds.
	 */
	double endTimer();
};

/**
 * @class TrackPerformance
 * RAII style wrapper for PerformanceTracker.
 *
 * The TrackPerformance class is a RAII (Resource Acquisition Is Initialization) style wrapper for the
 * PerformanceTracker class. It automatically starts the timer when constructed and stops the timer when destructed.
 * This ensures that the timer is always stopped, even in case of exceptions or early returns.
 * The class is non-copyable and non-movable to prevent unintended behavior.
 */
class TrackPerformance {
  private:
	PerformanceTracker &_tracker; ///< Reference to the PerformanceTracker object.

  public:
	/**
	 * Constructs a new TrackPerformance object.
	 * @param[in] tracker The PerformanceTracker object to track.
	 */
	explicit TrackPerformance(PerformanceTracker &tracker) : _tracker(tracker) { _tracker.startTimer(); }

	/**
	 * Destructs the TrackPerformance object and stops the timer.
	 */
	~TrackPerformance() { _tracker.endTimer(); }

	// Non-copyable and non-movable
	TrackPerformance(const TrackPerformance & /*unused*/) = delete;
	TrackPerformance(TrackPerformance && /*unused*/) = delete;
	TrackPerformance &operator=(const TrackPerformance & /*unused*/) = delete;
	TrackPerformance &operator=(TrackPerformance && /*unused*/) = delete;
};
