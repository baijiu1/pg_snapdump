//
// Created by 白杰 on 2026/5/19.
//

#include "pg_access.h"

void build_base_path(Relocator *r, const char *basic_path) {
    if (r->db_oid == 0) {
        memcpy(r->base_path, basic_path, strlen(basic_path));
    } else {
        snprintf(r->base_path, sizeof(r->base_path), "%s/%s/%u", r->base_path, "base", r->db_oid);
    }
}

int open_file(const char *file_path, FileHandle* f) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        LOG_ERROR("open file %s error.", file_path);
        exit(1);
    }
    off_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize < 0)
    {
        LOG_ERROR("file size: %d abnormal", filesize);
        close(fd);
        exit(1);
    } else if (filesize == 1) {
        return 1;
    }
    f->fd = fd;
    f->file_name = (char*)file_path;
    f->file_size = filesize;
    return fd;
};

DIR * open_dir(const char *dir_path, FileHandle* f) {
    DIR *dir;
    dir = opendir(dir_path);
    if (dir == NULL) {
        LOG_ERROR("open directory %s failed: %s",
                  dir_path,
                  strerror(errno));
        return NULL;
    }
    return dir;
}

int close_file(FileHandle* f) {
    return close(f->fd);
};

int mmap_file(FileHandle* f) {
    if (f->file_size == 0) {
        return 1;
    }
    f->addr = mmap(NULL, f->file_size, PROT_READ, MAP_PRIVATE, f->fd, 0);
    if (f->addr == MAP_FAILED) {
        LOG_ERROR("mmap file %s failed. exit.", f->file_name);
        close(f->fd);
        exit(1);
    }
    return true;
};

char* get_page(FileHandle* f, int page_no) {
    size_t offset = (size_t)page_no * PAGE_SIZE;

//    if (offset + _PAGESIZE > f->file_size) {
//        return NULL;
//    }

    size_t remain = f->file_size - offset;
    if (remain < PAGE_SIZE) {
        return (char*)f->addr + offset; // 只返回可用部分
    }

    return (char*)f->addr + offset;
};

// access.h
void init_file_access(Engine *e) {
    e->access->open_file(e->relocator->absolute_path, &e->access->file);
    e->access->read_file_size(&e->access->file);
    e->access->mmap_file(&e->access->file);
};

void build_from_path(Engine *e, const char* rel_file_name) {
    memset(e->relocator->absolute_path, 0, BASE_PATH_LEN);
    snprintf(e->relocator->absolute_path, sizeof(e->relocator->absolute_path), "%s%s", e->relocator->base_path, "/1259");
    if (!file_exists(e->relocator->absolute_path)) {
        LOG_WARN("can not find pg_class data file, will set relfilenode and oid 0");
        e->relocator->pg_class_rel_file_node = 0;
        e->relocator->pg_class_oid = 0;
        snprintf(e->relocator->absolute_path, sizeof(e->relocator->absolute_path), "%s%s", e->relocator->base_path, "/1249");
        if (!file_exists(e->relocator->absolute_path)) {
            LOG_WARN("can not find pg_attribute data file, will set relfilenode and oid 0");
            e->relocator->pg_attribute_rel_file_node = 0;
            e->relocator->pg_attribute_oid = 0;
            return;
        }
    }
    snprintf(e->relocator->absolute_path, sizeof(e->relocator->absolute_path), "%s%s", e->relocator->base_path, "/pg_filenode.map");
    LOG_DEBUG("find pg_filenode.map from %s", e->relocator->absolute_path);
    init_file_access(e);
    char* addr = e->access->get_page(&e->access->file, 0);

    RelFileNodeData file_node_map = (RelFileNodeData) (addr);
    for (int i = 0; i < 60; ++i) {
        if (file_node_map[i].rel_oid == 1259) {
            LOG_DEBUG("find pg_class info");
            e->relocator->pg_class_rel_file_node = file_node_map[i].node_id;
            e->relocator->pg_class_oid = file_node_map[i].rel_oid;
        } else if (file_node_map[i].rel_oid == 1249) {
            LOG_DEBUG("find pg_attribute info");
            e->relocator->pg_attribute_rel_file_node = file_node_map[i].node_id;
            e->relocator->pg_attribute_oid = file_node_map[i].rel_oid;
        }
    }
    munmap(e->access->file.addr, e->access->file.file_size);
    e->access->close_file(&e->access->file);
};

Relocator* init_relocator() {
    Relocator* r = (Relocator*) malloc(sizeof(Relocator));
    if (!r) return NULL;
    memset(r, 0, sizeof(Relocator));
    memset(r->base_path, 0, BASE_PATH_LEN);
    memset(r->absolute_path, 0, BASE_PATH_LEN);
    memset(r->rel_file_name, 0, BASE_PATH_LEN);
    r->db_oid = 0;
    r->relid = 0;
    r->rel_file_node = 0;
    r->pg_class_oid = 0;
    r->pg_class_rel_file_node = 0;
    r->pg_attribute_oid = 0;
    r->pg_attribute_rel_file_node = 0;
    r->pg_type_oid = 0;
    r->pg_type_rel_file_node = 0;
    r->file_size = 0;
    r->file_page_count = 0;
    r->build_base_path = build_base_path;
    r->build_from_path = build_from_path;
    return r;
};

off_t read_file_size(FileHandle* f){
    off_t fileSize = lseek(f->fd, 0, SEEK_END);
    if (fileSize == 8192) {
        f->file_page_count = 1;
    } else {
        f->file_page_count = fileSize / PAGE_SIZE;
    }
    f->file_size = fileSize;
};

Access* init_access() {
    Access *a = (Access *)malloc(sizeof(Access));
    if (a == NULL)
        return NULL;

    memset(a, 0, sizeof(Access));

    // 初始化函数指针（核心）
    a->open_file = open_file;
    a->open_dir = open_dir;
    a->read_file_size = read_file_size;
    a->mmap_file = mmap_file;
    a->get_page = get_page;
    a->close_file = close_file;

    return a;
};

Schema* build_user_schema(ScanContext* ctx) {
    Schema *s = (Schema*) malloc(sizeof(Schema));
    memset(s, 0, sizeof(Schema));

    s->natts = ctx->schema->natts;

    s->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * s->natts);
    memset(s->cols, 0, sizeof(ColumnInfo) * s->natts);


    return s;
};

Schema* build_pg_attribute_schema() {
    Schema *s = (Schema*) malloc(sizeof(Schema));
    memset(s, 0, sizeof(Schema));

    s->natts = 26;

    s->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * s->natts);
    memset(s->cols, 0, sizeof(ColumnInfo) * s->natts);

    s->cols[0]  = (ColumnInfo){ "attrelid",       26, 4,  true,  'i',  1 };
    s->cols[1]  = (ColumnInfo){ "attname",        19, 64, false, 'i',  2 };
    s->cols[2]  = (ColumnInfo){ "atttypid",       26, 4,  true,  'i',  3 };
    s->cols[3]  = (ColumnInfo){ "attlen",         21, 2,  true,  's',  4 };
    s->cols[4]  = (ColumnInfo){ "attnum",         21, 2,  true,  's',  5 };
    s->cols[5]  = (ColumnInfo){ "attcacheoff",    23, 4,  true,  'i',  6 };
    s->cols[6]  = (ColumnInfo){ "atttypmod",      23, 4,  true,  'i',  7 };
    s->cols[7]  = (ColumnInfo){ "attndims",       21, 2,  true,  's',  8 };
    s->cols[8]  = (ColumnInfo){ "attbyval",       16, 1,  true,  'c',  9 };
    s->cols[9]  = (ColumnInfo){ "attalign",       18, 1,  true,  'c', 10 };
    s->cols[10] = (ColumnInfo){ "attstorage",     18, 1,  true,  'c', 11 };
    s->cols[11] = (ColumnInfo){ "attcompression", 18, 1,  true,  'c', 12 };
    s->cols[12] = (ColumnInfo){ "attnotnull",     16, 1,  true,  'c', 13 };
    s->cols[13] = (ColumnInfo){ "atthasdef",      16, 1,  true,  'c', 14 };
    s->cols[14] = (ColumnInfo){ "atthasmissing",  16, 1,  true,  'c', 15 };
    s->cols[15] = (ColumnInfo){ "attidentity",    18, 1,  true,  'c', 16 };
    s->cols[16] = (ColumnInfo){ "attgenerated",   18, 1,  true,  'c', 17 };
    s->cols[17] = (ColumnInfo){ "attisdropped",   16, 1,  true,  'c', 18 };
    s->cols[18] = (ColumnInfo){ "attislocal",     16, 1,  true,  'c', 19 };
    s->cols[19] = (ColumnInfo){ "attinhcount",    21, 2,  true,  's', 20 };
    s->cols[20] = (ColumnInfo){ "attcollation",   26, 4,  true,  'i', 21 };
    s->cols[21] = (ColumnInfo){ "attstattarget",  21, 2,  true,  's', 22 };
    s->cols[22] = (ColumnInfo){ "attacl",         1034, -1, false,'i', 23 };
    s->cols[23] = (ColumnInfo){ "attoptions",     1009, -1, false,'i', 24 };
    s->cols[24] = (ColumnInfo){ "attfdwoptions",  1009, -1, false,'i', 25 };
    s->cols[25] = (ColumnInfo){ "attmissingval",  2277, -1, false,'i', 26 };

    return s;
}

void slot_bind_schema(tupleSlot *slot, Schema *schema)
{
    slot->schema = schema;

    slot->values = (Datum*) malloc(sizeof(Datum) * schema->natts);
    slot->is_null = (bool*) malloc(sizeof(bool) * schema->natts);

    memset(slot->values, 0, sizeof(Datum) * schema->natts);
    memset(slot->is_null, 0, sizeof(bool) * schema->natts);

    slot->nvalid = 0;
}

Schema *build_pg_database_schema() {
    Schema *s = (Schema*) malloc(sizeof(Schema));
    memset(s, 0, sizeof(Schema));

    s->natts = 18;

    s->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * s->natts);
    memset(s->cols, 0, sizeof(ColumnInfo) * s->natts);

    s->cols[0]  = (ColumnInfo){ "oid",            26,   4,  true,  'i', 1 };
    s->cols[1]  = (ColumnInfo){ "datname",        19,  64,  true,  'c', 2 };
    s->cols[2]  = (ColumnInfo){ "datdba",         26,   4,  true,  'i', 3 };
    s->cols[3]  = (ColumnInfo){ "encoding",       23,   4,  true,  'i', 4 };
    s->cols[4]  = (ColumnInfo){ "datlocprovider", 18,   1,  true,  'c', 5 };
    s->cols[5]  = (ColumnInfo){ "datistemplate",  16,   1,  true,  'b', 6 };
    s->cols[6]  = (ColumnInfo){ "datallowconn",   16,   1,  true,  'b', 7 };
    s->cols[7]  = (ColumnInfo){ "dathasloginevt", 16,   1,  true,  'b', 8 };
    s->cols[8]  = (ColumnInfo){ "datconnlimit",   23,   4,  true,  'i', 9 };
    s->cols[9]  = (ColumnInfo){ "datfrozenxid",   28,   4,  true,  'i', 10 };
    s->cols[10] = (ColumnInfo){ "datminmxid",     28,   4,  true,  'i', 11 };
    s->cols[11] = (ColumnInfo){ "dattablespace",  26,   4,  true,  'i', 12 };
    s->cols[12] = (ColumnInfo){ "datcollate",     25,  -1,  true,  'v', 13 };
    s->cols[13] = (ColumnInfo){ "datctype",       25,  -1,  true,  'v', 14 };
    s->cols[14] = (ColumnInfo){ "datlocale",      25,  -1,  false, 'v', 15 };
    s->cols[15] = (ColumnInfo){ "daticurules",    25,  -1,  false, 'v', 16 };
    s->cols[16] = (ColumnInfo){ "datcollversion", 25,  -1,  false, 'v', 17 };
    s->cols[17] = (ColumnInfo){ "datacl",       1034, -1,  false, 'v', 18 };

    return s;
}

Schema* build_pg_class_schema() {
    Schema *s = (Schema*) malloc(sizeof(Schema));
    memset(s, 0, sizeof(Schema));

    s->natts = 33;

    s->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * s->natts);
    memset(s->cols, 0, sizeof(ColumnInfo) * s->natts);

    s->cols[0]  = (ColumnInfo){ "oid",            26, 4,  true,  'i', 1 };
    s->cols[1]  = (ColumnInfo){ "relname",        19, 64, false, 'c', 2 };
    s->cols[2]  = (ColumnInfo){ "relnamespace",   26, 4,  true,  'i', 3 };
    s->cols[3]  = (ColumnInfo){ "reltype",        26, 4,  true,  'i', 4 };
    s->cols[4]  = (ColumnInfo){ "reloftype",      26, 4,  true,  'i', 5 };
    s->cols[5]  = (ColumnInfo){ "relowner",       26, 4,  true,  'i', 6 };
    s->cols[6]  = (ColumnInfo){ "relam",          26, 4,  true,  'i', 7 };
    s->cols[7]  = (ColumnInfo){ "relfilenode",    26, 4,  true,  'i', 8 };
    s->cols[8]  = (ColumnInfo){ "reltablespace",  26, 4,  true,  'i', 9 };
    s->cols[9]  = (ColumnInfo){ "relpages",       23, 4,  true,  'i', 10 };
    s->cols[10] = (ColumnInfo){ "reltuples",      700,4,  true,  'i', 11 };   // float4
    s->cols[11] = (ColumnInfo){ "relallvisible",  23, 4,  true,  'i', 12 };
    s->cols[12] = (ColumnInfo){ "reltoastrelid",  26, 4,  true,  'i', 13 };
    s->cols[13] = (ColumnInfo){ "relhasindex",    16, 1,  true,  'c', 14 };
    s->cols[14] = (ColumnInfo){ "relisshared",    16, 1,  true,  'c', 15 };
    s->cols[15] = (ColumnInfo){ "relpersistence", 18, 1,  true,  'c', 16 };
    s->cols[16] = (ColumnInfo){ "relkind",        18, 1,  true,  'c', 17 };
    s->cols[17] = (ColumnInfo){ "relnatts",       21, 2,  true,  's', 18 };   // int2
    s->cols[18] = (ColumnInfo){ "relchecks",      21, 2,  true,  's', 19 };
    s->cols[19] = (ColumnInfo){ "relhasrules",    16, 1,  true,  'c', 20 };
    s->cols[20] = (ColumnInfo){ "relhastriggers", 16, 1,  true,  'c', 21 };
    s->cols[21] = (ColumnInfo){ "relhassubclass", 16, 1,  true,  'c', 22 };
    s->cols[22] = (ColumnInfo){ "relrowsecurity", 16, 1,  true,  'c', 23 };
    s->cols[23] = (ColumnInfo){ "relforcerowsecurity", 16, 1, true, 'c', 24 };
    s->cols[24] = (ColumnInfo){ "relispopulated", 16, 1,  true,  'c', 25 };
    s->cols[25] = (ColumnInfo){ "relreplident",   18, 1,  true,  'c', 26 };
    s->cols[26] = (ColumnInfo){ "relispartition", 16, 1,  true,  'c', 27 };
    s->cols[27] = (ColumnInfo){ "relrewrite",     26, 4,  true,  'i', 28 };
    s->cols[28] = (ColumnInfo){ "relfrozenxid",   28, 4,  true,  'i', 29 };
    s->cols[29] = (ColumnInfo){ "relminmxid",     28, 4,  true,  'i', 30 };

    // nullable varlena
    s->cols[30] = (ColumnInfo){ "relacl",         1034, -1, false, 'i', 31 };
    s->cols[31] = (ColumnInfo){ "reloptions",     1009, -1, false, 'i', 32 };
    s->cols[32] = (ColumnInfo){ "relpartbound",   194,  -1, false, 'i', 33 };

    return s;
}


Engine* init_engine()
{
    Engine *e = (Engine*) malloc(sizeof(Engine));

    e->access = init_access();        // ✔ 必须返回指针
    e->relocator = init_relocator();  // ✔

    return e;
}

void init_slot_arena(tupleSlot *slot, size_t size)
{
    slot->arena.base = (char*) malloc(size);
    slot->arena.cur  = slot->arena.base;
    slot->arena.end  = slot->arena.base + size;
    slot->arena.size = size;
}

void init_slot_mvcc(tupleSlot *slot, size_t size) {
    slot->mvcc.ctid.ip_blkid.bi_hi = 0;
    slot->mvcc.ctid.ip_blkid.bi_lo = 0;
    slot->mvcc.ctid.ip_posid = 0;
    slot->mvcc.infomask = 0;
    slot->mvcc.infomask2 = 0;
    slot->mvcc.xmax = 0;
    slot->mvcc.xmin = 0;
}

SchemaBuilder * schema_builder_init()
{
    SchemaBuilder *b = (SchemaBuilder*) malloc(sizeof(SchemaBuilder));
    b->cap = 0;
    b->cols = NULL;
    b->ncols = 0;
    return b;
}

tupleSlot* make_slot() {
    tupleSlot *slot = (tupleSlot*) malloc(sizeof(tupleSlot));
    if (!slot) {
        perror("malloc slot failed");
        exit(1);
    }
    init_slot_arena(slot, 16 * 1024);
    init_slot_mvcc(slot, 1024);

    memset(slot, 0, sizeof(tupleSlot));

    // 注意：这里不要分配 values / is_null
    // 因为你还不知道 natts（schema 还没确定）

    slot->schema = NULL;
    slot->values = NULL;
    slot->is_null = NULL;
    slot->nvalid = 0;

    return slot;
}


int load_table(ScanContext* ctx) {
    off_t groupSize = 1000;
    bool has_segment;
    char *files[BASE_PATH_LEN];
    ctx->engine->access->open_file(ctx->engine->relocator->absolute_path, &ctx->engine->access->file);
    ctx->engine->access->read_file_size(&ctx->engine->access->file);
    ctx->engine->access->mmap_file(&ctx->engine->access->file);
    if (ctx->phase == PHASE_USER_TABLE) {
        char prefix[32];
        snprintf(prefix, sizeof(prefix), "%d", ctx->relfilenode);
        struct dirent *entry;
        int count = 0;
        DIR* dir = ctx->engine->access->open_dir(ctx->engine->relocator->base_path, &ctx->engine->access->file);

        if (dir == NULL)
        {
            return false;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
                char *suffix = entry->d_name + strlen(prefix);

                if (*suffix == '.')
                {
                    has_segment = true;
                    files[count++] = strdup(entry->d_name);
                    count++;
                }

                if (count >= BASE_PATH_LEN)
                    break;
            }
        }
        closedir(dir);
    }

    if (has_segment) {
        int arr_len = sizeof(files) / sizeof(files[0]);
//        int groupCount = (ctx->engine->access->file.file_page_count + groupSize - 1) / groupSize;
        for (int g = 0; g < arr_len; g++) {
            snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%s", ctx->engine->relocator->base_path, files[g]);
            init_file_access(ctx->engine);

            int start = g * groupSize;

            int end = (g + 1) * groupSize - 1;

            if (end >= ctx->engine->access->file.file_page_count) {
                end = ctx->engine->access->file.file_page_count - 1;  // 防止超过总页数
            }

            // 这里可以开并发任务处理 [start, end] 区间
            LOG_DEBUG("find %u page", ctx->engine->access->file.file_page_count);
            int max_pages = (ctx->engine->access->file.file_size + PAGE_SIZE - 1) / PAGE_SIZE;

            ctx->end = false;
            for (int i = 0; i < max_pages; i++)
            {
                if (i + 1 < max_pages)
                {
                    char *next_page =
                            (char *)ctx->engine->access->file.addr + (size_t)(i + 1) * PAGE_SIZE;

                    __builtin_prefetch(next_page, 0, 3);
                }

                ctx->current_page_no = i;

                char *page =
                        (char *)ctx->engine->access->file.addr + (size_t)i * PAGE_SIZE;

                ctx->page_buf = page;
                if (ctx->phase == PHASE_WAL) {
//                scan_wal_page(ctx);
                } else {
                    scan_page(ctx);
                }

            }
            ctx->end = true;
            ctx->engine->access->close_file(&ctx->engine->access->file);
        }
    } else {
        LOG_DEBUG("find %u page", ctx->engine->access->file.file_page_count);
        int max_pages = (ctx->engine->access->file.file_size + PAGE_SIZE - 1) / PAGE_SIZE;

        ctx->end = false;
        for (int i = 0; i < max_pages; i++)
        {
            if (i + 1 < max_pages)
            {
                char *next_page =
                        (char *)ctx->engine->access->file.addr + (size_t)(i + 1) * PAGE_SIZE;

                __builtin_prefetch(next_page, 0, 3);
            }

            ctx->current_page_no = i;

            char *page =
                    (char *)ctx->engine->access->file.addr + (size_t)i * PAGE_SIZE;

            ctx->page_buf = page;
            if (ctx->phase == PHASE_WAL) {
//                scan_wal_page(ctx);
            } else {
                scan_page(ctx);
            }

        }
        ctx->engine->access->close_file(&ctx->engine->access->file);
    }
};

toastCache* toast_cache_init() {
    toastCache *cache = (toastCache*) malloc(sizeof(toastCache));
    cache->count = 0;
    cache->capacity = 16;
    cache->lists = (toastChunkList*) malloc(sizeof(toastChunkList) * cache->capacity);
    return cache;
}

void WALOpenSegmentInit(WALOpenSegment *seg, WALSegmentContext *segcxt, int segsize, const char *waldir) {
    seg->ws_file = -1;
    seg->ws_segno = 0;
    seg->ws_tli = 0;

    segcxt->ws_segsize = segsize;
    if (waldir)
        snprintf(segcxt->ws_dir, MAXPGPATH, "%s", waldir);
}

XLogReaderState* xlog_reader_state_init(Engine * e, int wal_segment_size) {
    XLogReaderState *state;

    state = (XLogReaderState *) malloc(sizeof(XLogReaderState));

    if (!state)
        return NULL;

    state->routine = e->access ;

    state->readBuf = (char *) malloc(XLOG_BLCKSZ);
    if (!state->readBuf)
    {
        free(state);
        return NULL;
    }

    /* Initialize segment info. */
    WALOpenSegmentInit(&state->seg, &state->segcxt, wal_segment_size,
                       e->relocator->pg_wal_path);

//    /* system_identifier initialized to zeroes above */
//    state->private_data = private_data;
//    /* ReadRecPtr, EndRecPtr and readLen initialized to zeroes above */
//    state->errormsg_buf = palloc_extended(MAX_ERRORMSG_LEN + 1,
//                                          MCXT_ALLOC_NO_OOM);
    if (!state->errormsg_buf)
    {
        free(state->readBuf);
        free(state);
        return NULL;
    }
    state->errormsg_buf[0] = '\0';

    /*
     * Allocate an initial readRecordBuf of minimal size, which can later be
     * enlarged if necessary.
     */
//    allocate_recordbuf(state, 0);
    return state;
}

int dir_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1; // 存在且是目录
    }
    return 0; // 不存在或不是目录
}

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

Visibility_ctx* init_visibility_mode(ScanContext *ctx, char* pg_data_base_path) {
    xact_ctl *xact_ctl = NULL;
    assert(ctx && ctx->engine && ctx->engine->relocator);
    snprintf(ctx->engine->relocator->pg_xact_path, sizeof(ctx->engine->relocator->pg_xact_path), "%s/pg_xact",
             pg_data_base_path);

    if (dir_exists(ctx->engine->relocator->pg_xact_path)) {
        ctx->visible_mode = VISIBILITY_FULL;
        xact_ctl = xact_cache_create(1024);
        if (xact_ctl == NULL)
        {
            perror("create cache failed\n");
            return NULL;
        }
    } else {
        ctx->visible_mode = VISIBILITY_HINT_ONLY;
    }

    Visibility_ctx* vctx = (Visibility_ctx*) malloc(sizeof(Visibility_ctx));

    *vctx = (Visibility_ctx){
            .only_visible = ctx->op->only_visible,
            .visible_mode = ctx->visible_mode,
            .xmin_snapshot = 0,
            .xmax_snapshot = 0,
            .pg_xact_base_path = ctx->engine->relocator->pg_xact_path,
            .e = ctx->engine,
            .phase = ctx->phase,
            .xact_ctl = xact_ctl,
            .init_file_access = init_file_access
    };
    return vctx;
}

Schema *build_pg_toast_schema() {
    Schema *s = (Schema *) malloc(sizeof(Schema));

    s->natts = 3;
    s->cols = (ColumnInfo *) malloc(sizeof(ColumnInfo) * 3);
    s->cols[0]  = (ColumnInfo){ "chunk_id",            OIDOID, 4,  true,  'i', 1 };
    s->cols[1]  = (ColumnInfo){ "chunk_seq",            INT4OID, 4,  true,  'i', 1 };
    s->cols[2]  = (ColumnInfo){ "chunk_data",            BYTEAOID, -1,  true,  'i', 1 };
    return s;
}

int pg_forge(DumpOptions *op, char flag) {
    char *pg_data_base_path = NULL;

    if (op->db_path != NULL)
    {
        pg_data_base_path = resolve_pgdata(op->db_path);
    }
    else
    {
        pg_data_base_path = getenv("PGDATA");

        if (!pg_data_base_path)
        {
            LOG_ERROR("Error: no database path specified.\n" "Use -D or set PGDATA\n");
            exit(1);
        }
    }


    Engine *engine = init_engine();
    ScanContext *ctx = (ScanContext *) malloc(sizeof(ScanContext));
    memset(ctx, 0, sizeof(ScanContext));
//    xlog_reader_state_init(engine->access);
    ctx->op = op;
    ctx->fp = NULL;
    ctx->header_written = 0;
    ctx->pg_type_relid = 0;
    if (op->output_name == 1) {
        char* output_name;
        if (ctx->relname != NULL) {
            output_name = ctx->relname;
        } else {
            output_name = strdup("output.csv");
        }

        ctx->fp = fopen(output_name, "w");
        if (!ctx->fp) {
            LOG_ERROR("open %s error", op->output_name);
            exit(1);
        }
        setvbuf(ctx->fp, NULL, _IOFBF, 1 << 20); // 1MB buffer
    }
    ctx->engine = engine;
    ctx->end = false;
    LOG_DEBUG("init schema builder");
    SchemaBuilder * b = schema_builder_init();
    ctx->schema_builder = b;
    LOG_DEBUG("init slot");
    ctx->database_slot = make_slot();
    ctx->attr_slot = make_slot();
    ctx->class_slot = make_slot();
    ctx->user_slot = make_slot();
    LOG_DEBUG("init slot end");
    LOG_DEBUG("init toast cache list");
    toastCache *cache = toast_cache_init();
    ctx->toast_cache = cache;
    LOG_DEBUG("parser pg_database");
    ctx->phase = PHASE_PG_DATABASE;
    ctx->slot = ctx->database_slot;
    Schema *sys_schema = build_pg_database_schema();
    slot_bind_schema(ctx->database_slot, sys_schema);
    engine->relocator->build_base_path(engine->relocator, pg_data_base_path);
    Visibility_ctx* vctx = init_visibility_mode(ctx, pg_data_base_path);
    ctx->vsible_ctx = vctx;

    snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%s", ctx->engine->relocator->base_path, "global/1262");
    if (file_exists(ctx->engine->relocator->absolute_path)) {
        LOG_DEBUG("parser pg_database data file: %s", ctx->engine->relocator->absolute_path);
        load_table(ctx);
        ctx->phase = PHASE_PG_DATABASE_DONE;
        engine->relocator->db_oid = ctx->db_oid;
        engine->relocator->build_base_path(engine->relocator, pg_data_base_path);
        LOG_DEBUG("from pg_filenode.map to find pg_class and pg_attribute oid");
        engine->relocator->build_from_path(engine, "/pg_filenode.map");
    } else {
        LOG_WARN("can not find pg_database data file, foreach $PGDATA directory to parser pg_filenode.map");
        DIR *dir;
        struct dirent *entry;

        dir = opendir(pg_data_base_path);

        if (!dir) {
            LOG_ERROR("open directory error, %s not exists, exit...", pg_data_base_path);
            exit(1);
        }

        while ((entry = readdir(dir)) != NULL) {

            struct stat st;

            if (stat(entry->d_name, &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode)) {
                printf("[DIR ] %s\n", entry->d_name);
                if (strcmp(entry->d_name, "1") == 0 || strcmp(entry->d_name, "4") == 0 || strcmp(entry->d_name, "5") == 0 ) {
                    continue;
                } else {
                    char *endptr;

                    long db_oid = strtol(entry->d_name, &endptr, 10);

                    if (*endptr != '\0') {
                        printf("存在非法字符: %s\n", endptr);
                    }
                    LOG_WARN("enter %u directory", db_oid);
                    engine->relocator->db_oid = db_oid;
                    engine->relocator->build_from_path(engine, "/pg_filenode.map");
                    goto entry;
                }
            }
            else if (S_ISREG(st.st_mode)) {
                printf("[FILE] %s\n", entry->d_name);
            }
        }

        closedir(dir);

    }


    entry:
    if (flag == 'd') {
        LOG_DEBUG("dump module");
        int has_file = (op->file != NULL);
        int has_db_path = (op->db_path != NULL && strlen(op->db_path) > 0);
        int has_db   = (op->database != NULL && strlen(op->database) > 0);
        int has_tbl  = (op->table != NULL);
        int has_table_construct  = (op->table_construct != NULL);
        int has_none_info = (op->none_info != NULL);
        // ===== 情况1：使用 -f =====
        if (has_file)
        {
            if (!has_db_path) {
                fprintf(stderr,
                        "Error: -f must be used with -D to specify database path.\n");
                exit(1);
            }
            else if (has_tbl)
            {
                fprintf(stderr,
                        "Error: -f cannot be used with -t.\n");
                exit(1);
            } else if (!has_db) {
                fprintf(stderr,
                        "Error: -f must be used with -d to specify database.\n");
                exit(1);
            }

            engine->relocator->rel_file_node = (Oid)strtoul(op->file, NULL, 10);
            LOG_DEBUG("database directory: %s ", engine->relocator->base_path);
            ctx->slot = ctx->class_slot;
            ctx->phase = PHASE_PG_XACT;

            ctx->phase = PHASE_PG_XACT_DONE;

            LOG_DEBUG("build pg_class schema");
            Schema *sys_schema = build_pg_class_schema();
            slot_bind_schema(ctx->class_slot, sys_schema);
            LOG_DEBUG("pg_class parser begin");
            ctx->phase = PHASE_PG_CLASS;
            if (ctx->engine->relocator->pg_class_rel_file_node == 0) {
                LOG_WARN("pg_class data file noe exists, skip");
            } else {
                snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->engine->relocator->pg_class_rel_file_node);
                LOG_DEBUG("parser pg_class data file: %s ", ctx->engine->relocator->absolute_path);
                load_table(ctx);
                ctx->phase = PHASE_PG_CLASS_DONE;
                LOG_DEBUG("find toast table");
                if (ctx->toast_relid != 0) {
                    ctx->phase = PHASE_PG_TOAST;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_TOAST_DONE;
                    ctx->phase = PHASE_PG_INDEX;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_INDEX_DONE;
                }
            }


            ctx->slot = ctx->attr_slot;
            LOG_DEBUG("build pg_class schema");
            Schema *attr_schema = build_pg_attribute_schema();
            slot_bind_schema(ctx->attr_slot, attr_schema);
            LOG_DEBUG("pg_attribute parser begin");
            ctx->phase = PHASE_PG_ATTRIBUTE;
            if (ctx->engine->relocator->pg_attribute_rel_file_node == 0) {
                LOG_WARN("pg_attribute data file noe exists, skip");

            } else {
                snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->engine->relocator->pg_attribute_rel_file_node);
                LOG_DEBUG("parser pg_attribute data file: %s ", ctx->engine->relocator->absolute_path);
                load_table(ctx);
                ctx->schema = schema_builder_finalize(ctx->schema_builder);
                ctx->phase = PHASE_PG_ATTRIBUTE_DONE;
                if (ctx->toast_relfilenodeid != 0) {
                    snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->toast_relfilenodeid);
                    LOG_DEBUG("parser toast table data file: %s, build toast list", ctx->engine->relocator->absolute_path);
                    ctx->slot = ctx->attr_slot;   // 复用（或者 user_slot 也行）
                    Schema *toast_schema = build_pg_toast_schema();
                    slot_bind_schema(ctx->attr_slot, toast_schema);
                    ctx->phase = PHASE_PG_TOAST_DECODE;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_TOAST_DECODE_DONE;
                }
            }

            LOG_DEBUG("parser input table: %s", op->table);
            ctx->phase = PHASE_USER_TABLE;
            ctx->slot = ctx->user_slot;
            slot_bind_schema(ctx->user_slot, ctx->schema);
            snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->engine->relocator->rel_file_node);
            LOG_DEBUG("find table data file: %s", ctx->engine->relocator->absolute_path);
            load_table(ctx);
            LOG_DEBUG("parser input table: %s end", op->table);
            ctx->phase = vctx->phase = PHASE_USER_TABLE_DONE;

            ctx->phase = vctx->phase = PHASE_WAL;


            ctx->phase = vctx->phase = PHASE_WAL_DONE;

            ctx->phase = PHASE_DONE;
            ctx->phase = PHASE_DONE;
            free(engine->access);
            free(engine->relocator);
            free(engine);
            free(ctx->toast_cache);
            free(ctx);
        } else if (has_db || has_tbl) {
            if (!(has_db && has_tbl))
            {
                fprintf(stderr,
                        "Error: both -d and -t are required\n");
                exit(1);
            }

            LOG_DEBUG("database directory: %s ", engine->relocator->base_path);
            ctx->slot = ctx->class_slot;
            ctx->phase = PHASE_PG_XACT;

            ctx->phase = PHASE_PG_XACT_DONE;

            LOG_DEBUG("build pg_class schema");
            Schema *sys_schema = build_pg_class_schema();
            slot_bind_schema(ctx->class_slot, sys_schema);
            LOG_DEBUG("pg_class parser begin");
            ctx->phase = PHASE_PG_CLASS;
            if (ctx->engine->relocator->pg_class_rel_file_node == 0) {
                LOG_WARN("pg_class data file noe exists, skip");
            } else {
                snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->engine->relocator->pg_class_rel_file_node);
                LOG_DEBUG("parser pg_class data file: %s ", ctx->engine->relocator->absolute_path);
                load_table(ctx);
                ctx->phase = PHASE_PG_CLASS_DONE;
                LOG_DEBUG("find toast table");
                if (ctx->toast_relid != 0) {
                    ctx->phase = PHASE_PG_TOAST;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_TOAST_DONE;
                    ctx->phase = PHASE_PG_INDEX;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_INDEX_DONE;
                }
            }
            LOG_DEBUG("pg_class parser end");



            ctx->slot = ctx->attr_slot;
            LOG_DEBUG("build pg_class schema");
            Schema *attr_schema = build_pg_attribute_schema();
            slot_bind_schema(ctx->attr_slot, attr_schema);
            LOG_DEBUG("pg_attribute parser begin");
            ctx->phase = PHASE_PG_ATTRIBUTE;
            if (ctx->engine->relocator->pg_attribute_rel_file_node == 0) {
                LOG_WARN("pg_attribute data file noe exists, skip");

            } else {
                snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->engine->relocator->pg_attribute_rel_file_node);
                LOG_DEBUG("parser pg_attribute data file: %s ", ctx->engine->relocator->absolute_path);
                load_table(ctx);
                ctx->schema = schema_builder_finalize(ctx->schema_builder);
                ctx->phase = PHASE_PG_ATTRIBUTE_DONE;
                // 判断VARATT_EXTERNAL
                if (ctx->toast_relfilenodeid != 0) {
                    snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->toast_relfilenodeid);
                    LOG_DEBUG("parser toast table data file: %s, build toast list", ctx->engine->relocator->absolute_path);
                    ctx->slot = ctx->attr_slot;   // 复用（或者 user_slot 也行）
                    Schema *toast_schema = build_pg_toast_schema();
                    slot_bind_schema(ctx->attr_slot, toast_schema);
                    ctx->phase = PHASE_PG_TOAST_DECODE;
                    load_table(ctx);
                    ctx->phase = PHASE_PG_TOAST_DECODE_DONE;
                }
            }

            LOG_DEBUG("parser input table: %s", op->table);
            ctx->phase = vctx->phase = PHASE_USER_TABLE;
            ctx->slot = ctx->user_slot;
            slot_bind_schema(ctx->user_slot, ctx->schema);
            snprintf(ctx->engine->relocator->absolute_path, sizeof(ctx->engine->relocator->absolute_path), "%s/%u", ctx->engine->relocator->base_path, ctx->relfilenode);
            if (ctx->relfilenode == 0) {
                LOG_ERROR("can not find user table %s data file. exit.", op->table);
                exit(1);
            }
            LOG_DEBUG("find table data file: %s", ctx->engine->relocator->absolute_path);
            load_table(ctx);
            ctx->phase = vctx->phase = PHASE_USER_TABLE_DONE;

            ctx->phase = vctx->phase = PHASE_WAL;


            ctx->phase = vctx->phase = PHASE_WAL_DONE;

            ctx->phase = PHASE_DONE;
            LOG_DEBUG("parser input table: %s end", op->table);
            free(engine->access);
            free(engine->relocator);
            free(engine);
            free(ctx->toast_cache);
            free(ctx);
        } else if (has_table_construct) {
            // 根据表结构来推测，不需要去检查pg_attribute pg_class表
            if (!(has_db && has_db_path && has_table_construct)) {
                LOG_ERROR("Error: parameter -D <database directory path> and -d <database name> and -T <table construct> are required\n");
                print_dump_usage();
                exit(1);
            }


        } else if (has_none_info == 1) {
            // 什么信息都没有，只有数据库名
            if (!(has_db && has_db_path && has_none_info)) {
                LOG_ERROR("Error: parameter -D <database directory path> and -d <database name> and -n are required\n");
                print_dump_usage();
                exit(1);
            }

        } else {
            LOG_ERROR("Error: you must specify either:\n"
                      "  -f <relfilenode>\n"
                      "  OR\n"
                      "  -d <database> -t <table>\n"
                      "  OR\n"
                      "  -T <table construct>\n"
                      "  OR\n"
                      "  -n\n");
            print_dump_usage();
            exit(1);
        }
    }
    if (op->output_name == 1) {
//        if (ctx->fp) {
        fclose(ctx->fp);
        ctx->fp = NULL;
//        }
    }


}