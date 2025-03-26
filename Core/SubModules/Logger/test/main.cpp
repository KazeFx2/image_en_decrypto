//
// Created by Fx Kaze on 25-1-20.
//

// #define __DEBUG

#include "Logger.h"

int main() {
    Logger::init();
    LOG_DEBUG("Hello World!");
    LOG_INFO("Hello World!");
    LOG_NOTICE("Hello World!");
    LOG_WARNING("Hello World!");
    LOG_ERROR("Hello World!");
    LOG_FATAL("Hello World!");

    LOG_NAMED_DEBUG(__func__, "Hello World!");
    LOG_NAMED_INFO(__func__, "Hello World!");
    LOG_NAMED_NOTICE(__func__, "Hello World!");
    LOG_NAMED_WARNING(__func__, "Hello World!");
    LOG_NAMED_ERROR(__func__, "Hello World!");
    LOG_NAMED_FATAL(__func__, "Hello World!");

    return 0;
}
