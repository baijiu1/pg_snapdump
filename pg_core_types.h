//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_CORE_TYPES_H
#define PG_HEXRETRO_PG_CORE_TYPES_H
#include "pg_c.h"
#define MAX_TYPE 16000

typedef struct Engine Engine;
typedef struct Access Access;
typedef struct Relocator Relocator;
typedef struct ScanContext ScanContext;
typedef struct tupleSlot tupleSlot;
typedef struct tupleMVCC tupleMVCC;

typedef struct TypeHandler {
    const char *name;

    Datum (*decode)(char *ptr);
} TypeHandler;
extern TypeHandler *type_table[MAX_TYPE];

#endif //PG_HEXRETRO_PG_CORE_TYPES_H
