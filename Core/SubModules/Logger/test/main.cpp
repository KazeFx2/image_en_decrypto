//
// Created by Fx Kaze on 25-1-20.
//

// #define __DEBUG

#include "Logger.h"
#include "ThreadPool.h"

int main() {
    Logger::init();
    LOG_DEBUG("Hello World!");
    LOG_INFO("Hello World!");
    LOG_NOTICE("Hello World!");
    LOG_WARNING("Hello World!");
    LOG_ERROR("Hello World!");
    LOG_FATAL("Hello World!");
}
