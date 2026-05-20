//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_DATA_H
#define PG_HEXRETRO_PG_DATA_H
#include "pg_c.h"
#include "pg_heap.h"
#include "pg_core_types.h"
#include "pg_scan.h"
#include "pg_type.h"
#include "pg_bst.h"
#include "pg_varatt.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "pg_xact.h"


bool decode_tuple(tupleSlot *slot, char *tuple, uint16_t len, Visibility_ctx*);
void deform_tuple(tupleSlot *slot);

Datum slot_getattr(tupleSlot *slot, int attnum);
bool is_visible(tupleSlot *slot, int, int);

#endif //PG_HEXRETRO_PG_DATA_H
