//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_DETOAST_H
#define PG_HEXRETRO_PG_DETOAST_H

#include <assert.h>
#include "pg_core_types.h"
#include "pg_scan.h"
#include "pg_varatt.h"
#include "pg_c.h"
#include "pg_lzcompress.h"
#include "pg_toast.h"
#include "pg_relation.h"


#define NO_LZ4_SUPPORT() printf("compression method lz4 not supported")
struct toastChunkList;


toastChunkList* toast_cache_get_list(toastCache *cache, Oid chunk_id);
char* toast_assemble(toastChunkList *list, int *out_len);
struct varlena *pg_detoast_datum_packed(ScanContext *ctx, struct varlena *datum);

struct varlena *
detoast_attr(ScanContext *ctx, struct varlena *attr);

static struct varlena *
toast_decompress_datum(struct varlena *attr);
struct varlena *
pglz_decompress_datum(const struct varlena *value);
struct varlena *
lz4_decompress_datum(const struct varlena *value);


static struct varlena *
toast_fetch_datum(ScanContext *ctx, struct varlena *attr);

extern void scan_page(ScanContext *ctx);
char *fetch_toast_value(ScanContext *ctx,
                        Oid toast_rel_oid,
                        Oid chunk_id,
                        int32 rawsize);
#endif //PG_HEXRETRO_PG_DETOAST_H
