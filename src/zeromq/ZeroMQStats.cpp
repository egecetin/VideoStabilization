#include "zeromq/ZeroMQStats.hpp"

#include <date/date.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/summary.h>

ZeroMQStats::ZeroMQStats(const std::shared_ptr<prometheus::Registry> &reg, const std::string &prependName)
	: BaseServerStats()
{
	if (!reg)
	{
		throw std::invalid_argument("Can't init ZeroMQ statistics. Registry is null");
	}

	const auto name = prependName.empty() ? "zeromq_" : prependName + "_zeromq_";

	// Init stats from base class
	initBaseStats(reg, name);

	// Basic information
	_infoFamily =
		&prometheus::BuildInfo().Name(name.substr(0, name.size() - 1)).Help("ZeroMQ server information").Register(*reg);

	_infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															   std::chrono::high_resolution_clock::now()))}});
	_infoFamily->Add({{"performance_unit", "nanoseconds"}});

	// Command stats
	_succeededCommandParts = &prometheus::BuildCounter()
								  .Name(name + "succeeded_command_parts")
								  .Help("Number of received succeeded message parts")
								  .Register(*reg)
								  .Add({});
	_failedCommandParts = &prometheus::BuildCounter()
							   .Name(name + "failed_command_parts")
							   .Help("Number of received failed message parts")
							   .Register(*reg)
							   .Add({});
	_totalCommandParts = &prometheus::BuildCounter()
							  .Name(name + "total_command_parts")
							  .Help("Number of received total message parts")
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
}

void ZeroMQStats::consumeStats(const std::vector<zmq::message_t> &recvMsgs, const std::vector<zmq::message_t> &sendMsgs,
							   const ZeroMQServerStats &serverStats)
{
	consumeBaseStats(static_cast<uint64_t>(serverStats.isSuccessful), static_cast<uint64_t>(!serverStats.isSuccessful),
					 static_cast<double>((serverStats.processingTimeEnd - serverStats.processingTimeStart).count()));

	for (const auto &entry : recvMsgs)
	{
		_totalCommandParts->Increment();
		_totalDownloadBytes->Increment(static_cast<double>(entry.size()));

		if (serverStats.isSuccessful)
		{
			_succeededCommandParts->Increment();
		}
		else
		{
			_failedCommandParts->Increment();
		}
	}

	for (const auto &entry : sendMsgs)
	{
		_totalUploadBytes->Increment(static_cast<double>(entry.size()));
	}
}
