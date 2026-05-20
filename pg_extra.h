//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_EXTRA_H
#define PG_HEXRETRO_PG_EXTRA_H

#include <stdlib.h>
#include <stdio.h>
#include "pg_c.h"

Datum ns_decode_tid(char *ptr);
Datum ns_decode_xid(char *ptr);
Datum ns_decode_cid(char *ptr);
#endif //PG_HEXRETRO_PG_EXTRA_H
