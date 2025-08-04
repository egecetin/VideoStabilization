#include "metrics/Status.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

StatusTracker::StatusTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
							 uint64_t metricID)
{
	// Register values
	_activeCtr = &prometheus::BuildGauge()
					  .Name(name + "_active_event_ctr_" + std::to_string(metricID))
					  .Help("Currently active number of events of " + name)
					  .Register(*reg)
					  .Add({});
	_totalCtr = &prometheus::BuildCounter()
					 .Name(name + "_total_event_ctr_" + std::to_string(metricID))
					 .Help("Total occurrences of " + name)
					 .Register(*reg)
					 .Add({});
	_successCtr = &prometheus::BuildCounter()
					   .Name(name + "_success_event_ctr_" + std::to_string(metricID))
					   .Help("Successful events of " + name)
					   .Register(*reg)
					   .Add({});
	_failedCtr = &prometheus::BuildCounter()
					  .Name(name + "_fail_event_ctr_" + std::to_string(metricID))
					  .Help("Failed events of " + name)
					  .Register(*reg)
					  .Add({});
}

void StatusTracker::incrementActive() { _activeCtr->Increment(); }

void StatusTracker::decrementActive() { _activeCtr->Decrement(); }

void StatusTracker::incrementSuccess()
{
	_successCtr->Increment();
	_totalCtr->Increment();
	if (_activeCtr->Value() > 0)
	{
		_activeCtr->Decrement();
	}
}

void StatusTracker::incrementFail()
{
	_failedCtr->Increment();
	_totalCtr->Increment();
	if (_activeCtr->Value() > 0)
	{
		_activeCtr->Decrement();
	}
}
