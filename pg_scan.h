//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_SCAN_H
#define PG_HEXRETRO_PG_SCAN_H

#include "pg_c.h"
#include "pg_heap.h"
#include "pg_core_types.h"
#include "pg_usage.h"
#include "pg_wal.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>


#define BASE_PATH_LEN 600
#define SLOT_HAS_TUPLE     0x01
#define SLOT_DECODED       0x02
#define SLOT_TOASTED       0x04


typedef struct tupleMethod {
    void (*init)(struct tupleSlot *slot);

    Datum (*getattr)(struct tupleSlot *slot, int attnum, bool *isnull);

    void (*materialize)(struct tupleSlot *slot);

    void (*clear)(struct tupleSlot *slot);

    void (*destroy)(struct tupleSlot *slot);
} tupleMethod;

typedef struct ColumnInfo {
    char *attname;
    int atttypid;
    int attlen;
    bool attbyval;
    char attalign;
    int attnum;
} ColumnInfo __attribute__((aligned(64)));

typedef struct Schema {
    int natts;
    ColumnInfo *cols;
} Schema;

typedef struct SlotArena {
    char *base;
    char *cur;
    char *end;
    size_t size;
} SlotArena;

typedef struct tupleMVCC {
    TransactionId xmin;
    TransactionId xmax;
    uint16 infomask;
    uint16 infomask2;
    ItemPointerData ctid;
} tupleMVCC;



typedef struct tupleSlot {
    Oid relfilenode;
    Oid relid;
    char *tuple_data;
    int tuple_len;
    int tuple_natts;
    int schema_natts;
    HeapTupleHeaderData * tup;
    Schema* schema;
    tupleMethod* method;
    Datum *values;
    bool *is_null;
    int nvalid;              // 已经解析到第几列（lazy decode）
    SlotArena arena;
    tupleMVCC mvcc;
    uint32 flags;
    void *opaque;
} tupleSlot __attribute__((aligned(64)));

typedef struct FileHandle {
    int fd;
    void *addr;
    size_t file_size;
    size_t file_page_count;
    char* file_name;
} FileHandle;

// 访问层
typedef struct Access {
    FileHandle file;
    int (*open_file)(const char *,FileHandle*);
    DIR * (*open_dir)(const char *,FileHandle*);
    int (*mmap_file)(FileHandle*);
    char* (*get_page)(FileHandle*, int);
    off_t (*read_file_size)(FileHandle*);
    int (*close_file)(FileHandle*);
} Access;

typedef struct Relocator {
    char base_path[BASE_PATH_LEN];
    char datbase_path[BASE_PATH_LEN];

    Oid relid;
    Oid db_oid;
    Oid rel_file_node;
    Oid pg_class_oid;
    Oid pg_class_rel_file_node;
    Oid pg_attribute_oid;
    Oid pg_attribute_rel_file_node;
    Oid pg_type_oid;
    Oid pg_type_rel_file_node;

    off_t file_size;
    off_t file_page_count;

    char rel_file_name[BASE_PATH_LEN];
    char pg_xact_path[BASE_PATH_LEN];
    char pg_wal_path[BASE_PATH_LEN];

    char absolute_path[BASE_PATH_LEN];

    void (*build_from_path)(Engine *, const char*);
    void (*build_base_path)(Relocator *r, const char*);
} Relocator;

typedef struct Engine {
    Access *access;
    Relocator *relocator;
} Engine;

typedef enum {
    PHASE_PG_DATABASE,
    PHASE_PG_DATABASE_DONE,
    PHASE_PG_XACT,
    PHASE_PG_XACT_DONE,
    PHASE_PG_CLASS,
    PHASE_PG_CLASS_DONE,
    PHASE_PG_TOAST,
    PHASE_PG_TOAST_DONE,
    PHASE_PG_TOAST_DECODE,
    PHASE_PG_TOAST_DECODE_DONE,
    PHASE_PG_INDEX,
    PHASE_PG_INDEX_DONE,
    PHASE_PG_ATTRIBUTE,
    PHASE_PG_ATTRIBUTE_DONE,
    PHASE_USER_TABLE,
    PHASE_USER_TABLE_DONE,
    PHASE_WAL,
    PHASE_WAL_DONE,
    PHASE_DETOAST,
    PHASE_DETOAST_DONE,
    PHASE_DONE
} scan_phase;

typedef struct schemaBuilder {
    ColumnInfo *cols;
    int ncols;
    int cap;

    Oid relid;   // 当前正在构建的表
} SchemaBuilder;



typedef struct toastChunk {
    Oid chunk_id;          // valueid（用于分组）
    int32_t seq;           // chunk_seq（用于排序）

    char *data;            // chunk 数据
    int32_t len;           // chunk 数据长度

    // ⭐ 可选增强（强烈建议保留）
    uint32_t raw_size;     // 原始总大小（只在 seq=0 时有意义）
    uint32_t ext_size;     // 外部存储大小（压缩后）

    // ⭐ 调试 / 定位用（可选）
    uint32_t block_no;     // 所在 heap block（方便 debug）
    uint16_t offset;       // tuple offset

} toastChunk __attribute__((aligned(64)));

typedef struct toastChunkList {
    Oid chunk_id;

    toastChunk *chunks;
    int count;
    int capacity;

    int max_seq;   // 用于快速拼接
} toastChunkList;
typedef struct toastCache {
    toastChunkList *lists;
    int count;
    int capacity;
} toastCache;

typedef enum {
    VISIBILITY_FULL,        // 有 pg_xact，可精确判断
    VISIBILITY_HINT_ONLY    // 无 pg_xact，只能靠 infomask
} visibility_mode;

struct xact_ctl;
typedef struct {
    int only_visible;
    int visible_mode;
    TransactionId xmin_snapshot;
    TransactionId xmax_snapshot;
    char* pg_xact_base_path;
    Engine *e;
    scan_phase phase;
    struct xact_ctl *xact_ctl;
    void (*init_file_access)(Engine*);
} Visibility_ctx;

typedef enum {
    long_header,
    first_contrecord,
    first_overwrite_contrecord,
    bkp_removable,
} page_state;

typedef struct {
    XLogReaderState* state;
} pg_wal_ctx;

typedef struct ScanContext {
    Oid db_oid;
    // 主表信息
    Oid relid;
    Oid relfilenode;
    char* relname;

    Oid pg_type_relid;
    Oid pg_type_relfilenode;

    FILE *fp;
    int header_written;

    DumpOptions *op;
    Engine *engine;

    char *page_buf;
    int current_page_no;

    tupleSlot *slot;
    tupleSlot *database_slot;
    tupleSlot *class_slot;
    tupleSlot *attr_slot;
    tupleSlot *user_slot;  // 用户表
    tupleSlot *wal_slot;  // 用户表

    struct schemaBuilder *schema_builder;

    Schema *schema;
    // toast表
    Oid toast_relid;
    Oid toast_relfilenodeid;
    bool has_toast;
    // toast表索引
    Oid toast_index_relid;
    Oid toast_index_relfilenodeid;

    toastCache *toast_cache;

    visibility_mode visible_mode;
    Visibility_ctx *vsible_ctx;

    pg_wal_ctx *wal_ctx;

    scan_phase phase;
    bool end;
} ScanContext __attribute__((aligned(64)));


#endif //PG_HEXRETRO_PG_SCAN_H
