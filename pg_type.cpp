//
// Created by 白杰 on 2025/8/9.
//

#include "pg_type.h"

#ifdef defined(__linux__)
child_process_kind child_process_kinds[] = {
        {"default", &decode_default},
        {"smallint", &decode_smallint},
        {"int", &decode_int},
        {"bigint", &decode_bigint},
        {"oid", &decode_oid},
        {"numeric", &decode_numeric},
        {"name", &decode_name},
        {"float", &decode_float4},
        {"double", &decode_float8},
        {"varchar", &decode_string},
        {"text", &decode_string},
        {"bytea", &decode_string},
        {"char", &decode_char},
        {"bool", &decode_bool},
        {"date", &decode_date},
        {"time", &decode_time},
        {"timestamp", &decode_timestamp},
        {"bpchar", &decode_string},
        {"timestamp with time zone", &decode_timetz},
        {"interval", &decode_interval},
        {"uuid", &decode_uuid},
        {"inet", &decode_inet},
        {"cidr", &decode_cidr},
        {"macaddr", &decode_macaddr},
        {"json", &decode_string},
        {"xml", &decode_string},
        {"jsonb", &decode_jsonb},
        {"hstore", &decode_hstore},
        {"int4range", &decode_int4range},
        {"numrange", &decode_numrange},
        {"tsrange", &decode_tsrange},
        {"tstzrange", &decode_tstzrange},
        {"daterange", &decode_daterange},
};
#elif defined(__APPLE__)
child_process_kind child_process_kinds[] = {
        [T_DEFAULT] = {"default", &decode_default},
        [T_SMALLINT] = {"smallint", &decode_smallint},
        [T_INT] = {"int", &decode_int},
        [T_BIGINT] = {"bigint", &decode_bigint},
        [T_OID] = {"oid", &decode_oid},
        [T_NUMERIC] = {"numeric", &decode_numeric},
        [T_NAME] = {"name", &decode_name},
        [T_FLOAT] = {"float", &decode_float4},
        [T_DOUBLE] = {"double", &decode_float8},
        [T_VARCHAR] = {"varchar", &decode_string},
        [T_TEXT] = {"text", &decode_string},
        [T_BYTEA] = {"bytea", &decode_string},
        [T_CHAR] = {"char", &decode_char},
        [T_BOOL] = {"bool", &decode_bool},
        [T_DATE] = {"date", &decode_date},
        [T_TIME] = {"time", &decode_time},
        [T_TIMESTAMP] = {"timestamp", &decode_timestamp},
        [T_BPCHAR] = {"bpchar", &decode_string},
        [T_TIMESTAMPWITHTIMEZONE] = {"timestamp with time zone", &decode_timetz},
        [T_INTERVAL] = {"interval", &decode_interval},
        [T_UUID] = {"uuid", &decode_uuid},
        [T_INET] = {"inet", &decode_inet},
        [T_CIDR] = {"cidr", &decode_cidr},
        [T_MACADDR] = {"macaddr", &decode_macaddr},
        [T_JSON] = {"json", &decode_string},
        [T_XML] = {"xml", &decode_string},
        [T_JSONB] = {"jsonb", &decode_string},
        [T_HSTORE] = {"hstore", &decode_hstore},
        [T_INT4RANGE] = {"int4range", &decode_int4range},
        [T_NUMRANGE] = {"numrange", &decode_numrange},
        [T_TSRANGE] = {"tsrange", &decode_tsrange},
        [T_TSTZRANGE] = {"tstzrange", &decode_tstzrange},
        [T_DATERANGE] = {"daterange", &decode_daterange},
};
#endif

int tupleFetchType(CtidNode* tuple, int colSeq, uint32_t * offset) {
    switch (tuple->tuple.column_type_id[colSeq]) {
        case 19:
            child_process_kinds[T_NAME].main_fn(tuple, colSeq, offset);
            break;
        case 21:
            child_process_kinds[T_SMALLINT].main_fn(tuple, colSeq, offset);
            break;
        case 20:
            child_process_kinds[T_BIGINT].main_fn(tuple, colSeq, offset);
            
            break;
        case 23:
            child_process_kinds[T_INT].main_fn(tuple, colSeq, offset);
            
            break;
        case 26:
            child_process_kinds[T_OID].main_fn(tuple, colSeq, offset);
            
            break;
        case 1700:
            child_process_kinds[T_NUMERIC].main_fn(tuple, colSeq, offset);
            
            break;
        case 700:
            child_process_kinds[T_FLOAT].main_fn(tuple, colSeq, offset);
            
            break;
        case 701:
            child_process_kinds[T_DOUBLE].main_fn(tuple, colSeq, offset);
            
            break;
        case 18:
            child_process_kinds[T_CHAR].main_fn(tuple, colSeq, offset);
            
            break;
        case 1043:
            child_process_kinds[T_VARCHAR].main_fn(tuple, colSeq, offset);
            
            break;
        case 25:
            child_process_kinds[T_TEXT].main_fn(tuple, colSeq, offset);
            
            break;
        case 17:
            child_process_kinds[T_BYTEA].main_fn(tuple, colSeq, offset);
            
            break;
        case 16:
            child_process_kinds[T_BOOL].main_fn(tuple, colSeq, offset);
            
            break;
        case 1082:
            child_process_kinds[T_DATE].main_fn(tuple, colSeq, offset);
            
            break;
        case 1083:
            child_process_kinds[T_TIME].main_fn(tuple, colSeq, offset);
            
            break;
        case 1114:
            child_process_kinds[T_TIMESTAMP].main_fn(tuple, colSeq, offset);
            
            break;
        case 1042:
            child_process_kinds[T_BPCHAR].main_fn(tuple, colSeq, offset);
            
            break;
        case 1184:
            child_process_kinds[T_TIMESTAMPWITHTIMEZONE].main_fn(tuple, colSeq, offset);
            break;
        case 1186:
            child_process_kinds[T_INTERVAL].main_fn(tuple, colSeq, offset);
            break;
        case 2950:
            child_process_kinds[T_UUID].main_fn(tuple, colSeq, offset);
            break;
        case 869:
            child_process_kinds[T_INET].main_fn(tuple, colSeq, offset);
            break;
        case 650:
            child_process_kinds[T_CIDR].main_fn(tuple, colSeq, offset);
            break;
        case 829:
            child_process_kinds[T_MACADDR].main_fn(tuple, colSeq, offset);
            break;
        case 114:
            child_process_kinds[T_JSON].main_fn(tuple, colSeq, offset);
            break;
        case 142:
            child_process_kinds[T_XML].main_fn(tuple, colSeq, offset);
            break;
        case 3802:
            child_process_kinds[T_JSONB].main_fn(tuple, colSeq, offset);
            break;
        case 1033:
            child_process_kinds[T_HSTORE].main_fn(tuple, colSeq, offset);
            break;
        case 3904:
            child_process_kinds[T_INT4RANGE].main_fn(tuple, colSeq, offset);
            break;
        case 3906:
            child_process_kinds[T_NUMRANGE].main_fn(tuple, colSeq, offset);
            break;
        case 3908:
            child_process_kinds[T_TSRANGE].main_fn(tuple, colSeq, offset);
            break;
        case 3910:
            child_process_kinds[T_TSTZRANGE].main_fn(tuple, colSeq, offset);
            break;
        case 3912:
            child_process_kinds[T_DATERANGE].main_fn(tuple, colSeq, offset);
            break;
        default:
            child_process_kinds[T_DEFAULT].main_fn(tuple, colSeq, offset);
            break;
    }
    return 1;
};