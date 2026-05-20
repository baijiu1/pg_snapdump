//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_TIME_H
#define PG_HEXRETRO_PG_TIME_H
#include "pg_string_info.h"
#include "pg_c.h"
#include "pg_varatt.h"


#if defined(WIN32) || defined(__CYGWIN__)
#define PG_BINARY	O_BINARY
#define PG_BINARY_A "ab"
#define PG_BINARY_R "rb"
#define PG_BINARY_W "wb"
#else
#define PG_BINARY	0
#define PG_BINARY_A "a"
#define PG_BINARY_R "r"
#define PG_BINARY_W "w"
#endif
#define MAXPGPATH		1024
//typedef int64 Timestamp;
typedef int64 TimestampTz;
typedef int64 TimeOffset;
//typedef int32 fsec_t;			/* fractional seconds (in microseconds) */


static void
j2date(int jd, int *year, int *month, int *day);


typedef double fsec_t;
typedef int64 pg_time_t;
#define TZ_MAX_TIMES 2000
#define TZ_MAX_TYPES 256
#define TZ_MAX_CHARS 50
#define TZ_MAX_LEAPS 50
#define BIGGEST(a,b) (((a) > (b)) ? (a) : (b))


struct ttinfo
{								/* time type information */
    int32		tt_utoff;		/* UT offset in seconds */
    bool		tt_isdst;		/* used to set tm_isdst */
    int			tt_desigidx;	/* abbreviation list index */
    bool		tt_ttisstd;		/* transition is std time */
    bool		tt_ttisut;		/* transition is UT */
};

struct lsinfo
{								/* leap second information */
    pg_time_t	ls_trans;		/* transition time */
    int64		ls_corr;		/* correction to apply */
};
#define TZDEFAULT "/etc/localtime"

struct state
{
    int			leapcnt;
    int			timecnt;
    int			typecnt;
    int			charcnt;
    bool		goback;
    bool		goahead;
    pg_time_t	ats[TZ_MAX_TIMES];
    unsigned char types[TZ_MAX_TIMES];
    struct ttinfo ttis[TZ_MAX_TYPES];
    char		chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, 4 /* sizeof gmt */ ),
                              (2 * (TZ_STRLEN_MAX + 1)))];
    struct lsinfo lsis[TZ_MAX_LEAPS];

    /*
     * The time type to use for early times or if no transitions. It is always
     * zero for recent tzdb releases. It might be nonzero for data from tzdb
     * 2018e or earlier.
     */
    int			defaulttype;
};


struct pg_tz
{
    /* TZname contains the canonically-cased name of the timezone */
    char		TZname[TZ_STRLEN_MAX + 1];
    struct state state;
};

#define	TZ_MAGIC	"TZif"

struct tzhead
{
    char		tzh_magic[4];	/* TZ_MAGIC */
    char		tzh_version[1]; /* '\0' or '2' or '3' as of 2013 */
    char		tzh_reserved[15];	/* reserved; must be zero */
    char		tzh_ttisutcnt[4];	/* coded number of trans. time flags */
    char		tzh_ttisstdcnt[4];	/* coded number of trans. time flags */
    char		tzh_leapcnt[4]; /* coded number of leap seconds */
    char		tzh_timecnt[4]; /* coded number of transition times */
    char		tzh_typecnt[4]; /* coded number of local time types */
    char		tzh_charcnt[4]; /* coded number of abbr. chars */
};

union input_buffer
{
    /* The first part of the buffer, interpreted as a header.  */
    struct tzhead tzhead;

    /* The entire buffer.  */
    char		buf[2 * sizeof(struct tzhead) + 2 * sizeof(struct state)
                    + 4 * TZ_MAX_TIMES];
};

union local_storage
{
    /* The results of analyzing the file's contents after it is opened.  */
    struct file_analysis
    {
        /* The input buffer.  */
        union input_buffer u;

        /* A temporary state used for parsing a TZ string in the file.  */
        struct state st;
    }			u;

    /* We don't need the "fullname" member */
};
struct pg_tm
{
    int			tm_sec;
    int			tm_min;
    int			tm_hour;
    int			tm_mday;
    int			tm_mon;			/* see above */
    int			tm_year;		/* see above */
    int			tm_wday;
    int			tm_yday;
    int			tm_isdst;
    long int	tm_gmtoff;
    const char *tm_zone;
};
static struct pg_tm tm1;

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





Datum ns_decode_date(char *ptr);
Datum ns_decode_time(char *ptr);
Datum ns_decode_timestamp(char *ptr);
Datum ns_decode_timestampz(char *ptr);
Datum ns_decode_timetz(char *ptr);
Datum ns_decode_interval(char *ptr);

typedef struct FunctionCallInfoBaseData
{
//    FmgrInfo   *flinfo;			/* ptr to lookup info used for this call */
//    fmNodePtr	context;		/* pass info about context of call */
//    fmNodePtr	resultinfo;		/* pass or return extra info about result */
//    Oid			fncollation;	/* collation for function to use */
//#define FIELDNO_FUNCTIONCALLINFODATA_ISNULL 4
//    bool		isnull;			/* function must set true if result is NULL */
//    short		nargs;			/* # arguments actually passed */
//#define FIELDNO_FUNCTIONCALLINFODATA_ARGS 6
//    NullableDatum args[FLEXIBLE_ARRAY_MEMBER];
} FunctionCallInfoBaseData;

typedef struct FunctionCallInfoBaseData *FunctionCallInfo;
#define PG_FUNCTION_ARGS	FunctionCallInfo fcinfo
Datum timestamptz_out(PG_FUNCTION_ARGS);

TimestampTz
DatumGetTimestampTz(Datum X);
char *timestamptz_to_str(TimestampTz);
static char *timestamp_to_str_internal(int64_t ts, int tz_offset_hours, int apply_tz);
//char *timestamptz_to_str(TimestampTz ts, int tz_offset_hours);
char *timestamp_to_str(Timestamp ts);
struct tm *pg_localtime_simple(const pg_time_t *timep, int tz_offset_hours);
char *date_to_str(Datum);
char* time_to_str(Datum d);
#endif //PG_HEXRETRO_PG_TIME_H
