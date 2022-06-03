#pragma once
#include "engine_config.h"
#include <signal.h>
#include <cassert>
#include <Ren/Logger.hpp>
#include <memory>

#ifdef ENGINE_DEBUG
    #ifdef WIN32
        #define BREAK() __debugbreak()
    #else
        #define BREAK() raise(SIGTRAP)
    #endif

    #define REN_ASSERT(condition, message) \
        do { \
            if (! (condition)) { \
                std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                        << ":" << __LINE__ << ": " << message << std::endl; \
                std::terminate(); \
            } \
        } while (false)
#else
    #define BREAK()
    #define ASSERT(condition, message) do { } while (false)
#endif

template<class T>
using Ref = std::shared_ptr<T>;

#define REN_ERR_LOG(msg) Logger::LogE(msg, __FILE__, __LINE__)