//
// Created by Fx Kaze on 25-1-16.
//

#ifndef LOGGER_H
#define LOGGER_H

#define LOG_DEBUG(...) Logger::log(Logger::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::log(Logger::INFO, __VA_ARGS__)
#define LOG_NOTICE(...) Logger::log(Logger::NOTICE, __VA_ARGS__)
#define LOG_WARNING(...) Logger::log(Logger::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(Logger::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) Logger::log(Logger::FATAL, __VA_ARGS__)

#define LOG_NAMED(level, name, ...) Logger::logNamed(level, name, __VA_ARGS__)
#define LOG_NAMED_DEBUG(name, ...) LOG_NAMED(Logger::DEBUG, name, __VA_ARGS__)
#define LOG_NAMED_INFO(name, ...) LOG_NAMED(Logger::INFO, name, __VA_ARGS__)
#define LOG_NAMED_NOTICE(name, ...) LOG_NAMED(Logger::NOTICE, name, __VA_ARGS__)
#define LOG_NAMED_WARNING(name, ...) LOG_NAMED(Logger::WARNING, name, __VA_ARGS__)
#define LOG_NAMED_ERROR(name, ...) LOG_NAMED(Logger::ERROR, name, __VA_ARGS__)
#define LOG_NAMED_FATAL(name, ...) LOG_NAMED(Logger::FATAL, name, __VA_ARGS__)

namespace Logger {
    enum LogLevel {
        DEBUG = 0,
        INFO,
        NOTICE,
        WARNING,
        ERROR,
        FATAL,
        OFF
    };

    int setLogPathAndOpen(const char *logPath = nullptr);

    const char *getLogPath();

    void setLogLevel(LogLevel logLevel);

    LogLevel getLogLevel();

    int init();

    void ptr();

    void noPrt();

    void log(LogLevel logLevel, const char *format, ...);

    void logNamed(LogLevel logLevel, const char *name, const char *format, ...);
};

#endif //LOGGER_H
