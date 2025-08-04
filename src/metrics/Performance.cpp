#include "metrics/Performance.hpp"

#include <prometheus/gauge.h>
#include <prometheus/summary.h>

#define QUANTILE_DEFAULTS                                                                                              \
	prometheus::Summary::Quantiles { {0.5, 0.1}, {0.9, 0.1}, {0.99, 0.1} }

PerformanceTracker::PerformanceTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
									   uint64_t metricID)
{
	_perfTiming = &prometheus::BuildSummary()
					   .Name(name + "_processing_time_" + std::to_string(metricID))
					   .Help(name + " processing performance")
					   .Register(*reg)
					   .Add({}, QUANTILE_DEFAULTS);
	_maxTiming = &prometheus::BuildGauge()
					  .Name(name + "_maximum_processing_time_" + std::to_string(metricID))
					  .Help("Maximum value of the " + name + " processing performance")
					  .Register(*reg)
					  .Add({});
	_minTiming = &prometheus::BuildGauge()
					  .Name(name + "_minimum_processing_time_" + std::to_string(metricID))
					  .Help("Minimum value of the " + name + " processing performance")
					  .Register(*reg)
					  .Add({});

	_minTiming->Set(std::numeric_limits<int>::max());
}

void PerformanceTracker::startTimer() { _startTime = std::chrono::high_resolution_clock::now(); }

double PerformanceTracker::endTimer()
{
	const auto val = static_cast<double>((std::chrono::high_resolution_clock::now() - _startTime).count());

	_perfTiming->Observe(val);
	if (val < _minTiming->Value())
	{
		_minTiming->Set(val);
	}
	if (val > _maxTiming->Value())
	{
		_maxTiming->Set(val);
	}

	return val;
}
