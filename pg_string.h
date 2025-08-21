//
// Created by 白杰 on 2025/8/9.
//

#ifndef PG_SNPDUMP_PG_STRING_H
#define PG_SNPDUMP_PG_STRING_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include "pg_common.h"
using namespace std;

void decode_bool(CtidNode* tuple, int, uint32_t *);

void decode_uuid(CtidNode* tuple, int, uint32_t *);

void decode_string(CtidNode* tuple, int, uint32_t *);

void decode_char(CtidNode* tuple, int, uint32_t *);

void decode_name(CtidNode* tuple, int, uint32_t *);

void decode_json(CtidNode* tuple, int, uint32_t *);

void decode_jsonb(CtidNode* tuple, int, uint32_t *);

void decode_inet(CtidNode* tuple, int, uint32_t *);

void decode_cidr(CtidNode* tuple, int, uint32_t *);

void decode_hstore(CtidNode* tuple, int, uint32_t *);

#endif //PG_SNPDUMP_PG_STRING_H
