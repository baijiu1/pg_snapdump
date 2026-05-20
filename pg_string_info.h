//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_STRING_INFO_H
#define PG_HEXRETRO_PG_STRING_INFO_H
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "pg_c.h"

typedef struct StringInfoData
{
    char	   *data;
    int			len;
    int			maxlen;
    int			cursor;
} StringInfoData;

typedef StringInfoData *StringInfo;

void
enlargeStringInfo(StringInfo str, int needed);
void
#if PG_VERSION_NUM < 160000
appendBinaryStringInfo(StringInfo str, const char *data, int datalen);
#else
appendBinaryStringInfo(StringInfo str, const void *data, int datalen);
#endif
void
appendStringInfoString(StringInfo str, const char *s);
void
resetStringInfo(StringInfo str);
void
initStringInfo(StringInfo str);
char* CopyAppend(const char *str);
StringInfo makeStringInfo(void);
#endif //PG_HEXRETRO_PG_STRING_INFO_H
