//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_LOG_H
#define PG_HEXRETRO_PG_LOG_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

typedef enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

static int current_level = LOG_INFO;

void log_message(const char *level,
                 const char *file,
                 int line,
                 const char *fmt,
                 ...);

#define LOG_INFO(fmt, ...) \
    log_message("INFO", \
                __FILE__, \
                __LINE__, \
                fmt, \
                ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    log_message("ERROR", \
                __FILE__, \
                __LINE__, \
                fmt, \
                ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    log_message("DEBUG", \
                __FILE__, \
                __LINE__, \
                fmt, \
                ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    log_message("WARN", \
                __FILE__, \
                __LINE__, \
                fmt, \
                ##__VA_ARGS__)

#endif //PG_HEXRETRO_PG_LOG_H
