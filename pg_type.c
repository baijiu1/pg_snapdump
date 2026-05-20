//
// Created by 白杰 on 2026/5/20.
//

#include "pg_type.h"

void register_type(Oid oid, TypeHandler *handler)
{
    type_table[oid] = handler;
}

TypeHandler* get_type_handler(Oid oid)
{
    return type_table[oid];
}

TypeHandler *type_table[MAX_TYPE] = {0};

void init_type_system()
{
/* ===== default ===== */
    static TypeHandler ns_default = { "default", ns_decode_default };
    register_type(DEFAULTOID, &ns_default);

    /* ===== numeric types ===== */
    static TypeHandler ns_smallint = { "smallint", ns_decode_int2 };
    static TypeHandler ns_int4 = { "int4", ns_decode_int4 };
    static TypeHandler ns_int8 = { "int8", ns_decode_int8 };
    static TypeHandler ns_numeric = { "numeric", ns_decode_str };

    register_type(INT2OID, &ns_smallint);  // int2
    register_type(INT4OID, &ns_int4);      // int4
    register_type(INT8OID, &ns_int8);      // int8
    register_type(NUMERICOID, &ns_numeric);  // numeric

    /* ===== oid ===== */
    static TypeHandler ns_oid = { "oid", ns_decode_oid };
    register_type(OIDOID, &ns_oid);

    /* ===== float ===== */
    static TypeHandler ns_float = { "float", ns_decode_float };
    static TypeHandler ns_double = { "double", ns_decode_double };

    register_type(FLOAT4OID, &ns_float);   // float4
    register_type(FLOAT8OID, &ns_double);  // float8

    /* ===== string types ===== */
    static TypeHandler ns_varchar = { "varchar", ns_decode_str };
    static TypeHandler ns_text = { "text", ns_decode_str };
    static TypeHandler ns_bytea = { "bytea", ns_decode_str };
    static TypeHandler ns_char = { "char", ns_decode_char };
    static TypeHandler ns_bpchar = { "bpchar", ns_decode_str };
    static TypeHandler ns_name = { "name", ns_decode_name };


    register_type(VARCHAROID, &ns_varchar);
    register_type(TEXTOID, &ns_text);
    register_type(BYTEAOID, &ns_bytea);
    register_type(CHAROID, &ns_char);
    register_type(NAMEOID, &ns_name);
    register_type(BPCHAROID, &ns_bpchar);

    /* ===== bool ===== */
    static TypeHandler ns_bool = { "bool", ns_decode_bool };
    register_type(BOOLOID, &ns_bool);

    /* ===== date/time ===== */
    static TypeHandler ns_date = { "date", ns_decode_date };
    static TypeHandler ns_time = { "time", ns_decode_time };
    static TypeHandler ns_timestamp = { "timestamp", ns_decode_timestamp };
    static TypeHandler ns_timestampz = { "timestampz", ns_decode_timestampz };
    static TypeHandler ns_timetz = { "timetz", ns_decode_timetz };
    static TypeHandler ns_interval = { "interval", ns_decode_interval };

    register_type(DATEOID, &ns_date);
    register_type(TIMEOID, &ns_time);
    register_type(TIMESTAMPOID, &ns_timestamp);
    register_type(TIMETZOID, &ns_timetz);
    register_type(INTERVALOID, &ns_interval);
    register_type(TIMESTAMPTZOID, &ns_timestampz);

    /* ===== uuid / network ===== */
    static TypeHandler ns_uuid = { "uuid", ns_decode_uuid };
    static TypeHandler ns_inet = { "inet", ns_decode_inet };
    static TypeHandler ns_cidr = { "cidr", ns_decode_cidr };
    static TypeHandler ns_macaddr = { "macaddr", ns_decode_macaddr };

    register_type(UUIDOID, &ns_uuid);
    register_type(INETOID, &ns_inet);
    register_type(CIDROID, &ns_cidr);
    register_type(MACADDROID, &ns_macaddr);

    /* ===== json / hstore ===== */
    static TypeHandler ns_json = { "json", ns_decode_str };
    static TypeHandler ns_jsonb = { "jsonb", ns_decode_str };

    static TypeHandler ns_hstore = { "hstore", ns_decode_hstore };

    register_type(JSONOID, &ns_json);
    register_type(JSONBOID, &ns_jsonb);
    register_type(336, &ns_hstore);

    /* ===== range types ===== */
    static TypeHandler ns_int4range = { "int4range", ns_decode_int4range };
    static TypeHandler ns_numrange = { "numrange", ns_decode_numrange };
    static TypeHandler ns_tsrange = { "tsrange", ns_decode_tsrange };
    static TypeHandler ns_tstzrange = { "tstzrange", ns_decode_tstzrange };
    static TypeHandler ns_daterange = { "daterange", ns_decode_daterange };

    register_type(INT4RANGEOID, &ns_int4range);
    register_type(NUMRANGEOID, &ns_numrange);
    register_type(TSRANGEOID, &ns_tsrange);
    register_type(TSTZRANGEOID, &ns_tstzrange);
    register_type(DATERANGEOID, &ns_daterange);

    /* ===== extra types ===== */
    static TypeHandler ns_tid = { "tid", ns_decode_tid };
    static TypeHandler ns_xid = { "xid", ns_decode_xid };
    static TypeHandler ns_cid = { "cid", ns_decode_cid };

    register_type(TIDOID, &ns_tid);
    register_type(XIDOID, &ns_xid);
    register_type(CIDOID, &ns_cid);
}

Datum decode_value(char *data, ColumnInfo *col)
{
    init_type_system();
    TypeHandler *handler = get_type_handler(col->atttypid);

    if (!handler || !handler->decode) {
//        LOG_DEBUG("unknown type oid=%d", col->atttypid);
        return 0;
    }
    Datum val = handler->decode(data);

    return val;
}

static inline float4
DatumGetFloat4(Datum X)
{
    union
    {
        int32		value;
        float4		retval;
    }			myunion;

    myunion.value = DatumGetInt32(X);
    return myunion.retval;
}

static inline float8
DatumGetFloat8(Datum X)
{
#ifdef USE_FLOAT8_BYVAL
    union
	{
		int64		value;
		float8		retval;
	}			myunion;

	myunion.value = DatumGetInt64(X);
	return myunion.retval;
#else
    return *((float8 *) DatumGetPointer(X));
#endif
}



char *datum_to_string(ScanContext *ctx, Datum d, Oid typid)
{
    char buf[128];

    switch (typid)
    {
        case INT2OID:
            snprintf(buf, sizeof(buf), "%d", DatumGetInt16(d));
            return strdup(buf);

        case INT4OID:
            snprintf(buf, sizeof(buf), "%d", DatumGetInt32(d));
            return strdup(buf);

        case INT8OID:
            snprintf(buf, sizeof(buf), "%lld", (long long)DatumGetInt64(d));
            return strdup(buf);

        case FLOAT4OID:
            snprintf(buf, sizeof(buf), "%f", DatumGetFloat4(d));
            return strdup(buf);

        case FLOAT8OID:
            snprintf(buf, sizeof(buf), "%lf", DatumGetFloat8(d));
            return strdup(buf);

        case BOOLOID:
            return strdup(DatumGetBool(d) ? "true" : "false");
        case TEXTOID:
        case VARCHAROID:
        case BPCHAROID:
            return strdup(TextDatumGetCString(ctx, d));
        case BYTEAOID:
            return ByteaDatumToCString(ctx, d);
        case TIDOID:
            // ctid
        {
            ItemPointerData ctid;
            memcpy(&ctid, &d, sizeof(ItemPointerData));

            uint32 block =
                    ((uint32)ctid.ip_blkid.bi_hi << 16) |
                    (uint32)ctid.ip_blkid.bi_lo;

            char buf[64];
            snprintf(buf, sizeof(buf), "(%u,%u)", block, ctid.ip_posid);

            return strdup(buf);
        }
        case XIDOID:
            //xmin xmax
        {
            snprintf(buf, sizeof(buf), "%d", DatumGetInt32(d));
            return strdup(buf);
        }
        case CIDOID:
            // cmin cmax
        {

        }
        case OIDOID:
            // tableoid
        {
            snprintf(buf, sizeof(buf), "%d", DatumGetInt32(d));
            return strdup(buf);
        }
        case TIMESTAMPTZOID:
        {
            TimestampTz ts = DatumGetTimestampTz(d);
            char *str = timestamptz_to_str(ts);
            return strdup(str);
        }

        case TIMESTAMPOID:
        {
            TimestampTz ts = DatumGetTimestampTz(d);
            char *str = timestamp_to_str(ts);
            return strdup(str);
        }

        case DATEOID:
            return strdup(date_to_str(d));
        case TIMEOID:
            return strdup(time_to_str(d));
        case JSONBOID:
        {

            Jsonb *jb = DatumGetJsonb(ctx, d);

            char *out;

            int pos = 0;
            return strdup(JsonbToCString(NULL, &jb->root, VARSIZE(jb)));
        }
        case NUMERICOID:
        {
            Numeric		num = DatumGetNumeric(ctx, d);
            return strdup(numeric_to_str(num));
        }
        default:
        {
            char unknown[64];
            snprintf(unknown, sizeof(unknown), "[unsupported:%u]", typid);
            return strdup(unknown);
        }
    }
}



char * text_to_cstring(ScanContext *ctx, const text *t)
{
    /* must cast away the const, unfortunately */
    text	   *tunpacked = pg_detoast_datum_packed(ctx, unconstify(text *, t));
    int			len = VARSIZE_ANY_EXHDR(tunpacked);
    char	   *result;

    result = (char *) malloc(len + 1);
    memcpy(result, VARDATA_ANY(tunpacked), len);
    result[len] = '\0';

    if (tunpacked != t)
        free(tunpacked);

    return result;
}