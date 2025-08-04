#include "metrics/PrometheusServer.hpp"

#include "Version.h"

#include <date/date.h>
#include <prometheus/info.h>

#include <limits>

PrometheusServer::PrometheusServer(const std::string &serverAddr)
	: _mainExposer(std::make_unique<prometheus::Exposer>(serverAddr, 1))
{
	auto reg = std::make_shared<prometheus::Registry>();

	// Basic information
	_infoFamily =
		&prometheus::BuildInfo().Name(PROJECT_NAME).Help(std::string(PROJECT_NAME) + " information").Register(*reg);

	_infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															   std::chrono::high_resolution_clock::now()))}});
	_infoFamily->Add({{"version", PROJECT_FULL_REVISION}});

	_vRegister.emplace_back(std::numeric_limits<uint64_t>::max(), reg);
	_mainExposer->RegisterCollectable(reg);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::getRegistry(uint64_t regId)
{
	const std::scoped_lock guard(_guardLock);

	if (auto iter = std::find_if(_vRegister.begin(), _vRegister.end(),
								 [regId](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) {
									 return regId == val.first;
								 });
		iter != _vRegister.end())
	{
		return iter->second;
	}
	return nullptr;
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry()
{
	uint64_t regId = 0;
	return createNewRegistry(regId);
}

std::shared_ptr<prometheus::Registry> PrometheusServer::createNewRegistry(uint64_t &regId)
{
	const std::scoped_lock guard(_guardLock);

	// Create registry
	auto reg = std::make_shared<prometheus::Registry>();
	_mainExposer->RegisterCollectable(reg);

	// Push to vector (At least information registry always exist so back is valid)
	regId = _vRegister.back().first + 1;
	_vRegister.emplace_back(_vRegister.back().first + 1, reg);
	return reg;
}

bool PrometheusServer::deleteRegistry(uint64_t regId)
{
	if (regId == std::numeric_limits<uint64_t>::max())
	{
		return false;
	}

	const std::scoped_lock guard(_guardLock);

	_vRegister.erase(std::remove_if(
		_vRegister.begin(), _vRegister.end(),
		[regId](const std::pair<uint64_t, std::shared_ptr<prometheus::Registry>> &val) { return regId == val.first; }));

	return true;
}
