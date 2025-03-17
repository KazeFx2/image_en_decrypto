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
};

#endif //LOGGER_H
