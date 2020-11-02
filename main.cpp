#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <stdio.h>
#include <vector>
#include <memory>

#define SPDLOG_DISABLE_DEFAULT_LOGGER
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef __FILE_NAME__
#define F __FILE_NAME__
#else
#define F SPDLOG_FILENAME_T(__FILE__)
#endif
#define L __LINE__
#define FU __FUNCTION__
#define MY_ERROR(MSG) std::runtime_error(formatException(F, L, FU, MSG))

class MyException : std::exception
{
};

class MyLogger final
{
public:
    static inline void InitMyDefaultLogger(const std::string &logger_name = default_logger, const std::string &log_f = log_file)
    {
        if ((logger_sinks.empty()) && (spdlog::default_logger() == nullptr))
        {
            logger_sinks.reserve(2);
            addNewFileSink(log_f);
            addNewConsoleSink();
            registerNewLoggerInstance(logger_name);
            spdlog::set_default_logger(spdlog::get(logger_name));
        }
    }
    static inline void TestThrows()
    {
        try
        {
            throw1();
        }
        catch (const std::exception &some_exception)
        {
            logExceptions(some_exception);
        }
    }
    static inline void ShowLogFile()
    {
        spdlog::get(default_logger)->flush();
        std::ifstream logging_file(log_file);
        if (logging_file.is_open())
        {
            std::cout << logging_file.rdbuf();
        }
    }

protected:
    static inline void addNewFileSink(const std::string &f_name)
    {
        auto file_sink{std::make_shared<spdlog::sinks::basic_file_sink_mt>(f_name, true)};
        file_sink->set_level(spdlog::level::trace);
        logger_sinks.emplace_back(file_sink);
    }
    static inline void addNewConsoleSink()
    {
        auto console_sink{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
        console_sink->set_level(spdlog::level::info);
        logger_sinks.emplace_back(console_sink);
    }
    static inline void registerNewLoggerInstance(const std::string &logger_name)
    {
        auto logger_instance{spdlog::get(logger_name)};
        if (logger_instance == nullptr)
        {
            auto logger_instance = std::make_shared<spdlog::logger>(logger_name, std::begin(logger_sinks), std::end(logger_sinks));
            logger_instance->set_level(spdlog::level::trace);
            spdlog::register_logger(logger_instance);
        }
    }
    static inline std::string formatException(const char *f, const int l, const char *pf, const std::string &msg)
    {
        std::stringstream fmt_buffer;
        fmt_buffer << f << ":" << l << ":" << pf << ", " << msg;
        return fmt_buffer.str();
    }
    static inline void logThem(const std::exception &ex, int indent)
    {
        std::string exception_log_line{std::string(static_cast<unsigned long>(indent), ' ') + "Exception: "};
        SPDLOG_TRACE("{} {}", exception_log_line, ex.what());
        try
        {
            rethrow_if_nested(ex);
        }
        catch (const std::exception &_ex)
        {
            logThem(_ex, indent + 1);
        }
        catch (...)
        {
        }
    }
    static inline void throw3()
    {
        throw MyException();
    }
    static inline void throw2()
    {
        try
        {
            throw3();
        }
        catch (...)
        {
            std::throw_with_nested(MY_ERROR("Throw 2 failed because of Throw 3"));
        }
    }
    static inline void throw1()
    {
        try
        {
            throw2();
        }
        catch (...)
        {
            std::throw_with_nested(MY_ERROR("Throw 1 failed because of Throw 2"));
        }
    }
    static inline void logExceptions(const std::exception &ex)
    {
        spdlog::enable_backtrace(backtrace_count);
        logThem(ex, 0);
        spdlog::dump_backtrace();
        spdlog::disable_backtrace();
    }

private:
    static inline std::vector<spdlog::sink_ptr> logger_sinks;
    static constexpr char log_file[] = "test_logging.txt";
    static constexpr char default_logger[] = "my_default";
    static constexpr int backtrace_count{10};
};

int main()
{
    MyLogger::InitMyDefaultLogger();

    std::cout << "LOGGING STATEMENTS TEST: BEGIN" << std::endl;
    SPDLOG_INFO("This is INFO level logging");
    SPDLOG_DEBUG("This is DEBUG level logging");
    SPDLOG_WARN("This is WARNING level logging");
    SPDLOG_ERROR("This is ERROR level logging");
    SPDLOG_TRACE("This is TRACE level logging");
    std::cout << "LOGGING STATEMENTS TEST: END" << std::endl;
    std::cout << std::endl;

    std::cout << "LOGGING EXCEPTIONS TEST: BEGIN" << std::endl;
    MyLogger::TestThrows();
    std::cout << "LOGGING EXCEPTIONS TEST: END" << std::endl;
    std::cout << std::endl;

    std::cout << "LOG FILE OUTPUT" << std::endl;
    MyLogger::ShowLogFile();

    return 0;
}
