//
// Created by 白杰 on 2025/8/9.
//

#ifndef PG_SNPDUMP_PG_TIME_H
#define PG_SNPDUMP_PG_TIME_H
#include "cstdlib"
#include "cstdio"
#include "pg_common.h"

using namespace std;

static void
j2date(int jd, int *year, int *month, int *day);

void decode_time(CtidNode* tuple, int, uint32_t *);

void decode_timetz(CtidNode* tuple, int, uint32_t *);

void decode_date(CtidNode* tuple, int, uint32_t *);

void decode_timestamp_internal(CtidNode* tuple, int, uint32_t *, bool with_timezone);

void decode_timestamp(CtidNode* tuple, int, uint32_t *);

void decode_timestamptz(CtidNode* tuple, int, uint32_t *);

void decode_interval(CtidNode* tuple, int, uint32_t *);

void decode_tsrange(CtidNode* tuple, int, uint32_t *);

void decode_tstzrange(CtidNode* tuple, int, uint32_t *);

void decode_daterange(CtidNode* tuple, int, uint32_t *);


typedef double fsec_t;
typedef int64 pg_time_t;

struct pg_tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;  /* origin 0, not 1 */
    int tm_year; /* relative to 1900 */
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long int tm_gmtoff;
    const char* tm_zone;
};

typedef struct pg_tz pg_tz;
typedef struct pg_tzenum pg_tzenum;
typedef double Timestamp;

#define DAGO "ago"
#define DCURRENT "current"
#define EPOCH "epoch"
#define INVALID "invalid"
#define EARLY "-infinity"
#define LATE "infinity"
#define NOW "now"
#define TODAY "today"
#define TOMORROW "tomorrow"
#define YESTERDAY "yesterday"
#define ZULU "zulu"

#define DMICROSEC "usecond"
#define DMILLISEC "msecond"
#define DSECOND "second"
#define DMINUTE "minute"
#define DHOUR "hour"
#define DDAY "day"
#define DWEEK "week"
#define DMONTH "month"
#define DQUARTER "quarter"
#define DYEAR "year"
#define DDECADE "decade"
#define DCENTURY "century"
#define DMILLENNIUM "millennium"
#define DA_D "ad"
#define DB_C "bc"
#define DTIMEZONE "timezone"

/*
 * Fundamental time field definitions for parsing.
 *
 *	Meridian:  am, pm, or 24-hour style.
 *	Millennium: ad, bc
 */

#define AM 0
#define PM 1
#define HR24 2

#define AD 0
#define BC 1

/*
 * Fields for time decoding.
 *
 * Can't have more of these than there are bits in an unsigned int
 * since these are turned into bit masks during parsing and decoding.
 *
 * Furthermore, the values for YEAR, MONTH, DAY, HOUR, MINUTE, SECOND
 * must be in the range 0..14 so that the associated bitmasks can fit
 * into the left half of an INTERVAL's typmod value.  Since those bits
 * are stored in typmods, you can't change them without initdb!
 */

#define RESERV 0
#define MONTH 1
#define YEAR 2
#define DAY 3
#define JULIAN 4
#define TZ 5
#define DTZ 6
#define DTZMOD 7
#define IGNORE_DTF 8
#define AMPM 9
#define HOUR 10
#define MINUTE 11
#define SECOND 12
#define MILLISECOND 13
#define MICROSECOND 14
#define DOY 15
#define DOW 16
#define UNITS 17
#define ADBC 18
/* these are only for relative dates */
#define AGO 19
#define ABS_BEFORE 20
#define ABS_AFTER 21
/* generic fields to help with parsing */
#define ISODATE 22
#define ISOTIME 23
/* these are only for parsing intervals */
#define WEEK 24
#define DECADE 25
#define CENTURY 26
#define MILLENNIUM 27
/* reserved for unrecognized string values */
#define UNKNOWN_FIELD 31

/*
 * Token field definitions for time parsing and decoding.
 * These need to fit into the datetkn table type.
 * At the moment, that means keep them within [-127,127].
 * These are also used for bit masks in DecodeDateDelta()
 *	so actually restrict them to within [0,31] for now.
 * - thomas 97/06/19
 * Not all of these fields are used for masks in DecodeDateDelta
 *	so allow some larger than 31. - thomas 1997-11-17
 */

#define DTK_NUMBER 0
#define DTK_STRING 1

#define DTK_DATE 2
#define DTK_TIME 3
#define DTK_TZ 4
#define DTK_AGO 5

#define DTK_SPECIAL 6
#define DTK_INVALID 7
#define DTK_CURRENT 8
#define DTK_EARLY 9
#define DTK_LATE 10
#define DTK_EPOCH 11
#define DTK_NOW 12
#define DTK_YESTERDAY 13
#define DTK_TODAY 14
#define DTK_TOMORROW 15
#define DTK_ZULU 16

#define DTK_DELTA 17
#define DTK_SECOND 18
#define DTK_MINUTE 19
#define DTK_HOUR 20
#define DTK_DAY 21
#define DTK_WEEK 22
#define DTK_MONTH 23
#define DTK_QUARTER 24
#define DTK_YEAR 25
#define DTK_DECADE 26
#define DTK_CENTURY 27
#define DTK_MILLENNIUM 28
#define DTK_MILLISEC 29
#define DTK_MICROSEC 30
#define DTK_JULIAN 31

#define DTK_DOW 32
#define DTK_DOY 33
#define DTK_TZ_HOUR 34
#define DTK_TZ_MINUTE 35
#define DTK_ISOYEAR 36
#define DTK_ISODOW 37

/*
 * Bit mask definitions for time parsing.
 */

#define DTK_M(t) (0x01 << (t))

/* Convenience: a second, plus any fractional component */
#define DTK_ALL_SECS_M (DTK_M(SECOND) | DTK_M(MILLISECOND) | DTK_M(MICROSECOND))
#define DTK_DATE_M (DTK_M(YEAR) | DTK_M(MONTH) | DTK_M(DAY))
#define DTK_TIME_M (DTK_M(HOUR) | DTK_M(MINUTE) | DTK_ALL_SECS_M)

/*
 * Working buffer size for input and output of interval, timestamp, etc.
 * Inputs that need more working space will be rejected early.  Longer outputs
 * will overrun buffers, so this must suffice for all possible output.  As of
 * this writing, interval_out() needs the most space at ~90 bytes.
 */
#define MAXDATELEN 128
/* only this many chars are stored in datetktbl */
#define TOKMAXLEN 10

/* keep this struct small; it gets used a lot */
typedef struct datetkn {
    char token[TOKMAXLEN + 1];
    char type;
    char value; /* this may be unsigned, alas */
} datetkn;

/* one of its uses is in tables of time zone abbreviations */
typedef struct TimeZoneAbbrevTable {
    int numabbrevs;
    datetkn abbrevs[FLEXIBLE_ARRAY_MEMBER]; /* VARIABLE LENGTH ARRAY */
} TimeZoneAbbrevTable;

/* FMODULO()
 * Macro to replace modf(), which is broken on some platforms.
 * t = input and remainder
 * q = integer part
 * u = divisor
 */
#define FMODULO(t, q, u)                                        \
    do {                                                        \
        (q) = (((t) < 0) ? ceil((t) / (u)) : floor((t) / (u))); \
        if ((q) != 0)                                           \
            (t) -= rint((q) * (u));                             \
    } while (0)

/* TMODULO()
 * Like FMODULO(), but work on the timestamp datatype (either int64 or float8).
 * We assume that int64 follows the C99 semantics for division (negative
 * quotients truncate towards zero).
 */
#ifdef HAVE_INT64_TIMESTAMP
#define TMODULO(t, q, u)        \
    do {                        \
        (q) = ((t) / (u));      \
        if ((q) != 0)           \
            (t) -= ((q) * (u)); \
    } while (0)
#else
#define TMODULO(t, q, u)                                        \
    do {                                                        \
        (q) = (((t) < 0) ? ceil((t) / (u)) : floor((t) / (u))); \
        if ((q) != 0)                                           \
            (t) -= rint((q) * (u));                             \
    } while (0)
#endif

/*
 * Date/time validation
 * Include check for leap year.
 */

extern const int day_tab[2][13];

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

/*
 * Datetime input parsing routines (ParseDateTime, DecodeDateTime, etc)
 * return zero or a positive value on success.	On failure, they return
 * one of these negative code values.  DateTimeParseError may be used to
 * produce a correct ereport.
 */
#define DTERR_BAD_FORMAT (-1)
#define DTERR_FIELD_OVERFLOW (-2)
#define DTERR_MD_FIELD_OVERFLOW (-3) /* triggers hint about u_sess->time_cxt.DateStyle */
#define DTERR_INTERVAL_OVERFLOW (-4)
#define DTERR_TZDISP_OVERFLOW (-5)
#endif //PG_SNPDUMP_PG_TIME_H
