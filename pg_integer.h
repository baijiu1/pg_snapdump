//
// Created by 白杰 on 2025/8/9.
//

#ifndef PG_SNPDUMP_PG_INTEGER_H
#define PG_SNPDUMP_PG_INTEGER_H

#include "cstdlib"
#include "cstdio"
#include "pg_common.h"

using namespace std;

void decode_smallint(CtidNode* tuple, int, uint32_t *);

void decode_int(CtidNode* tuple, int, uint32_t *);

void decode_uint(CtidNode* tuple, int, uint32_t *);

void decode_bigint(CtidNode* tuple, int, uint32_t *);

void decode_oid(CtidNode* tuple, int, uint32_t *);

void decode_float4(CtidNode* tuple, int, uint32_t *);

void decode_float8(CtidNode* tuple, int, uint32_t *);

void decode_numeric(CtidNode* tuple, int, uint32_t *);

void decode_macaddr(CtidNode* tuple, int, uint32_t *);

void decode_int4range(CtidNode* tuple, int, uint32_t *);

void decode_numrange(CtidNode* tuple, int, uint32_t *);
#endif //PG_SNPDUMP_PG_INTEGER_H
