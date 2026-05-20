//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_TYPE_H
#define PG_HEXRETRO_PG_TYPE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pg_varatt.h"
#include "pg_extra.h"
#include "pg_c.h"
#include "pg_integer.h"
#include "pg_time.h"
#include "pg_string.h"
#include "pg_default.h"
#include "pg_scan.h"
#include "pg_core_types.h"
#include "pg_detoast.h"

#define USE_FLOAT8_BYVAL true
typedef struct TypeOps {
    Datum (*decode)(char *data);
} TypeOps;



void register_type(Oid oid, TypeHandler *handler);
TypeHandler* get_type_handler(Oid oid);
Datum decode_value(char *data, ColumnInfo *col);
void init_type_system();

char *datum_to_string(ScanContext *ctx, Datum d, Oid typid);

#endif //PG_HEXRETRO_PG_TYPE_H
