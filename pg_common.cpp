//
// Created by 白杰 on 2025/8/10.
//

#include "pg_common.h"
#include "cstdio"
#include "cstdlib"

using namespace std;

size_t computeAttAlign(uint32_t& offset, CtidNode*& tuple, int colSeq) {
    uint32 startOffset = offset;
    if (tuple->tuple.colAttlen[colSeq] == -1) {
        offset = att_align_pointer(offset, tuple->tuple.colAttalign[colSeq], tuple->tuple.colAttlen[colSeq], tuple->tuple.cache_data + offset);
    } else {
        offset = att_align_nominal(offset, tuple->tuple.colAttalign[colSeq]);
    }
    offset = att_addlength_pointer(offset, tuple->tuple.colAttlen[colSeq], tuple->tuple.cache_data + offset);
    return offset - startOffset;
};

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


static bool copyStringInitDone = false;
static StringInfoData copyString;
/* Append given string to current COPY line */
char*
CopyAppend(const char *str)
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
    printf(" %s ", result);
    return result;
}

int CopyAppendEncode(const unsigned char *str, int orig_len)
{
    int			curr_offset = 0;
    int			len = orig_len;
    char* tmp_buff = new char[2 * orig_len + 1];

    if (tmp_buff == NULL)
    {
        perror("malloc");
        exit(1);
    }

    while (len > 0)
    {
        /*
         * Since we are working with potentially corrupted data we can
         * encounter \0 as well.
         */
        if (*str == '\0')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = '0';
            curr_offset += 2;
        }
        else if (*str == '\r')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'r';
            curr_offset += 2;
        }
        else if (*str == '\n')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'n';
            curr_offset += 2;
        }
        else if (*str == '\t')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'r';
            curr_offset += 2;
        }
        else if (*str == '\\')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = '\\';
            curr_offset += 2;
        }
        else
        {
            /* It's a regular symbol. */
            tmp_buff[curr_offset] = *str;
            curr_offset++;
        }

        str++;
        len--;
    }

    tmp_buff[curr_offset] = '\0';
    CopyAppend(tmp_buff);
    delete[] tmp_buff;

    return 0;
}

/*
 * Decode a numeric type and append the result to current COPY line
 */
int
CopyAppendNumeric(const unsigned char *buffer, int num_size)
{

    struct NumericData *num = (struct NumericData *) malloc(num_size);
//    NumericData* num = reinterpret_cast<NumericData*>(new char[num_size]);


    if (num == NULL)
        return -2;

    memcpy((char *) num, buffer, num_size);
//    for (int i = 0; i < 15; ++i) {
//        printf(" %x ", buffer[i]);
//    }
    if (NUMERIC_IS_SPECIAL(num))
    {
        int	result = -2;

        if (NUMERIC_IS_NINF(num))
        {
            CopyAppend("-Infinity");
            result = 0;
        }
        if (NUMERIC_IS_PINF(num))
        {
            CopyAppend("Infinity");
            result = 0;
        }
        if (NUMERIC_IS_NAN(num))
        {
            CopyAppend("NaN");
            result = 0;
        }

        free(num);

        return result;
    }
    else
    {
        int			sign;
        int			weight;
        int			dscale;
        int			ndigits;
        int			i;
        char	   *str;
        char	   *cp;
        char	   *endcp;
        int			d;
        bool		putit;
        NumericDigit d1;
        NumericDigit dig;
        NumericDigit *digits;

        sign = NUMERIC_SIGN(num);
        weight = NUMERIC_WEIGHT(num);
        dscale = NUMERIC_DSCALE(num);
        if (num_size == NUMERIC_HEADER_SIZE(num))
        {
            /* No digits - compressed zero. */
            CopyAppendFmt("%d", 0);
            free(num);
            return 0;
        }
        else
        {
//            ndigits = num_size / sizeof(NumericDigit);
            ndigits = (num_size - NUMERIC_HEADER_SIZE(num)) / sizeof(NumericDigit);
            digits = (NumericDigit *) ((char *) num + NUMERIC_HEADER_SIZE(num));

            i = (weight + 1) * DEC_DIGITS;
            if (i <= 0)
                i = 1;

            str = new char[i + dscale + DEC_DIGITS + 2];
            cp = str;
            /*
             * Output a dash for negative values
             */
            if (sign == NUMERIC_NEG)
                *cp++ = '-';

            /*
             * Output all digits before the decimal point
             */
            if (weight < 0)
            {
                d = weight + 1;
                *cp++ = '0';
            }
            else
            {
                for (d = 0; d <= weight; d++)
                {
                    dig = (d < ndigits) ? digits[d] : 0;
                    /*
                     * In the first digit, suppress extra leading decimal
                     * zeroes
                     */
                    putit = (d > 0);
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
            }
            /*
             * If requested, output a decimal point and all the digits that
             * follow it. We initially put out a multiple of DEC_DIGITS
             * digits, then truncate if needed.
             */
            if (dscale > 0)
            {
                *cp++ = '.';
                endcp = cp + dscale;
                for (i = 0; i < dscale; d++, i += DEC_DIGITS)
                {
                    dig = (d >= 0 && d < ndigits) ? digits[d] : 0;
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
                cp = endcp;
            }
            *cp = '\0';
            CopyAppend(str);
            delete[] str;
            free(num);
            return 0;
        }
    }
}

int CopyAppendNumericValue(const char *buffer, int num_size)
{
    struct NumericData *num = (struct NumericData *) malloc(num_size);

    if (num == NULL)
        return -2;

    memcpy((char *) num, buffer, num_size);

    if (NUMERIC_IS_SPECIAL(num))
    {
        int	result = -2;

        if (NUMERIC_IS_NINF(num))
        {
            CopyAppend("-Infinity");
            result = 0;
        }
        if (NUMERIC_IS_PINF(num))
        {
            CopyAppend("Infinity");
            result = 0;
        }
        if (NUMERIC_IS_NAN(num))
        {
            CopyAppend("NaN");
            result = 0;
        }

        free(num);

        return result;
    }
    else
    {
        int			sign;
        int			weight;
        int			dscale;
        int			ndigits;
        int			i;
        char	   *str;
        char	   *cp;
        char	   *endcp;
        int			d;
        bool		putit;
        NumericDigit d1;
        NumericDigit dig;
        NumericDigit *digits;

        sign = NUMERIC_SIGN(num);
        weight = NUMERIC_WEIGHT(num);
        dscale = NUMERIC_DSCALE(num);

        if (num_size == NUMERIC_HEADER_SIZE(num))
        {
            /* No digits - compressed zero. */
//            CopyAppendFmt("%d", 0);
            free(num);
            return 0;
        }
        else
        {
//            ndigits = num_size / sizeof(NumericDigit);
            ndigits = (num_size - NUMERIC_HEADER_SIZE(num)) / sizeof(NumericDigit);
            digits = (NumericDigit *) ((char *) num + NUMERIC_HEADER_SIZE(num));

            i = (weight + 1) * DEC_DIGITS;
            if (i <= 0)
                i = 1;

            str = (char*)malloc(i + dscale + DEC_DIGITS + 2);
            cp = str;
            /*
             * Output a dash for negative values
             */
            if (sign == NUMERIC_NEG)
                *cp++ = '-';

            /*
             * Output all digits before the decimal point
             */
            if (weight < 0)
            {
                d = weight + 1;
                *cp++ = '0';
            }
            else
            {
                for (d = 0; d <= weight; d++)
                {
                    dig = (d < ndigits) ? digits[d] : 0;
                    /*
                     * In the first digit, suppress extra leading decimal
                     * zeroes
                     */
                    putit = (d > 0);
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
            }

            /*
             * If requested, output a decimal point and all the digits that
             * follow it. We initially put out a multiple of DEC_DIGITS
             * digits, then truncate if needed.
             */
            if (dscale > 0)
            {
                *cp++ = '.';
                endcp = cp + dscale;
                for (i = 0; i < dscale; d++, i += DEC_DIGITS)
                {
                    dig = (d >= 0 && d < ndigits) ? digits[d] : 0;
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
                cp = endcp;
            }
            *cp = '\0';
            CopyAppend(str);
            free(str);
            free(num);
            return 0;
        }
    }
}

/* ----------
 * pglz_decompress -
 *
 *        Decompresses source into dest. Returns the number of bytes
 *        decompressed in the destination buffer, and *optionally*
 *        checks that both the source and dest buffers have been
 *        fully read and written to, respectively.
 * ----------
 */
int32
pglz_decompress(const char *source, int32 slen, char *dest,
                int32 rawsize, bool check_complete)
{
    const unsigned char *sp;
    const unsigned char *srcend;
    unsigned char *dp;
    unsigned char *destend;

    sp = (const unsigned char *) source;
    srcend = ((const unsigned char *) source) + slen;
    dp = (unsigned char *) dest;
    destend = dp + rawsize;

    while (sp < srcend && dp < destend)
    {
        /*
         * Read one control byte and process the next 8 items (or as many as
         * remain in the compressed input).
         */
        unsigned char ctrl = *sp++;
        int            ctrlc;

        for (ctrlc = 0; ctrlc < 8 && sp < srcend && dp < destend; ctrlc++)
        {

            if (ctrl & 1)
            {
                /*
                 * Otherwise it contains the match length minus 3 and the
                 * upper 4 bits of the offset. The next following byte
                 * contains the lower 8 bits of the offset. If the length is
                 * coded as 18, another extension tag byte tells how much
                 * longer the match really was (0-255).
                 */
                int32        len;
                int32        off;

                len = (sp[0] & 0x0f) + 3;
                off = ((sp[0] & 0xf0) << 4) | sp[1];
                sp += 2;
                if (len == 18)
                    len += *sp++;

                /*
                 * Now we copy the bytes specified by the tag from OUTPUT to
                 * OUTPUT. It is dangerous and platform dependent to use
                 * memcpy() here, because the copied areas could overlap
                 * extremely!
                 */
                len = Min(len, destend - dp);
                while (len--)
                {
                    *dp = dp[-off];
                    dp++;
                }
            }
            else
            {
                /*
                 * An unset control bit means LITERAL BYTE. So we just copy
                 * one from INPUT to OUTPUT.
                 */
                *dp++ = *sp++;
            }

            /*
             * Advance the control bit
             */
            ctrl >>= 1;
        }
    }

    /*
     * Check we decompressed the right amount. If we are slicing, then we
     * won't necessarily be at the end of the source or dest buffers when we
     * hit a stop, so we don't test them.
     */
    if (check_complete && (dp != destend || sp != srcend))
        return -1;

    /*
     * That's it.
     */
    return (char *) dp - dest;
}


int extract_data(const unsigned char *buffer, int (*parse_value)(const unsigned char *, int), CtidNode* tuple, int colSeq, uint32_t * offset) {
    int			padding = 0;
    int			result	= 0;

    /* Skip padding bytes. */
    while (*buffer == 0)
    {
        buffer++;
        padding++;
    }

    if (VARATT_IS_1B_E(buffer))
    {
        /*
         * 00000001 1-byte length word, unaligned, TOAST pointer
         */
        uint32		len = VARSIZE_EXTERNAL(buffer);
        size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
        unsigned int blockOptions = 0;
        if (len > buff_size)
            return -1;

        if (blockOptions & BLOCK_DECODE_TOAST)
        {
//            result = ReadStringFromToast(buffer, buff_size, out_size, parse_value);
        }
        else if (VARATT_IS_EXTERNAL_ONDISK(buffer))
        {
            varatt_external toast_ptr;
            VARATT_EXTERNAL_GET_POINTER(toast_ptr, buffer);
            if (VARATT_EXTERNAL_IS_COMPRESSED(toast_ptr))
            {
#if PG_VERSION_NUM >= 140000
                switch (VARATT_EXTERNAL_GET_COMPRESS_METHOD(toast_ptr))
				{
					case TOAST_PGLZ_COMPRESSION_ID:
#endif
                CopyAppend("(TOASTED,pglz)");
#if PG_VERSION_NUM >= 140000
                break;
					case TOAST_LZ4_COMPRESSION_ID:
						CopyAppend("(TOASTED,lz4)");
						break;
					default:
						CopyAppend("(TOASTED,unknown)");
						break;
				}
#endif
            }
            else
                CopyAppend("(TOASTED,uncompressed)");
        }
            /* If tag is indirect or expanded, it was stored in memory. */
        else
            CopyAppend("(TOASTED IN MEMORY)");

//        *out_size = padding + len;
        return result;
    }

    if (VARATT_IS_1B(buffer))
    {
        /*
         * xxxxxxx1 1-byte length word, unaligned, uncompressed data (up to
         * 126b) xxxxxxx is 1 + string length
         */
        uint8		len = VARSIZE_1B(buffer);
        size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
        if (len > buff_size)
            return -1;
//        unsigned char numeric_buffer[len];
//        memcpy(numeric_buffer, buffer, len);
        result = parse_value(buffer + 1, len - 1);
        return result;
    }

//    if (VARATT_IS_4B_U(buffer) && buff_size >= 4)
    if (VARATT_IS_4B_U(buffer))
    {
        /*
         * xxxxxx00 4-byte length word, aligned, uncompressed data (up to 1G)
         */
        uint32		len = VARSIZE_4B(buffer);
        size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
        if (len > buff_size)
            return -1;

        result = parse_value(buffer + 4, len - 4);
        return result;
    }
//
//    if (VARATT_IS_4B_C(buffer) && buff_size >= 8)
    if (VARATT_IS_4B_C(buffer))
    {
        /*
         * xxxxxx10 4-byte length word, aligned, *compressed* data (up to 1G)
         */
        int						decompress_ret;
        uint32					len = VARSIZE_4B(buffer);
        size_t buff_size = computeAttAlign(*offset, tuple, colSeq);
        uint32					decompressed_len = 0;
        char				   *decompress_tmp_buff;
#if PG_VERSION_NUM >= 140000
        unsigned int cmid;
#endif

#if PG_VERSION_NUM >= 140000
        decompressed_len = VARDATA_COMPRESSED_GET_EXTSIZE(buffer);
#else
        decompressed_len = VARRAWSIZE_4B_C(buffer);
#endif

        if (len > buff_size)
            return -1;

        if ((decompress_tmp_buff = new char[decompressed_len]) == NULL)
        {
            perror("malloc");
            exit(1);
        }

#if PG_VERSION_NUM >= 140000
        cmid = VARDATA_COMPRESSED_GET_COMPRESS_METHOD(buffer);
		switch(cmid)
		{
			case TOAST_PGLZ_COMPRESSION_ID:
                for (int i = 0; i < 30; ++i) {
                    printf(" %x ", VARDATA_4B_C(buffer)[i]);
                }
				decompress_ret = pglz_decompress(VARDATA_4B_C(buffer), len - 2 * sizeof(uint32),
												 decompress_tmp_buff, decompressed_len, true);
                printf("\ndecompress_ret: %d\n", decompress_ret);
				break;
#ifdef USE_LZ4
			case TOAST_LZ4_COMPRESSION_ID:
				decompress_ret = LZ4_decompress_safe(VARDATA_4B_C(buffer), decompress_tmp_buff,
													 len - 2 * sizeof(uint32), decompressed_len);
				break;
#endif
			default:
				decompress_ret = -1;
				break;
		}
#else /* PG_VERSION_NUM < 140000 */
#if PG_VERSION_NUM >= 120000
        decompress_ret = pglz_decompress(VARDATA_4B_C(buffer), len - 2 * sizeof(uint32),
                                         decompress_tmp_buff, decompressed_len, true)
#else
        decompress_ret = pglz_decompress(VARDATA_4B_C(buffer), len - 2 * sizeof(uint32),
        decompress_tmp_buff, decompressed_len);
#endif
#endif /* PG_VERSION_NUM >= 140000 */

        if ((decompress_ret != decompressed_len) || (decompress_ret < 0))
        {
            printf("WARNING: Corrupted toast data, unable to decompress.\n");
            CopyAppend("(inline compressed, corrupted)");
//            *out_size = padding + len;
            delete[] decompress_tmp_buff;
//            free(decompress_tmp_buff);
            return 0;
        }

        result = parse_value((unsigned char*)decompress_tmp_buff, decompressed_len);
//        *out_size = padding + len;
//        free(decompress_tmp_buff);
        delete[] decompress_tmp_buff;
        return result;
    }

    return -9;
};

