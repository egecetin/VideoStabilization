#include "utils/ConfigParser.hpp"
#include "utils/ErrorHelpers.hpp"
#include "utils/FileHelpers.hpp"
#include "utils/InputParser.hpp"
#include "utils/Tracer.hpp"

#include "test-static-definitions.h"

#include <gtest/gtest.h>

TEST(Utils_Tests, ConfigParserUnitTests)
{
	// Copy original file to prevent modifying the original file
	std::ifstream srcFile(TEST_CONFIG_PATH, std::ios::binary);
	std::ofstream dstFile(TEST_CONFIG_PATH_COPY, std::ios::binary);
	dstFile << srcFile.rdbuf();
	dstFile.close();
	ASSERT_FALSE(srcFile.fail());
	ASSERT_FALSE(dstFile.fail());

	ConfigParser parser(TEST_CONFIG_PATH_COPY);

	ASSERT_EQ(parser.get("TELNET_PORT"), "23000");
	ASSERT_EQ(parser.get("LOKI_ADDRESS"), "http://localhost:8400");
	ASSERT_EQ(parser.get("NonExistentKey"), "");

	ASSERT_NO_THROW(parser.load());
	ASSERT_GT(parser.getConfigMap().size(), 0);

	ASSERT_NO_THROW(parser.set("NEW_KEY", "NEW_VALUE"));
	ASSERT_EQ(parser.get("NEW_KEY"), "NEW_VALUE");
	ASSERT_NO_THROW(parser.load());
	ASSERT_EQ(parser.get("NEW_KEY"), "");

	ASSERT_NO_THROW(parser.set("NEW_KEY", "NEW_VALUE"));
	ASSERT_NO_THROW(parser.save());

	ConfigParser parser2(TEST_CONFIG_PATH_COPY);
	ASSERT_EQ(parser2.get("NEW_KEY"), "NEW_VALUE");

	ASSERT_NO_THROW(parser.remove("NEW_KEY"));
	ASSERT_EQ(parser.get("NEW_KEY"), "");
	ASSERT_NO_THROW(parser.save());

	ASSERT_EQ(parser2.get("NEW_KEY"), "NEW_VALUE");
	ASSERT_NO_THROW(parser2.load());
	ASSERT_EQ(parser2.get("NEW_KEY"), "");

	ASSERT_THROW(ConfigParser(""), std::invalid_argument);
	ASSERT_THROW(ConfigParser(TEST_CONFIG_EMPTY_PATH), std::invalid_argument);
}

TEST(Utils_Tests, ErrorHelpersUnitTests)
{
	ASSERT_EQ(getErrnoString(0), "Success");
	ASSERT_EQ(getErrnoString(EACCES), "Permission denied");
}

TEST(Utils_Tests, FileHelpersUnitTests)
{
	auto readLines = findFromFile(TEST_DATA_READ_PATH, "^(cpu family)");
	ASSERT_EQ(readLines.size(), 2);
	ASSERT_EQ(readLines[0], "cpu family      : 6");
	ASSERT_EQ(readLines[1], "cpu family      : 6");

	std::string word;
	readLines = findFromFile(TEST_DATA_READ_PATH, "^processor", word);
	ASSERT_EQ(readLines.size(), 2);
	ASSERT_EQ(word, "6");
	ASSERT_EQ(readLines[0], "processor       : 6");
	ASSERT_EQ(readLines[1], "processor       : 7");

	readLines = findFromFile(TEST_DATA_READ_PATH, "^dummy", word);
	ASSERT_TRUE(readLines.empty());

	FileMonitor monitor(TEST_DATA_READ_PATH, IN_OPEN);
	int val = 0xFF;
	monitor.userPtr(&val);

	bool isInvoked = false;
	bool isAccessed = false;
	monitor.notifyCallback([&isInvoked, &isAccessed](const void *ptr) {
		isInvoked = true;
		if (ptr)
		{
			isAccessed = *(static_cast<const int *>(ptr)) == 0xFF;
		}
	});

	std::ifstream file(TEST_DATA_READ_PATH);
	ASSERT_TRUE(file.is_open());

	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	ASSERT_TRUE(isInvoked);
	ASSERT_TRUE(isAccessed);
}

TEST(Utils_Tests, InputParserUnitTests)
{
	// Test
	int nArgc = 4;
	char *nArgv[] = {"program", "argument1", "option1", "--argument2", NULL};
	InputParser parser(nArgc, nArgv);

	ASSERT_EQ("", parser.getCmdOption("program"));
	ASSERT_FALSE(parser.cmdOptionExists("program"));

	ASSERT_EQ("", parser.getCmdOption("unknownArg"));
	ASSERT_FALSE(parser.cmdOptionExists("unknownArg"));

	ASSERT_EQ("option1", parser.getCmdOption("argument1"));
	ASSERT_TRUE(parser.cmdOptionExists("argument1"));

	ASSERT_EQ("", parser.getCmdOption("--argument2"));
	ASSERT_TRUE(parser.cmdOptionExists("--argument2"));

	auto options = parser.getCmdOptions();
	ASSERT_EQ(options.size(), 1);
	ASSERT_EQ(options[0].first, "--argument2");
	ASSERT_EQ(options[0].second, "");
}

#ifndef XXX_ENABLE_MEMLEAK_CHECK
// Google tracer client does not support destroying tracer completely
// This is a workaround to avoid memory leak detection issues with the Google tracer client.
// The tracer client is not designed to be destroyed, so we skip this test when memory leak
// detection is enabled.
TEST(Utils_Tests, TracerUnitTests)
{
	{
		Tracer tracer(std::make_shared<std::atomic_flag>(false), "", "", TEST_CRASHPAD_EXECUTABLE_PATH,
					  TEST_CRASHPAD_REPORT_DIR);

		ASSERT_TRUE(tracer.isRunning());
	}
}
#endif // XXX_ENABLE_MEMLEAK_CHECK
