//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_C_H
#define PG_HEXRETRO_PG_C_H

#include "pg_log.h"
#include "pg_config.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned int Oid;
typedef uint32_t TransactionId;
typedef uint16_t OffsetNumber;
typedef uint32_t BlockNumber;
typedef uint32_t CommandId;



typedef uint16_t LocationIndex;


typedef uint32_t ShortTransactionId;



typedef float float4;
typedef double float8;
#define MAX_PAGE_COUNT 131072
#define NAMEDATALEN 64
#define PAGE_SIZE 8192
#define Assert(condition)	((void)true)
#define AssertMacro(condition)	((void)true)
#ifndef HAVE_UINT8
typedef unsigned char uint8;	/* == 8 bits */
typedef unsigned short uint16;	/* == 16 bits */
typedef unsigned int uint32;	/* == 32 bits */
#endif							/* not HAVE_UINT8 */
#ifndef HAVE_INT64
typedef long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif

typedef uint32 TransactionId;
typedef uint32 pg_crc32c;
typedef uint16 RepOriginId;
typedef int Buffer;
typedef uint32 BlockNumber;
typedef uint64 XLogSegNo;

#define MaxAllocSize	((int) 0x3fffffff) /* 1 gigabyte - 1 */

typedef enum {
    TUPLE_OK,
    TUPLE_BAD_HOFF,
    TUPLE_OOB,
    TUPLE_DEFORM_FAIL
} TupleStatus;
typedef uint8 RmgrId;

typedef uint32 JEntry;

#ifdef HAVE__BUILTIN_CONSTANT_P
#define ereport_domain(elevel, domain, ...)	\
	do { \
		pg_prevent_errno_in_scope(); \
		if (__builtin_constant_p(elevel) && (elevel) >= ERROR ? \
			errstart_cold(elevel, domain) : \
			errstart(elevel, domain)) \
			__VA_ARGS__, errfinish(__FILE__, __LINE__, __func__); \
		if (__builtin_constant_p(elevel) && (elevel) >= ERROR) \
			pg_unreachable(); \
	} while(0)
#else							/* !HAVE__BUILTIN_CONSTANT_P */
#define ereport_domain(elevel, domain, ...)	\
	do { \
		const int elevel_ = (elevel); \
		pg_prevent_errno_in_scope(); \
		if (errstart(elevel_, domain)) \
			__VA_ARGS__, errfinish(__FILE__, __LINE__, __func__); \
		if (elevel_ >= ERROR) \
			pg_unreachable(); \
	} while(0)
#endif



#define ereport(elevel, ...)	\
	ereport_domain(elevel, TEXTDOMAIN, __VA_ARGS__)
#define elog(elevel, ...)  \
	ereport(elevel, errmsg_internal(__VA_ARGS__))


#if __GNUC__ >= 3
#define likely(x)	__builtin_expect((x) != 0, 1)
#define unlikely(x) __builtin_expect((x) != 0, 0)
#else
#define likely(x)	((x) != 0)
#define unlikely(x) ((x) != 0)
#endif

#define Max(x, y)		((x) > (y) ? (x) : (y))

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))


#ifndef HAVE_INT8
typedef signed char int8;		/* == 8 bits */
typedef signed short int16;		/* == 16 bits */
typedef signed int int32;		/* == 32 bits */
#endif							/* not HAVE_INT8 */

/*
 * uintN
 *		Unsigned integer, EXACTLY N BITS IN SIZE,
 *		used for numerical computations and the
 *		frontend/backend protocol.
 */
#ifndef HAVE_UINT8
typedef unsigned char uint8;	/* == 8 bits */
typedef unsigned short uint16;	/* == 16 bits */
typedef unsigned int uint32;	/* == 32 bits */
#endif							/* not HAVE_UINT8 */

/*
 * bitsN
 *		Unit of bitwise operation, AT LEAST N BITS IN SIZE.
 */
typedef uint8 bits8;			/* >= 8 bits */
typedef uint16 bits16;			/* >= 16 bits */
typedef uint32 bits32;			/* >= 32 bits */

#ifndef HAVE_INT64
typedef long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif

#define ERROR 21
typedef uint32 TimeLineID;
typedef uint64 XLogRecPtr;

#define XLOG_BLCKSZ 8192
#define MAX_ERRORMSG_LEN 1000
#define RELSEG_SIZE    131072

/* Plain "long int" fits, use it */

#ifndef HAVE_INT64
typedef long int int64;
#endif
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif


typedef uint64 Datum;



#define INT64CONST(x)  (x##L)
#define UINT64CONST(x) (x##UL)

#define PG_INT8_MIN		(-0x7F-1)
#define PG_INT8_MAX		(0x7F)
#define PG_UINT8_MAX	(0xFF)
#define PG_INT16_MIN	(-0x7FFF-1)
#define PG_INT16_MAX	(0x7FFF)
#define PG_UINT16_MAX	(0xFFFF)
#define PG_INT32_MIN	(-0x7FFFFFFF-1)
#define PG_INT32_MAX	(0x7FFFFFFF)
#define PG_UINT32_MAX	(0xFFFFFFFFU)
#define PG_INT64_MIN	(-INT64CONST(0x7FFFFFFFFFFFFFFF) - 1)
#define PG_INT64_MAX	INT64CONST(0x7FFFFFFFFFFFFFFF)
#define PG_UINT64_MAX	UINT64CONST(0xFFFFFFFFFFFFFFFF)

/* Limits on the "precision" option (typmod) for these data types */
#define MAX_TIMESTAMP_PRECISION 6
#define MAX_INTERVAL_PRECISION 6

/*
 *	Round off to MAX_TIMESTAMP_PRECISION decimal places.
 *	Note: this is also used for rounding off intervals.
 */
#define TS_PREC_INV 1000000.0
#define TSROUND(j) (rint(((double) (j)) * TS_PREC_INV) / TS_PREC_INV)


/*
 * Assorted constants for datetime-related calculations
 */

#define DAYS_PER_YEAR	365.25	/* assumes leap year every four years */
#define MONTHS_PER_YEAR 12
/*
 *	DAYS_PER_MONTH is very imprecise.  The more accurate value is
 *	365.2425/12 = 30.436875, or '30 days 10:29:06'.  Right now we only
 *	return an integral number of days, but someday perhaps we should
 *	also return a 'time' value to be used as well.  ISO 8601 suggests
 *	30 days.
 */
#define DAYS_PER_MONTH	30		/* assumes exactly 30 days per month */
#define DAYS_PER_WEEK	7
#define HOURS_PER_DAY	24		/* assume no daylight savings time changes */

/*
 *	This doesn't adjust for uneven daylight savings time intervals or leap
 *	seconds, and it crudely estimates leap years.  A more accurate value
 *	for days per years is 365.2422.
 */
#define SECS_PER_YEAR	(36525 * 864)	/* avoid floating-point computation */
#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600
#define SECS_PER_MINUTE 60
#define MINS_PER_HOUR	60

#define USECS_PER_DAY	INT64CONST(86400000000)
#define USECS_PER_HOUR	INT64CONST(3600000000)
#define USECS_PER_MINUTE INT64CONST(60000000)
#define USECS_PER_SEC	INT64CONST(1000000)

/*
 * We allow numeric timezone offsets up to 15:59:59 either way from Greenwich.
 * Currently, the record holders for wackiest offsets in actual use are zones
 * Asia/Manila, at -15:56:00 until 1844, and America/Metlakatla, at +15:13:42
 * until 1867.  If we were to reject such values we would fail to dump and
 * restore old timestamptz values with these zone settings.
 */
#define MAX_TZDISP_HOUR		15	/* maximum allowed hour part */
#define TZDISP_LIMIT		((MAX_TZDISP_HOUR + 1) * SECS_PER_HOUR)

/*
 * We reserve the minimum and maximum integer values to represent
 * timestamp (or timestamptz) -infinity and +infinity.
 */
#define TIMESTAMP_MINUS_INFINITY	PG_INT64_MIN
#define TIMESTAMP_INFINITY	PG_INT64_MAX

/*
 * Historically these aliases for infinity have been used.
 */
#define DT_NOBEGIN		TIMESTAMP_MINUS_INFINITY
#define DT_NOEND		TIMESTAMP_INFINITY

#define TIMESTAMP_NOBEGIN(j)	\
	do {(j) = DT_NOBEGIN;} while (0)

#define TIMESTAMP_IS_NOBEGIN(j) ((j) == DT_NOBEGIN)

#define TIMESTAMP_NOEND(j)		\
	do {(j) = DT_NOEND;} while (0)

#define TIMESTAMP_IS_NOEND(j)	((j) == DT_NOEND)

#define TIMESTAMP_NOT_FINITE(j) (TIMESTAMP_IS_NOBEGIN(j) || TIMESTAMP_IS_NOEND(j))

/*
 * Infinite intervals are represented by setting all fields to the minimum or
 * maximum integer values.
 */
#define INTERVAL_NOBEGIN(i)	\
	do {	\
		(i)->time = PG_INT64_MIN;	\
		(i)->day = PG_INT32_MIN;	\
		(i)->month = PG_INT32_MIN;	\
	} while (0)

#define INTERVAL_IS_NOBEGIN(i)	\
	((i)->month == PG_INT32_MIN && (i)->day == PG_INT32_MIN && (i)->time == PG_INT64_MIN)

#define INTERVAL_NOEND(i)	\
	do {	\
		(i)->time = PG_INT64_MAX;	\
		(i)->day = PG_INT32_MAX;	\
		(i)->month = PG_INT32_MAX;	\
	} while (0)

#define INTERVAL_IS_NOEND(i)	\
	((i)->month == PG_INT32_MAX && (i)->day == PG_INT32_MAX && (i)->time == PG_INT64_MAX)

#define INTERVAL_NOT_FINITE(i) (INTERVAL_IS_NOBEGIN(i) || INTERVAL_IS_NOEND(i))

/*
 * Julian date support.
 *
 * date2j() and j2date() nominally handle the Julian date range 0..INT_MAX,
 * or 4714-11-24 BC to 5874898-06-03 AD.  In practice, date2j() will work and
 * give correct negative Julian dates for dates before 4714-11-24 BC as well.
 * We rely on it to do so back to 4714-11-01 BC.  Allowing at least one day's
 * slop is necessary so that timestamp rotation doesn't produce dates that
 * would be rejected on input.  For example, '4714-11-24 00:00 GMT BC' is a
 * legal timestamptz value, but in zones east of Greenwich it would print as
 * sometime in the afternoon of 4714-11-23 BC; if we couldn't process such a
 * date we'd have a dump/reload failure.  So the idea is for IS_VALID_JULIAN
 * to accept a slightly wider range of dates than we really support, and
 * then we apply the exact checks in IS_VALID_DATE or IS_VALID_TIMESTAMP,
 * after timezone rotation if any.  To save a few cycles, we can make
 * IS_VALID_JULIAN check only to the month boundary, since its exact cutoffs
 * are not very critical in this scheme.
 *
 * It is correct that JULIAN_MINYEAR is -4713, not -4714; it is defined to
 * allow easy comparison to tm_year values, in which we follow the convention
 * that tm_year <= 0 represents abs(tm_year)+1 BC.
 */

#define JULIAN_MINYEAR (-4713)
#define JULIAN_MINMONTH (11)
#define JULIAN_MINDAY (24)
#define JULIAN_MAXYEAR (5874898)
#define JULIAN_MAXMONTH (6)
#define JULIAN_MAXDAY (3)

#define IS_VALID_JULIAN(y,m,d) \
	(((y) > JULIAN_MINYEAR || \
	  ((y) == JULIAN_MINYEAR && ((m) >= JULIAN_MINMONTH))) && \
	 ((y) < JULIAN_MAXYEAR || \
	  ((y) == JULIAN_MAXYEAR && ((m) < JULIAN_MAXMONTH))))

/* Julian-date equivalents of Day 0 in Unix and Postgres reckoning */
#define UNIX_EPOCH_JDATE		2440588 /* == date2j(1970, 1, 1) */
#define POSTGRES_EPOCH_JDATE	2451545 /* == date2j(2000, 1, 1) */

/*
 * Range limits for dates and timestamps.
 *
 * We have traditionally allowed Julian day zero as a valid datetime value,
 * so that is the lower bound for both dates and timestamps.
 *
 * The upper limit for dates is 5874897-12-31, which is a bit less than what
 * the Julian-date code can allow.  For timestamps, the upper limit is
 * 294276-12-31.  The int64 overflow limit would be a few days later; again,
 * leaving some slop avoids worries about corner-case overflow, and provides
 * a simpler user-visible definition.
 */

/* First allowed date, and first disallowed date, in Julian-date form */
#define DATETIME_MIN_JULIAN (0)
#define DATE_END_JULIAN (2147483494)	/* == date2j(JULIAN_MAXYEAR, 1, 1) */
#define TIMESTAMP_END_JULIAN (109203528)	/* == date2j(294277, 1, 1) */

/* Timestamp limits */
#define MIN_TIMESTAMP	INT64CONST(-211813488000000000)
/* == (DATETIME_MIN_JULIAN - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY */
#define END_TIMESTAMP	INT64CONST(9223371331200000000)
/* == (TIMESTAMP_END_JULIAN - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY */

#define INT64_FORMAT "%" INT64_MODIFIER "d"
#define UINT64_FORMAT "%" INT64_MODIFIER "u"


/* Maximum length of a timezone name (not including trailing null) */
#define TZ_STRLEN_MAX 255

/* Range-check a date (given in Postgres, not Julian, numbering) */
#define IS_VALID_DATE(d) \
	((DATETIME_MIN_JULIAN - POSTGRES_EPOCH_JDATE) <= (d) && \
	 (d) < (DATE_END_JULIAN - POSTGRES_EPOCH_JDATE))

/* Range-check a timestamp */
#define IS_VALID_TIMESTAMP(t)  (MIN_TIMESTAMP <= (t) && (t) < END_TIMESTAMP)

typedef size_t Size;
typedef unsigned int Oid;
#define FLEXIBLE_ARRAY_MEMBER	/* empty */
#define VARHDRSZ		((int32) sizeof(int32))

#define DEFAULTOID 0
#define BOOLOID 16
#define BYTEAOID 17
#define CHAROID 18
#define NAMEOID 19
#define INT8OID 20
#define INT2OID 21
#define INT2VECTOROID 22
#define INT4OID 23
#define REGPROCOID 24
#define TEXTOID 25
#define OIDOID 26
#define TIDOID 27
#define XIDOID 28
#define CIDOID 29
#define OIDVECTOROID 30
#define JSONOID 114
#define XMLOID 142
#define PG_NODE_TREEOID 194
#define PG_NDISTINCTOID 3361
#define PG_DEPENDENCIESOID 3402
#define PG_MCV_LISTOID 5017
#define PG_DDL_COMMANDOID 32
#define XID8OID 5069
#define POINTOID 600
#define LSEGOID 601
#define PATHOID 602
#define BOXOID 603
#define POLYGONOID 604
#define LINEOID 628
#define FLOAT4OID 700
#define FLOAT8OID 701
#define UNKNOWNOID 705
#define CIRCLEOID 718
#define MONEYOID 790
#define MACADDROID 829
#define INETOID 869
#define CIDROID 650
#define MACADDR8OID 774
#define ACLITEMOID 1033
#define BPCHAROID 1042
#define VARCHAROID 1043
#define DATEOID 1082
#define TIMEOID 1083
#define TIMESTAMPOID 1114
#define TIMESTAMPTZOID 1184
#define INTERVALOID 1186
#define TIMETZOID 1266
#define BITOID 1560
#define VARBITOID 1562
#define NUMERICOID 1700
#define REFCURSOROID 1790
#define REGPROCEDUREOID 2202
#define REGOPEROID 2203
#define REGOPERATOROID 2204
#define REGCLASSOID 2205
#define REGCOLLATIONOID 4191
#define REGTYPEOID 2206
#define REGROLEOID 4096
#define REGNAMESPACEOID 4089
#define UUIDOID 2950
#define PG_LSNOID 3220
#define TSVECTOROID 3614
#define GTSVECTOROID 3642
#define TSQUERYOID 3615
#define REGCONFIGOID 3734
#define REGDICTIONARYOID 3769
#define JSONBOID 3802
#define JSONPATHOID 4072
#define TXID_SNAPSHOTOID 2970
#define PG_SNAPSHOTOID 5038
#define INT4RANGEOID 3904
#define NUMRANGEOID 3906
#define TSRANGEOID 3908
#define TSTZRANGEOID 3910
#define DATERANGEOID 3912
#define INT8RANGEOID 3926
#define INT4MULTIRANGEOID 4451
#define NUMMULTIRANGEOID 4532
#define TSMULTIRANGEOID 4533
#define TSTZMULTIRANGEOID 4534
#define DATEMULTIRANGEOID 4535
#define INT8MULTIRANGEOID 4536
#define RECORDOID 2249
#define RECORDARRAYOID 2287
#define CSTRINGOID 2275
#define ANYOID 2276
#define ANYARRAYOID 2277
#define VOIDOID 2278
#define TRIGGEROID 2279
#define EVENT_TRIGGEROID 3838
#define LANGUAGE_HANDLEROID 2280
#define INTERNALOID 2281
#define ANYELEMENTOID 2283
#define ANYNONARRAYOID 2776
#define ANYENUMOID 3500
#define FDW_HANDLEROID 3115
#define INDEX_AM_HANDLEROID 325
#define TSM_HANDLEROID 3310
#define TABLE_AM_HANDLEROID 269
#define ANYRANGEOID 3831
#define ANYCOMPATIBLEOID 5077
#define ANYCOMPATIBLEARRAYOID 5078
#define ANYCOMPATIBLENONARRAYOID 5079
#define ANYCOMPATIBLERANGEOID 5080
#define ANYMULTIRANGEOID 4537
#define ANYCOMPATIBLEMULTIRANGEOID 4538
#define PG_BRIN_BLOOM_SUMMARYOID 4600
#define PG_BRIN_MINMAX_MULTI_SUMMARYOID 4601
#define BOOLARRAYOID 1000
#define BYTEAARRAYOID 1001
#define CHARARRAYOID 1002
#define NAMEARRAYOID 1003
#define INT8ARRAYOID 1016
#define INT2ARRAYOID 1005
#define INT2VECTORARRAYOID 1006
#define INT4ARRAYOID 1007
#define REGPROCARRAYOID 1008
#define TEXTARRAYOID 1009
#define OIDARRAYOID 1028
#define TIDARRAYOID 1010
#define XIDARRAYOID 1011
#define CIDARRAYOID 1012
#define OIDVECTORARRAYOID 1013
#define PG_TYPEARRAYOID 210
#define PG_ATTRIBUTEARRAYOID 270
#define PG_PROCARRAYOID 272
#define PG_CLASSARRAYOID 273
#define JSONARRAYOID 199
#define XMLARRAYOID 143
#define XID8ARRAYOID 271
#define POINTARRAYOID 1017
#define LSEGARRAYOID 1018
#define PATHARRAYOID 1019
#define BOXARRAYOID 1020
#define POLYGONARRAYOID 1027
#define LINEARRAYOID 629
#define FLOAT4ARRAYOID 1021
#define FLOAT8ARRAYOID 1022
#define CIRCLEARRAYOID 719
#define MONEYARRAYOID 791
#define MACADDRARRAYOID 1040
#define INETARRAYOID 1041
#define CIDRARRAYOID 651
#define MACADDR8ARRAYOID 775
#define ACLITEMARRAYOID 1034
#define BPCHARARRAYOID 1014
#define VARCHARARRAYOID 1015
#define DATEARRAYOID 1182
#define TIMEARRAYOID 1183
#define TIMESTAMPARRAYOID 1115
#define TIMESTAMPTZARRAYOID 1185
#define INTERVALARRAYOID 1187
#define TIMETZARRAYOID 1270
#define BITARRAYOID 1561
#define VARBITARRAYOID 1563
#define NUMERICARRAYOID 1231
#define REFCURSORARRAYOID 2201
#define REGPROCEDUREARRAYOID 2207
#define REGOPERARRAYOID 2208
#define REGOPERATORARRAYOID 2209
#define REGCLASSARRAYOID 2210
#define REGCOLLATIONARRAYOID 4192
#define REGTYPEARRAYOID 2211
#define REGROLEARRAYOID 4097
#define REGNAMESPACEARRAYOID 4090
#define UUIDARRAYOID 2951
#define PG_LSNARRAYOID 3221
#define TSVECTORARRAYOID 3643
#define GTSVECTORARRAYOID 3644
#define TSQUERYARRAYOID 3645
#define REGCONFIGARRAYOID 3735
#define REGDICTIONARYARRAYOID 3770
#define JSONBARRAYOID 3807
#define JSONPATHARRAYOID 4073
#define TXID_SNAPSHOTARRAYOID 2949
#define PG_SNAPSHOTARRAYOID 5039
#define INT4RANGEARRAYOID 3905
#define NUMRANGEARRAYOID 3907
#define TSRANGEARRAYOID 3909
#define TSTZRANGEARRAYOID 3911
#define DATERANGEARRAYOID 3913
#define INT8RANGEARRAYOID 3927
#define INT4MULTIRANGEARRAYOID 6150
#define NUMMULTIRANGEARRAYOID 6151
#define TSMULTIRANGEARRAYOID 6152
#define TSTZMULTIRANGEARRAYOID 6153
#define DATEMULTIRANGEARRAYOID 6155
#define INT8MULTIRANGEARRAYOID 6157
#define CSTRINGARRAYOID 1263


#if defined(__cplusplus)
#define unconstify(underlying_type, expr) const_cast<underlying_type>(expr)
#define unvolatize(underlying_type, expr) const_cast<underlying_type>(expr)
#elif defined(HAVE__BUILTIN_TYPES_COMPATIBLE_P)
#define StaticAssertStmt(condition, errmessage) \
	do { _Static_assert(condition, errmessage); } while(0)
#define StaticAssertExpr(condition, errmessage) \
	((void) ({ StaticAssertStmt(condition, errmessage); true; }))
#define unconstify(underlying_type, expr) \
	(StaticAssertExpr(__builtin_types_compatible_p(__typeof(expr), const underlying_type), \
					  "wrong cast"), \
	 (underlying_type) (expr))
#define unvolatize(underlying_type, expr) \
	(StaticAssertExpr(__builtin_types_compatible_p(__typeof(expr), volatile underlying_type), \
					  "wrong cast"), \
	 (underlying_type) (expr))
#else
#define unconstify(underlying_type, expr) \
	((underlying_type) (expr))
#define unvolatize(underlying_type, expr) \
	((underlying_type) (expr))
#endif

typedef Oid RelFileNumber;


#endif //PG_HEXRETRO_PG_C_H
