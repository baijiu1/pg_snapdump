//
// Created by 白杰 on 2025/8/9.
//

#ifndef PG_SNPDUMP_PG_TYPE_H
#define PG_SNPDUMP_PG_TYPE_H

#include <cstdlib>
#include "string"
#include "cstdio"
#include "pg_common.h"
#include "pg_integer.h"
#include "pg_time.h"
#include "pg_string.h"
#include "pg_default.h"

using namespace std;

typedef struct
{
    const char *name;
    void		(*main_fn) (CtidNode*, int, uint32_t *);
} child_process_kind;


typedef enum BackendType
{
    T_SMALLINT = 0,
    T_INT,
    T_BIGINT,
    T_OID,
    T_NUMERIC,
    T_NAME,
    T_FLOAT,
    T_DOUBLE,
    T_VARCHAR,
    T_TEXT,
    T_BYTEA,
    T_CHAR,
    T_BOOL,
    T_DATE,
    T_TIME,
    T_TIMESTAMP,
    T_BPCHAR,
    T_TIMESTAMPWITHTIMEZONE,
    T_INTERVAL,
    T_UUID,
    T_INET,
    T_CIDR,
    T_MACADDR,
    T_JSON,
    T_XML,
    T_JSONB,
    T_HSTORE,
    T_INT4RANGE,
    T_NUMRANGE,
    T_TSRANGE,
    T_TSTZRANGE,
    T_DATERANGE,
    T_DEFAULT,
} BackendType;

int tupleFetchType(CtidNode*, int, uint32_t *);


#endif //PG_SNPDUMP_PG_TYPE_H
