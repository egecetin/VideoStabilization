#include "metrics/ProcessMetrics.hpp"

#include "utils/FileHelpers.hpp"

#include <spdlog/spdlog.h>

#include <dirent.h>
#include <sys/resource.h>

constexpr int SLEEP_INTERVAL_SEC = 1;

size_t ProcessMetrics::countDirectoryEntries(const std::string &path)
{
	DIR *dir = nullptr;
	size_t count = 0;
	if ((dir = opendir(path.c_str())) != nullptr)
	{
		while (readdir(dir) != nullptr) // NOLINT(concurrency-mt-unsafe)
		{
			++count;
		}
		closedir(dir);
	}

	return count;
}

long int ProcessMetrics::getMemoryUsage()
{
	struct rusage r_usage{};
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_maxrss; // NOLINT(cppcoreguidelines-pro-type-union-access)
}

long int ProcessMetrics::getPageFaults()
{
	struct rusage r_usage{};
	getrusage(RUSAGE_SELF, &r_usage);
	return r_usage.ru_majflt; // NOLINT(cppcoreguidelines-pro-type-union-access)
}

double ProcessMetrics::getCpuUsage()
{
	if (_oldCpuTime == 0)
	{
		_oldCpuTime = times(&_oldCpu);
		if (_oldCpuTime == static_cast<clock_t>(-1))
		{
			_oldCpuTime = 0;
		}
		return 0.0;
	}

	struct tms nowCpu{};
	auto nowCpuTime = times(&nowCpu);

	const double usage = 100.0 * static_cast<double>(nowCpu.tms_utime - _oldCpu.tms_utime) /
						 static_cast<double>(nowCpuTime - _oldCpuTime);

	std::swap(_oldCpu, nowCpu);
	std::swap(_oldCpuTime, nowCpuTime);

	return usage;
}

std::pair<size_t, size_t> ProcessMetrics::getDiskIO()
{
	std::string buffer;
	findFromFile("/proc/self/io", "read_bytes", buffer);
	size_t readBytes = buffer.empty() ? 0 : std::stoull(buffer);

	buffer.clear();
	findFromFile("/proc/self/io", "write_bytes", buffer);
	size_t writeBytes = buffer.empty() ? 0 : std::stoull(buffer);

	std::pair<size_t, size_t> result = {readBytes - _oldReadBytes, writeBytes - _oldWriteBytes};

	std::swap(_oldReadBytes, readBytes);
	std::swap(_oldWriteBytes, writeBytes);

	return result;
}

size_t ProcessMetrics::getThreadCount() { return countDirectoryEntries("/proc/self/task"); }

size_t ProcessMetrics::getFileDescriptorCount() { return countDirectoryEntries("/proc/self/fd"); }

void ProcessMetrics::update()
{
	_pCurrentTime->SetToCurrentTime();
	_pMemory->Set(static_cast<double>(getMemoryUsage()));
	_pPageFaults->Set(static_cast<double>(getPageFaults()));
	_pCpuUsage->Set(getCpuUsage());
	auto [diskRead, diskWrite] = getDiskIO();
	_pDiskRead->Set(static_cast<double>(diskRead));
	_pDiskWrite->Set(static_cast<double>(diskWrite));
	_pThreadCount->Set(static_cast<double>(getThreadCount()));
	_pFileDescriptorCount->Set(static_cast<double>(getFileDescriptorCount()));
}

void ProcessMetrics::threadRunner() noexcept
{
	while (!_shouldStop._M_i)
	{
		try
		{
			update();
			if (_checkFlag)
			{
				_checkFlag->test_and_set();
			}
		}
		catch (const std::exception &e)
		{
			spdlog::error("Self monitoring failed: {}", e.what());
		}

		std::this_thread::sleep_for(std::chrono::seconds(SLEEP_INTERVAL_SEC));
	}
}

ProcessMetrics::ProcessMetrics(std::shared_ptr<std::atomic_flag> checkFlag,
							   const std::shared_ptr<prometheus::Registry> &reg)
	: _checkFlag(std::move(checkFlag))
{
	if (reg == nullptr)
	{
		throw std::invalid_argument("Registry is nullptr");
	}

	_pInitTime =
		&prometheus::BuildGauge().Name("init_time").Help("Initialization time of application").Register(*reg).Add({});
	_pCurrentTime =
		&prometheus::BuildGauge().Name("current_time").Help("Current time of application").Register(*reg).Add({});
	_pMemory =
		&prometheus::BuildGauge().Name("memory_usage").Help("Memory usage of application").Register(*reg).Add({});
	_pPageFaults =
		&prometheus::BuildGauge().Name("page_faults").Help("Page faults of application").Register(*reg).Add({});
	_pCpuUsage = &prometheus::BuildGauge().Name("cpu_usage").Help("CPU usage of application").Register(*reg).Add({});
	_pDiskRead = &prometheus::BuildGauge().Name("disk_read").Help("Disk read of application").Register(*reg).Add({});
	_pDiskWrite = &prometheus::BuildGauge().Name("disk_write").Help("Disk write of application").Register(*reg).Add({});
	_pThreadCount =
		&prometheus::BuildGauge().Name("thread_count").Help("Thread count of application").Register(*reg).Add({});
	_pFileDescriptorCount = &prometheus::BuildGauge()
								 .Name("file_descriptor_count")
								 .Help("File descriptor count of application")
								 .Register(*reg)
								 .Add({});

	_pInitTime->SetToCurrentTime();

	_thread = std::make_unique<std::thread>(&ProcessMetrics::threadRunner, this);
}

ProcessMetrics::~ProcessMetrics()
{
	_shouldStop.test_and_set();
	if (_thread && _thread->joinable())
	{
		_thread->join();
	}
}
