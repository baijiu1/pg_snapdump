//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_PAGE_H
#define PG_HEXRETRO_PG_PAGE_H

#include "pg_core_types.h"
#include "pg_scan.h"
#include "pg_heap.h"
#include "pg_data.h"
#include "pg_detoast.h"
#include "pg_varatt.h"
#include "pg_c.h"
#include "pg_output.h"
#include "pg_wal.h"
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>


#define PG

#ifdef PG
#define HeapPageHeaderSize 24
#elif OPENGAUSS
#define HeapPageHeaderSize 40
#endif


//const long osPagesize = sysconf(_SC_PAGE_SIZE);

#ifdef __APPLE__
#define LOGICAL_PAGE_SIZE (osPagesize / 2)
#define _PAGESIZE osPagesize
#elif defined(__linux__)
#define LOGICAL_PAGE_SIZE (osPagesize * 2)
#define _PAGESIZE (osPagesize * 2)
#endif




#if defined(HAVE__BUILTIN_BSWAP16)

#define pg_bswap16(x) __builtin_bswap16(x)

#elif defined(_MSC_VER)

#define pg_bswap16(x) _byteswap_ushort(x)

#else

static uint16_t pg_bswap16(uint16_t x)
{
    return
            ((x << 8) & 0xff00) |
            ((x >> 8) & 0x00ff);
}

#endif							/* HAVE__BUILTIN_BSWAP16 */


/* implementation of uint32 pg_bswap32(uint32) */
#if defined(HAVE__BUILTIN_BSWAP32)

#define pg_bswap32(x) __builtin_bswap32(x)

#elif defined(_MSC_VER)

#define pg_bswap32(x) _byteswap_ulong(x)

#else

static inline uint32_t pg_bswap32(uint32_t x)
{
    return
            ((x << 24) & 0xff000000) |
            ((x << 8) & 0x00ff0000) |
            ((x >> 8) & 0x0000ff00) |
            ((x >> 24) & 0x000000ff);
}

#endif							/* HAVE__BUILTIN_BSWAP32 */


/* implementation of uint64 pg_bswap64(uint64) */
#if defined(HAVE__BUILTIN_BSWAP64)

#define pg_bswap64(x) __builtin_bswap64(x)


#elif defined(_MSC_VER)

#define pg_bswap64(x) _byteswap_uint64(x)

#else
#ifndef HAVE_UINT64
typedef unsigned long int uint64;
#endif
#define INT64CONST(x)  (x##L)
#define UINT64CONST(x) (x##UL)

static uint64_t pg_bswap64(uint64_t x)
{
    return
            ((x << 56) & UINT64CONST(0xff00000000000000)) |
            ((x << 40) & UINT64CONST(0x00ff000000000000)) |
            ((x << 24) & UINT64CONST(0x0000ff0000000000)) |
            ((x << 8) & UINT64CONST(0x000000ff00000000)) |
            ((x >> 8) & UINT64CONST(0x00000000ff000000)) |
            ((x >> 24) & UINT64CONST(0x0000000000ff0000)) |
            ((x >> 40) & UINT64CONST(0x000000000000ff00)) |
            ((x >> 56) & UINT64CONST(0x00000000000000ff));
}
#endif							/* HAVE__BUILTIN_BSWAP64 */


#ifdef WORDS_BIGENDIAN

#define pg_hton16(x)		(x)
#define pg_hton32(x)		(x)
#define pg_hton64(x)		(x)

#define pg_ntoh16(x)		(x)
#define pg_ntoh32(x)		(x)
#define pg_ntoh64(x)		(x)

#else

#define pg_hton16(x)		pg_bswap16(x)
#define pg_hton32(x)		pg_bswap32(x)
#define pg_hton64(x)		pg_bswap64(x)

#define pg_ntoh16(x)		pg_bswap16(x)
#define pg_ntoh32(x)		pg_bswap32(x)
#define pg_ntoh64(x)		pg_bswap64(x)

#endif							/* WORDS_BIGENDIAN */


typedef struct {
    uint32_t rel_oid;
    uint32_t node_id;
} RelFileNode;

typedef RelFileNode *RelFileNodeData;


extern struct ScanContext;
void scan_page(ScanContext *ctx);
bool extract_tuple(ScanContext *ctx, ItemIdData *item);
void reset_slot(ScanContext *ctx, tupleSlot *slot);
bool exec_filter(tupleSlot *slot);
bool exec_pg_class_filter(ScanContext *ctx, tupleSlot *slot);
bool exec_pg_attribute_filter(ScanContext *ctx, tupleSlot *slot);
bool exec_user_table_filter(ScanContext *ctx, tupleSlot *slot);
int map_pg_class(ScanContext *ctx, tupleSlot *slot);
ColumnInfo map_pg_attribute(tupleSlot *slot);
void map_user_table(ScanContext *ctx, tupleSlot *slot);
//Datum slot_getattr(tupleSlot *slot, int attnum);
void schema_builder_add(SchemaBuilder *b, ColumnInfo col);
Schema *schema_builder_finalize(SchemaBuilder *b);
void print_tuple(tupleSlot *slot);
bool exec_pg_index_filter(ScanContext *ctx, tupleSlot *slot);
int map_pg_index(ScanContext *ctx, tupleSlot *slot);
toastChunk extract_toast_chunk(tupleSlot *slot);

void scan_wal_page(ScanContext *ctx);
#endif //PG_HEXRETRO_PG_PAGE_H
