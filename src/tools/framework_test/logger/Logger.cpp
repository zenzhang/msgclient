// Logger.cpp

#include "tools/framework_test/Common.h"

#include <framework/configure/Config.h>
#include <framework/logger/LoggerListRecord.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::configure;
using namespace framework::logger;

#include <boost/preprocessor/cat.hpp>

std::vector<int> func()
{
    return std::vector<int>(2, 1);
}

#define  STRINGIFY(x) #x 
#define  TOSTRING(x) STRINGIFY(x) 

FRAMEWORK_LOGGER_DECLARE_MODULE("TestLogger");

static void test_logger(Config & conf)
{

    framework::configure::Config testConfig;
    testConfig.profile().set("Logger", "stream_count", "1");
    testConfig.profile().set("LogStream0", "file", "F://config//zz.log");
    testConfig.profile().set("LogStream0", "append", "false");
    testConfig.profile().set("LogStream0", "level", "7");
    testConfig.profile().set("LogStream0", "size", "0");
    framework::logger::global_logger().load_config(testConfig);

    LOG_S(Logger::kLevelEvent, "zenzhang1");
    LOG_S(Logger::kLevelEvent, "zenzhang2");
    LOG_S(Logger::kLevelEvent, "zenzhang3");
    /*int i = 1;
    float f = 2.0;
    std::string s = "str";
    char m[] = "str";

    LOG_L(0, (i)(f)(s)(func()));

    LOG_F(0, "(i=%i,f=%f,s=%s)" % i % f % s.c_str());

    LOG_S(0, "(i=" << i << ",f=" << f << ",s=" << s << ")");*/
}

static TestRegister test("logger", test_logger);
