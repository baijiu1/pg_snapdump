//
// Created by 白杰 on 2025/9/17.
//

#ifndef PG_SNAPDUMP_PG_LOG_H
#define PG_SNAPDUMP_PG_LOG_H

#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;

// log level
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#define LOG(level, msg, ...) do { \
    if (level >= LOG_LEVEL) { \
        time_t now = time(NULL); \
        char ts[20]; \
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now)); \
        fprintf(stderr, "[%s] [%s] %s:%d %s(): " msg "\n", \
                log_level_str(level), ts, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } \
} while (0)

static inline const char* log_level_str(int level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

#endif //PG_SNAPDUMP_PG_LOG_H
