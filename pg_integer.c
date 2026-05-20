//
// Created by 白杰 on 2026/5/20.
//

#include "pg_integer.h"

Datum ns_decode_int2(char *ptr)
{
    return *(int16_t *)ptr;
}

Datum ns_decode_int4(char *ptr)
{

    return *(int32_t *)ptr;
}

Datum ns_decode_int8(char *ptr)
{

    return *(int64_t *)ptr;
}

Datum ns_decode_oid(char *ptr)
{

    return *(uint32_t *)ptr;
}

Datum ns_decode_bool(char *ptr)
{

    return *(uint8_t *)ptr;
}
static inline Datum Float4GetDatum(float x)
{
    Datum d;
    memcpy(&d, &x, sizeof(float));
    return d;
}
Datum ns_decode_float(char *ptr)
{

    float v;
    memcpy(&v, ptr, 4);
    return Float4GetDatum(v);
//    return *(Datum *)&v;
}

static inline Datum Float8GetDatum(double x)
{
    Datum d;
    memcpy(&d, &x, sizeof(double));
    return d;
}

Datum ns_decode_double(char *ptr)
{

    double v;
//    Datum d;

    memcpy(&v, ptr, 8);
    return Float8GetDatum(v);
//    memcpy(&d, &v, sizeof(double));

//    return d;
}

Datum ns_decode_ns_numeric(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;



    return (Datum)(ptr + 4);
}

Datum ns_decode_int4range(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_numrange(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_tsrange(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_tstzrange(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_daterange(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Numeric
DatumGetNumeric(ScanContext *ctx, Datum X)
{
    struct varlena *v = PG_DETOAST_DATUM(ctx, X);

    return (Numeric) VARDATA(v);
}

struct varlena *
pg_detoast_datum(ScanContext *ctx, struct varlena *datum)
{
    if (VARATT_IS_EXTENDED(datum))
        return detoast_attr(ctx, datum);
    else
        return datum;
}

char* numeric_to_str(Numeric num) {
    NumericVar	x;
    char	   *str;

    if (NUMERIC_IS_SPECIAL(num))
    {
        if (NUMERIC_IS_PINF(num))
            return strdup("Infinity");
        else if (NUMERIC_IS_NINF(num))
            return strdup("-Infinity");
        else
            return strdup("NaN");
    }

    init_var_from_num(num, &x);
    str = get_str_from_var(&x);
    return str;
}

static void
init_var_from_num(Numeric num, NumericVar *dest)
{
    dest->ndigits = NUMERIC_NDIGITS(num);
    dest->weight = NUMERIC_WEIGHT(num);
    dest->sign = NUMERIC_SIGN(num);
    dest->dscale = NUMERIC_DSCALE(num);
    dest->digits = NUMERIC_DIGITS(num);
    dest->buf = NULL;			/* digits array is not palloc'd */
}

static char *
get_str_from_var(const NumericVar *var)
{
    int			dscale;
    char	   *str;
    char	   *cp;
    char	   *endcp;
    int			i;
    int			d;
    NumericDigit dig;

#if DEC_DIGITS > 1
    NumericDigit d1;
#endif

    dscale = var->dscale;

    /*
     * Allocate space for the result.
     *
     * i is set to the # of decimal digits before decimal point. dscale is the
     * # of decimal digits we will print after decimal point. We may generate
     * as many as DEC_DIGITS-1 excess digits at the end, and in addition we
     * need room for sign, decimal point, null terminator.
     */
    i = (var->weight + 1) * DEC_DIGITS;
    if (i <= 0)
        i = 1;

    str = (char*) malloc(i + dscale + DEC_DIGITS + 2);
    cp = str;

    /*
     * Output a dash for negative values
     */
    if (var->sign == NUMERIC_NEG)
        *cp++ = '-';

    /*
     * Output all digits before the decimal point
     */
    if (var->weight < 0)
    {
        d = var->weight + 1;
        *cp++ = '0';
    }
    else
    {
        for (d = 0; d <= var->weight; d++)
        {
            dig = (d < var->ndigits) ? var->digits[d] : 0;
            /* In the first digit, suppress extra leading decimal zeroes */
#if DEC_DIGITS == 4
            {
                bool		putit = (d > 0);

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
#elif DEC_DIGITS == 2
            d1 = dig / 10;
			dig -= d1 * 10;
			if (d1 > 0 || d > 0)
				*cp++ = d1 + '0';
			*cp++ = dig + '0';
#elif DEC_DIGITS == 1
            *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
        }
    }

    /*
     * If requested, output a decimal point and all the digits that follow it.
     * We initially put out a multiple of DEC_DIGITS digits, then truncate if
     * needed.
     */
    if (dscale > 0)
    {
        *cp++ = '.';
        endcp = cp + dscale;
        for (i = 0; i < dscale; d++, i += DEC_DIGITS)
        {
            dig = (d >= 0 && d < var->ndigits) ? var->digits[d] : 0;
#if DEC_DIGITS == 4
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
#elif DEC_DIGITS == 2
            d1 = dig / 10;
			dig -= d1 * 10;
			*cp++ = d1 + '0';
			*cp++ = dig + '0';
#elif DEC_DIGITS == 1
            *cp++ = dig + '0';
#else
#error unsupported NBASE
#endif
        }
        cp = endcp;
    }

    /*
     * terminate the string and return it
     */
    *cp = '\0';
    return str;
}