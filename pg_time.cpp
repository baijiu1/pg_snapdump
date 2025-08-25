//
// Created by 白杰 on 2025/8/9.
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

void decode_time(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [buff_size];
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (buff_size - sizeof(int64)), buff_size);
    /* Skip padding bytes. */
//    while (*buffer == 0x00)
//    {
//        buffer++;
//    }
    int64		timestamp,
            timestamp_sec;
    timestamp = *(int64 *) buffer;
    timestamp_sec = timestamp / 1000000;

    CopyAppendFmt("%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d.%06" INT64_MODIFIER "d",
                  timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
                  timestamp % 1000000);
};

void decode_timetz(CtidNode* tuple, int colSeq, uint32_t * offset){
    decode_timestamp_internal(tuple, colSeq, offset, true);
};

void decode_date(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [buff_size];
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (buff_size - sizeof(int32)), buff_size);

    int32		d,
            jd,
            year,
            month,
            day;

    if (buff_size < sizeof(int32))
        return;

    d = *(int32 *) buffer;
    if (d == PG_INT32_MIN)
    {
        CopyAppend("-infinity");
        return;
    }
    if (d == PG_INT32_MAX)
    {
        CopyAppend("infinity");
        return;
    }

    jd = d + POSTGRES_EPOCH_JDATE;
    j2date(jd, &year, &month, &day);

    CopyAppendFmt("%04d-%02d-%02d%s", (year <= 0) ? -year + 1 : year, month, day, (year <= 0) ? " BC" : "");
    delete[] buffer;
};

void decode_timestamp_internal(CtidNode* tuple, int colSeq, uint32_t * offset, bool with_timezone){
    uint32 startOffset = *offset;
    size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [buff_size];
    memcpy(buffer, tuple->tuple.cache_data + startOffset, buff_size);
    int64		timestamp,
            timestamp_sec;
    int32		jd,
            year,
            month,
            day;

    if (buff_size < sizeof(int64))
        return;

    memcpy(&timestamp, buffer + (buff_size - sizeof(timestamp)), sizeof(timestamp));
    timestamp = *(int64 *) buffer;

    if (timestamp == DT_NOBEGIN)
    {
        CopyAppend("-infinity");
        return;
    }
    if (timestamp == DT_NOEND)
    {
        CopyAppend("infinity");
        return;
    }

    jd = timestamp / USECS_PER_DAY;
    if (jd != 0)
        timestamp -= jd * USECS_PER_DAY;

    if (timestamp < INT64CONST(0))
    {
        timestamp += USECS_PER_DAY;
        jd -= 1;
    }

    /* add offset to go from J2000 back to standard Julian date */
    jd += POSTGRES_EPOCH_JDATE;

    j2date(jd, &year, &month, &day);
    timestamp_sec = timestamp / 1000000;

    CopyAppendFmt("%04d-%02d-%02d %02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d.%06" INT64_MODIFIER "d%s%s",
                  (year <= 0) ? -year + 1 : year, month, day,
                  timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
                  timestamp % 1000000,
                  with_timezone ? "+00" : "",
                  (year <= 0) ? " BC" : "");
};

void decode_timestamp(CtidNode* tuple, int colSeq, uint32_t * offset){
    decode_timestamp_internal(tuple, colSeq, offset, false);
};


void decode_interval(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_tsrange(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_tstzrange(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_daterange(CtidNode* tuple, int colSeq, uint32_t * offset){

};

