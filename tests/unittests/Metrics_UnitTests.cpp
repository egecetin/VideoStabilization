#include "test-static-definitions.h"

#include "metrics/Performance.hpp"
#include "metrics/ProcessMetrics.hpp"
#include "metrics/PrometheusServer.hpp"
#include "metrics/Status.hpp"

#include <fstream>
#include <thread>

#include <gtest/gtest.h>

bool isAllValuesExist(std::ifstream &promFile, const std::vector<std::string> &testVals,
					  std::vector<std::string> &readVals)
{
	for (size_t idx = 0; idx < testVals.size(); ++idx)
	{
		while (!promFile.eof())
		{
			std::string line;
			std::getline(promFile, line);
			if (line.rfind(testVals[idx], 0) == 0)
			{
				readVals.push_back(line.substr(line.find(' ')));

				promFile.clear();
				promFile.seekg(0);
				break;
			}
		}

		if (promFile.eof())
			return false;
	}

	return true;
}

TEST(Metrics_Tests, PrometheusServerUnitTests)
{
	std::string promServerAddr = "localhost:8100";

	PrometheusServer reporter(promServerAddr);

	ASSERT_NE(reporter.createNewRegistry(), nullptr);

	uint64_t regId = 10; // Just random number
	auto reg2 = reporter.createNewRegistry(regId);
	ASSERT_EQ(regId, 1);
	ASSERT_NE(reg2, nullptr);
	ASSERT_EQ(reporter.getRegistry(regId), reg2);
	ASSERT_EQ(reporter.getRegistry(1000), nullptr);

	ASSERT_FALSE(reporter.deleteRegistry(std::numeric_limits<uint64_t>::max()));
	ASSERT_TRUE(reporter.deleteRegistry(regId));
	ASSERT_TRUE(reporter.deleteRegistry(regId));
}

TEST(Metrics_Tests, PerformanceTrackerUnitTests)
{
	std::string promServerAddr = "localhost:8101";

	PrometheusServer reporter(promServerAddr);
	PerformanceTracker perfTracker(reporter.createNewRegistry(), "test_performance", 0);

	perfTracker.startTimer();
	std::this_thread::sleep_for(std::chrono::milliseconds(150));
	perfTracker.endTimer();
	{
		TrackPerformance guard(perfTracker);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system((std::string("curl ") + promServerAddr + "/metrics --output metrics2.prom").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {"test_performance_processing_time_0",
										   "test_performance_maximum_processing_time_0",
										   "test_performance_minimum_processing_time_0"};

	// Parse output
	std::ifstream promFileStream("metrics2.prom");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_TRUE(std::stoi(readValues[0]));			// test_performance_processing_time_0
	ASSERT_GT(std::stoi(readValues[1]), 145 * 1e6); // test_performance_maximum_processing_time_0
	ASSERT_LE(std::stoi(readValues[2]), 55 * 1e6);	// test_performance_minimum_processing_time_0
}

TEST(Metrics_Tests, StatusTrackerUnitTests)
{
	std::string promServerAddr = "localhost:8102";

	PrometheusServer reporter(promServerAddr);
	StatusTracker statTracker(reporter.createNewRegistry(), "test_status", 2);

	statTracker.incrementActive();
	statTracker.incrementActive();
	statTracker.incrementActive();
	{
		TrackStatus guard(statTracker);
	}
	statTracker.incrementSuccess();
	statTracker.incrementFail();

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system((std::string("curl ") + promServerAddr + "/metrics --output metrics3.prom").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {"test_status_total_event_ctr_2", "test_status_success_event_ctr_2",
										   "test_status_fail_event_ctr_2", "test_status_active_event_ctr_2"};

	// Parse output
	std::ifstream promFileStream("metrics3.prom");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_EQ(2, std::stoi(readValues[0])); // test_status_total_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[1])); // test_status_success_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[2])); // test_status_fail_event_ctr_2
	ASSERT_EQ(1, std::stoi(readValues[3])); // test_status_active_event_ctr_2
}

TEST(Metrics_Tests, ProcessMetricsUnitTests)
{
	std::string promServerAddr = "localhost:8103";

	PrometheusServer reporter(promServerAddr);
	std::shared_ptr<std::atomic_flag> checkFlag = std::make_shared<std::atomic_flag>(false);
	ProcessMetrics procMetrics(checkFlag, reporter.createNewRegistry());

	// Collect data from socket
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	ASSERT_FALSE(system((std::string("curl ") + promServerAddr + "/metrics --output metrics4.prom").c_str()));

	// Metrics
	std::vector<std::string> readValues;
	std::vector<std::string> testValues = {
		"memory_usage", "page_faults", "cpu_usage", "disk_read", "disk_write", "thread_count", "file_descriptor_count"};

	// Parse output
	std::ifstream promFileStream("metrics4.prom");

	ASSERT_TRUE(promFileStream.is_open());
	ASSERT_TRUE(isAllValuesExist(promFileStream, testValues, readValues));
	ASSERT_EQ(testValues.size(), readValues.size());

	ASSERT_GT(std::stod(readValues[0]), 0); // memory_usage
	ASSERT_GE(std::stod(readValues[1]), 0); // page_faults
	ASSERT_GE(std::stod(readValues[2]), 0); // cpu_usage
	ASSERT_GE(std::stod(readValues[3]), 0); // disk_read
	ASSERT_GE(std::stod(readValues[4]), 0); // disk_write
	ASSERT_GT(std::stod(readValues[5]), 0); // thread_count
	ASSERT_GT(std::stod(readValues[6]), 0); // file_descriptor_count
}
