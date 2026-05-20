//
// Created by 白杰 on 2026/5/20.
//

#include "pg_time.h"

static void
j2date(int jd, int *year, int *month, int *day)
{
    unsigned int julian;
    unsigned int quad;
    unsigned int extra;
    int			y;

    julian = jd;
    julian += 32044;
    quad = julian / 146097;
    extra = (julian - quad * 146097) * 4 + 3;
    julian += 60 + quad * 3 + extra / 146097;
    quad = julian / 1461;
    julian -= quad * 1461;
    y = julian * 4 / 1461;
    julian = ((y != 0) ? ((julian + 305) % 365) : ((julian + 306) % 366))
             + 123;
    y += quad * 4;
    *year = y - 4800;
    quad = julian * 2141 / 65536;
    *day = julian - 7834 * quad / 256;
    *month = (quad + 10) % MONTHS_PER_YEAR + 1;
}

//static void EncodeSpecialTimestamp(Timestamp dt, char* str)
//{
//    int rc = 0;
//    if (TIMESTAMP_IS_NOBEGIN(dt)) {
//        rc = strcpy_s(str, MAXDATELEN + 1, EARLY);
//        securec_check(rc, "\0", "\0");
//    } else if (TIMESTAMP_IS_NOEND(dt)) {
//        rc = strcpy_s(str, MAXDATELEN + 1, LATE);
//        securec_check(rc, "\0", "\0");
//    } else {
//        /* shouldn't happen */
//        ereport(ERROR,
//                (errcode(ERRCODE_INVALID_ARGUMENT_FOR_NTH_VALUE), errmsg("invalid argument for EncodeSpecialTimestamp")));
//    }
//}


Datum ns_decode_date(char *ptr)
{
//    *len = 4;
    return *(int32_t *)ptr;
}

Datum ns_decode_time(char *ptr)
{
//    *len = 8;
    return *(int64_t *)ptr;
}

Datum ns_decode_timestamp(char *ptr)
{
//    *len = 8;
    return *(int64_t *)ptr;
}

TimestampTz
DatumGetTimestampTz(Datum X)
{
    return (TimestampTz) DatumGetInt64(X);
}

Datum ns_decode_timestampz(char *ptr)
{
//    *len = 8;
    return *(int64_t *)ptr;
}

Datum ns_decode_timetz(char *ptr)
{
//    *len = 12;
    return (Datum)ptr;
}

Datum ns_decode_interval(char *ptr)
{
//    *len = 16;
    return (Datum)ptr;
}

#define POSTGRES_EPOCH_JDATE 2451545
#define UNIX_EPOCH_JDATE     2440588
#define SECS_PER_DAY         86400
#define USECS_PER_SEC        1000000

char *timestamptz_to_str(TimestampTz ts)
{
    return timestamp_to_str_internal(ts, 8, 1);
}

char *timestamp_to_str(Timestamp ts)
{
    return timestamp_to_str_internal(ts, 0, 0);
}

static char *timestamp_to_str_internal(int64_t ts, int tz_offset_hours, int apply_tz)
{
    int64_t timestamp = ts;

    // ===== 1️⃣ 是否应用时区 =====
    if (apply_tz)
        timestamp += tz_offset_hours * 3600LL * USECS_PER_SEC;

    // ===== 2️⃣ PG epoch → Unix epoch =====
    int64_t epoch_diff_days =
            (POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE);

    timestamp += epoch_diff_days * USECS_PER_DAY;

    // ===== 3️⃣ 拆秒 =====
    time_t sec = (time_t)(timestamp / USECS_PER_SEC);
    int usec = (int)(timestamp % USECS_PER_SEC);

    if (usec < 0)
    {
        usec += USECS_PER_SEC;
        sec -= 1;
    }

    // ===== 4️⃣ 转 tm =====
    struct tm *tm = gmtime(&sec);
    if (!tm)
        return NULL;

    // ===== 5️⃣ 输出 =====
    char *buf = (char*) malloc(64);

    if (apply_tz)
    {
        // timestamptz
        snprintf(buf, 64,
                 "%04d-%02d-%02d %02d:%02d:%02d.%06d%+03d",
                 tm->tm_year + 1900,
                 tm->tm_mon + 1,
                 tm->tm_mday,
                 tm->tm_hour,
                 tm->tm_min,
                 tm->tm_sec,
                 usec,
                 tz_offset_hours);
    }
    else
    {
        // timestamp（无时区）
        snprintf(buf, 64,
                 "%04d-%02d-%02d %02d:%02d:%02d.%06d",
                 tm->tm_year + 1900,
                 tm->tm_mon + 1,
                 tm->tm_mday,
                 tm->tm_hour,
                 tm->tm_min,
                 tm->tm_sec,
                 usec);
    }

    return buf;
}

char *date_to_str(Datum d) {
    int32   jd,
            year,
            month,
            day;

    if (d == PG_INT32_MIN)
    {
        CopyAppend("-infinity");
        return NULL;
    }
    if (d == PG_INT32_MAX)
    {
        CopyAppend("infinity");
        return NULL;
    }

    jd = d + POSTGRES_EPOCH_JDATE;
    j2date(jd, &year, &month, &day);
    char *buf = (char*) malloc(64);

    snprintf(buf, 64,
             "%04d-%02d-%02d%s",
             year,
             month,
             day,
             (year <= 0) ? " BC" : "");

    return buf;
}

char* time_to_str(Datum d) {
    int64		timestamp,
            timestamp_sec;
    timestamp = (int64) d;
    timestamp_sec = timestamp / 1000000;
    char *buf = (char*) malloc(64);
    snprintf(buf, 64,
             "%02ld:%02ld:%02ld.%06ld",
             timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
             timestamp % 1000000);
    return buf;
}