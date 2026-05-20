//
// Created by 白杰 on 2026/5/20.
//

#include "pg_log.h"


void log_message(const char *level,
                 const char *file,
                 int line,
                 const char *fmt,
                 ...)
{
    time_t now = time(NULL);

    struct tm *tm_info = localtime(&now);

    char timebuf[64];

    strftime(timebuf,
             sizeof(timebuf),
             "%Y-%m-%d %H:%M:%S",
             tm_info);

    printf("[%s][%s][%s:%d] ",
           timebuf,
           level,
           file,
           line);

    va_list args;

    va_start(args, fmt);

    vprintf(fmt, args);

    va_end(args);

    printf("\n");
}