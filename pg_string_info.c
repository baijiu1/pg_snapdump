//
// Created by 白杰 on 2026/5/20.
//

#include "pg_string_info.h"


void
enlargeStringInfo(StringInfo str, int needed)
{
    int		newlen;
    int		limit;
    char	   *old_data;

    limit = MaxAllocSize;

    /*
     * Guard against out-of-range "needed" values.  Without this, we can get
     * an overflow or infinite loop in the following.
     */
    if (needed < 0)				/* should not happen */
    {
        printf("Error: invalid string enlargement request size: %d", needed);
        exit(1);
    }

    if (((int) needed) >= (limit - (int) str->len))
    {
        printf("Error: cannot enlarge string buffer containing %d bytes by %d more bytes.",
               str->len, needed);
        exit(1);
    }

    needed += str->len + 1;		/* total space required now */

    /* Because of the above test, we now have needed <= limit */

    if (needed <= str->maxlen)
        return;					/* got enough space already */

    /*
     * We don't want to allocate just a little more space with each append;
     * for efficiency, double the buffer size each time it overflows.
     * Actually, we might need to more than double it if 'needed' is big...
     */
    newlen = 2 * str->maxlen;
    while (needed > newlen)
        newlen = 2 * newlen;

    /*
     * Clamp to the limit in case we went past it.  Note we are assuming here
     * that limit <= INT_MAX/2, else the above loop could overflow.  We will
     * still have newlen >= needed.
     */
    if (newlen > limit)
        newlen = limit;

    old_data = str->data;
    str->data = (char *) realloc(str->data, (int) newlen);
    if (str->data == NULL)
    {
        free(old_data);
        printf("Error: realloc() failed!\n");
        exit(1);
    }

    str->maxlen = newlen;
}


void
#if PG_VERSION_NUM < 160000
appendBinaryStringInfo(StringInfo str, const char *data, int datalen)
#else
appendBinaryStringInfo(StringInfo str, const void *data, int datalen)
#endif
{
    assert(str != NULL);

    /* Make more room if needed */
    enlargeStringInfo(str, datalen);

    /* OK, append the data */
    memcpy(str->data + str->len, data, datalen);
    str->len += datalen;

    /*
     * Keep a trailing null in place, even though it's probably useless for
     * binary data.  (Some callers are dealing with text but call this because
     * their input isn't null-terminated.)
     */
    str->data[str->len] = '\0';
}

void
appendStringInfoString(StringInfo str, const char *s)
{
    appendBinaryStringInfo(str, s, strlen(s));
}

void
resetStringInfo(StringInfo str)
{
    str->data[0] = '\0';
    str->len = 0;
    str->cursor = 0;
}

void
initStringInfo(StringInfo str)
{
//    printf("\ninit StringInfo!\n");
    int			size = 1024;	/* initial default buffer size */

    str->data = (char *) malloc(size);
    str->maxlen = size;
    resetStringInfo(str);
}

StringInfo
makeStringInfo(void)
{
    StringInfo	res;

    res = (StringInfo) malloc(sizeof(StringInfoData));

    initStringInfo(res);

    return res;
}

static bool copyStringInitDone = false;
static StringInfoData copyString;
/* Append given string to current COPY line */
char* CopyAppend(const char *str)
{
    if (!copyStringInitDone)
    {
        initStringInfo(&copyString);
        copyStringInitDone = true;
    }

    /* Caller probably wanted just to init copyString */
    if (str == NULL)
        return "0";

    if (copyString.data[0] != '\0')
        appendStringInfoString(&copyString, "\t");

    appendStringInfoString(&copyString, str);
    char* result = strdup(str);
//    printf(" %s ", result);
    return result;
}