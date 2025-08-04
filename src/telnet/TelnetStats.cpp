#include "telnet/TelnetStats.hpp"

#include <date/date.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/summary.h>

#include <limits>

TelnetStats::TelnetStats(const std::shared_ptr<prometheus::Registry> &reg, uint16_t portNumber,
						 const std::string &prependName)
	: BaseServerStats()
{
	if (!reg)
	{
		throw std::invalid_argument("Can't init Telnet statistics. Registry is null");
	}

	const auto name = prependName.empty() ? "telnet_" : prependName + "_telnet_";

	// Stats from base class
	initBaseStats(reg, name);

	// Basic information
	_infoFamily =
		&prometheus::BuildInfo().Name(name.substr(0, name.size() - 1)).Help("Telnet server information").Register(*reg);

	_infoFamily->Add({{"server_port", std::to_string(portNumber)}});
	_infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															   std::chrono::high_resolution_clock::now()))}});
	_infoFamily->Add({{"performance_unit", "nanoseconds"}});

	// Connection stats
	_activeConnection = &prometheus::BuildGauge()
							 .Name(name + "active_connections")
							 .Help("Number of active connections")
							 .Register(*reg)
							 .Add({});
	_refusedConnection = &prometheus::BuildCounter()
							  .Name(name + "refused_connections")
							  .Help("Number of refused connections")
							  .Register(*reg)
							  .Add({});
	_totalConnection = &prometheus::BuildCounter()
							.Name(name + "received_connections")
							.Help("Number of received connections")
							.Register(*reg)
							.Add({});

	// Bandwidth stats
	_totalUploadBytes =
		&prometheus::BuildCounter().Name(name + "uploaded_bytes").Help("Total uploaded bytes").Register(*reg).Add({});
	_totalDownloadBytes = &prometheus::BuildCounter()
							   .Name(name + "downloaded_bytes")
							   .Help("Total downloaded bytes")
							   .Register(*reg)
							   .Add({});

	// Session durations
	_sessionDuration = &prometheus::BuildSummary()
							.Name(name + "session_duration")
							.Help("Duration of sessions")
							.Register(*reg)
							.Add({}, QUANTILE_DEFAULTS);
	_maxSessionDuration = &prometheus::BuildGauge()
							   .Name(name + "maximum_session_duration")
							   .Help("Maximum duration of sessions")
							   .Register(*reg)
							   .Add({});
	_minSessionDuration = &prometheus::BuildGauge()
							   .Name(name + "minimum_session_duration")
							   .Help("Minimum duration of sessions")
							   .Register(*reg)
							   .Add({});

	// Set defaults
	_minSessionDuration->Set(std::numeric_limits<int>::max());
}

void TelnetStats::consumeStats(const TelnetSessionStats &stat, bool sessionClosed)
{
	consumeBaseStats(stat.successCmdCtr, stat.failCmdCtr, 0);

	_totalUploadBytes->Increment(static_cast<double>(stat.uploadBytes));
	_totalDownloadBytes->Increment(static_cast<double>(stat.downloadBytes));

	if (sessionClosed)
	{
		// Session durations
		const auto sessionTime = static_cast<double>(
			std::chrono::duration_cast<std::chrono::seconds>(stat.disconnectTime - stat.connectTime).count());
		_sessionDuration->Observe(sessionTime);
		if (sessionTime < _minSessionDuration->Value())
		{
			_minSessionDuration->Set(sessionTime);
		}
		if (sessionTime > _maxSessionDuration->Value())
		{
			_maxSessionDuration->Set(sessionTime);
		}
	}
}

void TelnetStats::consumeStats(const TelnetServerStats &stat)
{
	// Connection stats
	_activeConnection->Set(static_cast<double>(stat.activeConnectionCtr));
	_refusedConnection->Increment(static_cast<double>(stat.refusedConnectionCtr));
	_totalConnection->Increment(static_cast<double>(stat.acceptedConnectionCtr));

	// Performance stats if there is an active connection
	if (stat.activeConnectionCtr > 0)
	{
		consumeBaseStats(0, 0, static_cast<double>((stat.processingTimeEnd - stat.processingTimeStart).count()));
	}
}
