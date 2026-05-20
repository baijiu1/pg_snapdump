//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_LZCOMPRESS_H
#define PG_HEXRETRO_PG_LZCOMPRESS_H
#include "pg_c.h"

int32
pglz_decompress(const char *source, int32 slen, char *dest,
                int32 rawsize, bool check_complete);

#endif //PG_HEXRETRO_PG_LZCOMPRESS_H
