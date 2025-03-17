//
// Created by Fx Kaze on 25-1-16.
//

#include "Logger.h"
#include "ColorfulConsole.h"

#include "ConditionVariable.h"
#include "ThreadPool.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>

extern "C" {
#include "list/CList.h"
}

#define LOG_FILE_NAME "CryptoCore"
#define LOG_PATH "./log/../../haha/log/cow/log"

namespace Logger {
    static FILE *logFile = nullptr;
    static char *logPath = nullptr;
    static char *logFullFile = nullptr;
#ifdef __DEBUG
    static LogLevel logLevel = DEBUG;
#else
    static LogLevel logLevel = INFO;
#endif
    static list_t msgList;
    static int prt = 0;
    static int term = 0;

    static RWLock rw;

    static ConditionVariable cond;

    static ThreadPool::taskDescriptor<void> thread_logger;

    typedef struct msgNodeType {
        node_t node;
        char *msg;
        LogLevel level;
    } msgNode;

    static int mkdirs(const char *full_path, const char *solved_path, mode_t mode) {
        const char *p = solved_path;
        if (*p == '/')
            return mkdirs(full_path, p + 1, mode);
        if (*p == '\0')
            return 0;
        int len = 0, path_len = solved_path - full_path;
        while (p[len] != '/' && p[len] != '\0')
            len++;
        char *tmp_path = (char *) malloc(len + path_len + 1);
        char *tmp_name = (char *) malloc(len + 1);
        memcpy(tmp_path, full_path, len + path_len);
        memcpy(tmp_name, solved_path, len);
        tmp_path[len + path_len] = '\0';
        tmp_name[len] = '\0';
        if (strcmp(tmp_name, ".") != 0) {
            struct stat st;
            if (stat(tmp_path, &st) == 0) {
                if (!S_ISDIR(st.st_mode)) {
                    fprintf(stderr, "%s is not a directory.\n", tmp_path);
                    free(tmp_path);
                    free(tmp_name);
                    return EOF;
                }
            } else {
                if (mkdir(tmp_path, mode) != 0) {
                    fprintf(stderr, "mkdir(%s) failed.\n", tmp_path);
                    free(tmp_path);
                    free(tmp_name);
                    return EOF;
                }
            }
        }
        free(tmp_path);
        free(tmp_name);
        return mkdirs(full_path, p + len, mode);
    }

    static int openLogFile() {
        struct stat file_stat;
        if (stat(logPath, &file_stat) == 0) {
            if (access(logPath, R_OK | W_OK) != 0) {
                fprintf(stderr, "Could not read/write log directory '%s'.\n", logPath);
                return EOF;
            }
            if (!S_ISDIR(file_stat.st_mode)) {
                fprintf(stderr, "'%s' is not a directory.\n", logPath);
                return EOF;
            }
        } else {
            if (mkdirs(logPath, logPath, 0755) != 0)
                return EOF;
        }
        int path_len = strlen(logPath) + 128;
        logFullFile = static_cast<char *>(malloc(path_len + 1));
        memcpy(logFullFile, logPath, path_len - 128);
        char *p = logFullFile + path_len - 128;
        *p++ = '/';
        time_t _time = time(nullptr);
        struct tm *tm = localtime(&_time);
        sprintf(p, "%s_%04d%02d%02d.log", LOG_FILE_NAME, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        logFile = fopen(logFullFile, "a");
        if (!logFile) {
            fprintf(stderr, "Could not open log file '%s'.\n", logFullFile);
            return EOF;
        }
        return 0;
    }

    static void closeLogFile() {
        if (logPath)
            free(logPath);
        if (logFullFile)
            free(logFullFile);
        if (logFile)
            fclose(logFile);
    }

    static char *strCat(const char *s1, const char *s2) {
        char *ret = static_cast<char *>(malloc(strlen(s1) + strlen(s2) + 1));
        strcpy(ret, s1);
        strcat(ret + strlen(s1), s2);
        return ret;
    }

    static char *parentPath(const char *path) {
        int len = strlen(path);
        const char *p = path + len - 1;
        while (p != path && *p != '/')
            p--, len--;
        if (len == 0)
            len = 1;
        char *ret = static_cast<char *>(malloc(len));
        memcpy(ret, path, len - 1);
        ret[len - 1] = '\0';
        return ret;
    }

    static char *solvePathRecursive(char *path, const char *left) {
        char *tmp = nullptr;
        if (left[0] == '.') {
            if (left[1] == '.') {
                // ../.....
                tmp = parentPath(path);
                free(path);
                if (left[2] == '\0') return tmp;
                return solvePathRecursive(tmp, left + 3);
            }
            // ./.....
            if (left[1] == '\0') return tmp;
            return solvePathRecursive(path, left + 2);
        }
        const char *p = left;
        int len = 0;
        while (*p != '\0' && *p != '/')
            p++, len++;
        char *t_name = static_cast<char *>(malloc(len + 2));
        t_name[0] = '/';
        memcpy(t_name + 1, left, len);
        t_name[len + 1] = '\0';
        tmp = strCat(path, t_name);
        free(t_name);
        free(path);
        if (*p == '\0') return tmp;
        return solvePathRecursive(tmp, p + 1);
    }

    static char *solvePath(const char *path) {
        char *tmp = static_cast<char *>(malloc(1024));
        tmp[0] = '\0';
        if (*path == '\0')
            return tmp;
        if (path[0] != '/') {
            getcwd(tmp, 1024);
            return solvePathRecursive(tmp, path);
        }
        tmp[0] = '\0';
        return solvePathRecursive(tmp, path + 1);
    }

    static void logger() {
        while (true) {
            rw.ReaderLock();
            if (listSize(msgList) == 0) {
                rw.ReaderUnlock();
                if (term)
                    break;
                cond.wait();
                if (term)
                    break;
                rw.ReaderLock();
            }
            msgNode *node = *reinterpret_cast<msgNode **>(findNodeByIndex(&msgList, 0));
            removeByIndex(msgList, 0);
            rw.ReaderUnlock();
            time_t _time = time(nullptr);
            struct tm *tm = localtime(&_time);
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            fprintf(logFile, "%04d/%02d/%02d %02d:%02d:%02d.%06d%s\n",
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec,
                    node->msg);
            if (prt) {
                const char *color = Cor(_Kaze::F_WHITE);
                FILE *fd = stdout;
                switch (node->level) {
                    case DEBUG:
                        color = Cor(_Kaze::BF_CYAN);
                        fd = stdout;
                        break;
                    case INFO:
                        color = Cor(_Kaze::BF_GREEN);
                        fd = stdout;
                        break;
                    case NOTICE:
                        color = Cor(_Kaze::BF_BLUE);
                        fd = stdout;
                        break;
                    case WARNING:
                        color = Cor(_Kaze::BF_YELLOW);
                        fd = stdout;
                        break;
                    case ERROR:
                        color = Cor(_Kaze::BF_MAGENTA);
                        fd = stderr;
                        break;
                    case FATAL:
                        color = Cor(_Kaze::BF_RED);
                        fd = stderr;
                        break;
                }
                fprintf(fd, "%s%04d/%02d/%02d %02d:%02d:%02d.%06d%s%s\n",
                        color,
                        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec,
                        node->msg,
                        Cor(_Kaze::F_D));
            }
            free(node->msg);
            free(node);
        }
    }

    class Logger : ThreadPool {
    public:
        Logger(): ThreadPool(1) {
        }

        int init() {
            msgList = initList();
            if (!msgList) {
                fprintf(stderr, "Could not initialize message list.\n");
                return EOF;
            }
            if (!logFile) {
                if (setLogPathAndOpen() == 0) {
                    printf("Log file '%s'.\n", logFullFile);
                } else
                    return EOF;
            }
#ifdef __DEBUG
            prt = 1;
#endif
            thread_logger = addThreadEX(logger).start();
            return 0;
        }

        ~Logger() {
            rw.WriterLock();
            term = 1;
            if (listSize(msgList) == 0) {
                cond.notify_one();
            }
            rw.WriterUnlock();
            thread_logger.wait();
            destroyList(msgList);
            closeLogFile();
        }
    };

    static Logger __logger;


    int setLogPathAndOpen(const char *_logPath) {
        if (!_logPath) {
            _logPath = LOG_PATH;
        }
        closeLogFile();
        logPath = solvePath(_logPath);
        return openLogFile();
    }

    const char *getLogPath() {
        return logPath;
    }

    void setLogLevel(LogLevel _logLevel) {
        logLevel = _logLevel;
    }

    LogLevel getLogLevel() {
        return logLevel;
    }

    int init() {
        if (!logFile)
            return __logger.init();
        return 0;
    }

    void ptr() {
        prt = 1;
    }

    void noPrt() {
        prt = 0;
    }

    void log(LogLevel _logLevel, const char *format, ...) {
        if (_logLevel < logLevel || _logLevel == OFF) return;
        char buffer[8192], *p = buffer;
        switch (_logLevel) {
            case DEBUG:
                p += snprintf(buffer, sizeof(buffer), "   [DEBUG] ");
                break;
            case INFO:
                p += snprintf(buffer, sizeof(buffer), "    [INFO] ");
                break;
            case NOTICE:
                p += snprintf(buffer, sizeof(buffer), "  [NOTICE] ");
                break;
            case WARNING:
                p += snprintf(buffer, sizeof(buffer), " [WARNING] ");
                break;
            case ERROR:
                p += snprintf(buffer, sizeof(buffer), "   [ERROR] ");
                break;
            case FATAL:
                p += snprintf(buffer, sizeof(buffer), "   [FATAL] ");
                break;
        }
        va_list args;
        va_start(args, format);
        vsnprintf(p, sizeof(buffer), format, args);
        va_end(args);
        msgNode *newNode = static_cast<msgNode *>(malloc(sizeof(msgNode)));
        newNode->msg = static_cast<char *>(malloc(strlen(buffer) + 1));
        strcpy(newNode->msg, buffer);
        newNode->level = _logLevel;
        rw.WriterLock();
        pushExistEnd(msgList, reinterpret_cast<node_t *>(newNode));
        if (listSize(msgList) == 1)
            cond.notify_one();
        rw.WriterUnlock();
    }
}
