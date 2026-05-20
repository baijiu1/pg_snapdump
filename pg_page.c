//
// Created by 白杰 on 2026/5/19.
//

#include "pg_page.h"

bool exec_pg_class_filter(ScanContext *ctx, tupleSlot *slot)
{
    // 判断
    if (ctx->op->file != NULL) {
        NameData* rel_name = (NameData *) slot_getattr(slot, 1);
        Oid rel_file_node = (Oid) slot_getattr(slot, 7);
        if (rel_file_node == ctx->engine->relocator->rel_file_node && is_visible(slot, -1, -1))
        {
            return true;
        }
//        else if (strcmp(rel_name->data, "pg_type") == 0 && is_visible(slot)) {
//            return true;
//        }
    } else if (ctx->op->table != NULL) {
        NameData* rel_name = (NameData *) slot_getattr(slot, 1);
        if (strcmp(rel_name->data, ctx->op->table) == 0 && is_visible(slot, -1, -1)) {
            return true;
        }
//        else if (strcmp(rel_name->data, "pg_type") == 0 && is_visible(slot)) {
//            return true;
//        }
    }

    return false;
}

bool exec_pg_database_filter(ScanContext *ctx, tupleSlot *slot) {
    NameData* database_name = (NameData*) slot_getattr(slot, 1);
    if (strcmp(ctx->op->database, database_name->data) == 0) {
        return true;
    }
    return false;
}

int map_pg_database(ScanContext *ctx, tupleSlot *slot) {
    ctx->db_oid = (Oid) slot_getattr(slot, 0);
    return true;
}

int map_pg_class(ScanContext *ctx, tupleSlot *slot) {
//    if (strcmp(((NameData*) slot_getattr(slot, 1))->data, "pg_type") == 0 && is_visible(slot)) {
//        ctx->pg_type_relid = (Oid) slot_getattr(slot, 0);
//    } else {
    ctx->relid = (Oid) slot_getattr(slot, 0);
    ctx->relname = ((NameData*) slot_getattr(slot, 1))->data;
    ctx->relfilenode = (Oid) slot_getattr(slot, 7);
    Oid toast_id = (Oid) slot_getattr(slot, 12);

//    printf("toast_id: %d %d ", ctx->relid, toast_id);
    if (toast_id != 0) {
        ctx->has_toast = true;
        ctx->toast_relid = toast_id;
    }
//    else if ( toast_id == 0 && ctx->relfilenode != 0 && strstr(relname->data, buf) != NULL) {
//        // toast index file
//        printf(" tost index name: %s \n", relname->data);
//    }
//    }

    return true;
};

bool exec_pg_index_filter(ScanContext *ctx, tupleSlot *slot) {
    NameData* relname = (NameData*) slot_getattr(slot, 1);
    char buf[120];
    snprintf(buf, sizeof(buf), "%u_index", ctx->relid);
    if (strstr(relname->data, buf) != NULL) {
        return true;
    }
    return false;
}

int map_pg_index(ScanContext *ctx, tupleSlot *slot) {
    Oid relid = (Oid) slot_getattr(slot, 0);
    Oid relfilenode = (Oid) slot_getattr(slot, 7);
    ctx->toast_index_relid = relid;
    ctx->toast_index_relfilenodeid = relfilenode;
    return true;
}

bool exec_pg_toast_filter(ScanContext *ctx, tupleSlot *slot) {
    Oid toast_relid = (Oid) slot_getattr(slot, 0);
    if (toast_relid == ctx->toast_relid) {
        return true;
    }
    return false;
}

int map_pg_toast(ScanContext *ctx, tupleSlot *slot) {
    Oid toast_relfilenode = (Oid) slot_getattr(slot, 7);
    ctx->toast_relfilenodeid = toast_relfilenode;
    return true;
}


bool exec_pg_attribute_filter(ScanContext *ctx, tupleSlot *slot) {
    Oid pg_attribute_oid = (Oid) slot_getattr(slot, 0);
    return (ctx->relid == pg_attribute_oid && is_visible(slot, -1, -1));
};

bool exec_user_table_filter(ScanContext *ctx, tupleSlot *slot) {
    return true;
};



ColumnInfo map_pg_attribute(tupleSlot *slot)
{
    ColumnInfo col;

    col.attname = DatumGetPointer(slot_getattr(slot, 1));   // ❗不 strdup、不 decode
    col.atttypid = (Oid)slot_getattr(slot, 2);
    col.attlen   = (int16)slot_getattr(slot, 3);
    col.attnum   = (int16)slot_getattr(slot, 4);
    col.attbyval = (bool)slot_getattr(slot, 8);
    col.attalign = (char)slot_getattr(slot, 9);

    return col;
}

void schema_builder_add(SchemaBuilder *b, ColumnInfo col)
{
//    memset(b, 0, sizeof(SchemaBuilder));
    if (b->cols == NULL) {
        b->cap = 8;
        b->ncols = 0;
        b->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * b->cap);
    }

    if (b->ncols >= b->cap) {
        b->cap *= 2;
        b->cols = (ColumnInfo*) realloc(b->cols, sizeof(ColumnInfo) * b->cap);
    }

    b->cols[b->ncols] = col;

    if (col.attname) {
        b->cols[b->ncols].attname = strdup(((NameData *)col.attname)->data);
    }

    b->ncols++;
}

int cmp_attnum(const void *a, const void *b)
{
    const ColumnInfo *ca = (const ColumnInfo *)a;
    const ColumnInfo *cb = (const ColumnInfo *)b;
    return ca->attnum - cb->attnum;
}

Schema *schema_builder_finalize(SchemaBuilder *b)
{
    int valid = 0;
    for (int i = 0; i < b->ncols; i++)
    {
        if (b->cols[i].attnum > 0)
        {
            b->cols[valid++] = b->cols[i];
        }
    }

    qsort(b->cols, valid, sizeof(ColumnInfo), cmp_attnum);

    Schema *s = (Schema*) malloc(sizeof(Schema));

//    s->natts = b->ncols;
    s->natts = valid;
    s->cols = (ColumnInfo*) malloc(sizeof(ColumnInfo) * b->ncols);

    memcpy(s->cols, b->cols, sizeof(ColumnInfo) * b->ncols);

    return s;
}

toastChunk extract_toast_chunk(tupleSlot *slot) {
    toastChunk c;
    Oid chunk_id = (Oid) slot_getattr(slot, 0);
    int32_t seq = (int32_t) slot_getattr(slot, 1);
    c.chunk_id = chunk_id;
    c.seq = seq;
    struct varlena *v = (struct varlena *) slot_getattr(slot, 2);

    c.len  = VARSIZE_ANY_EXHDR(v);
    c.data = (char*) malloc(c.len);
    memcpy(c.data, VARDATA_ANY(v), c.len);

    return c;
}

void map_user_table(ScanContext *ctx, tupleSlot *slot)
{
    Schema *schema = ctx->schema;

    if (ctx->op->output_name == 1 && ctx->fp != NULL) {
        if (ctx->header_written == 0) {
            for (int i = 0; i < schema->natts; ++i) {
                ColumnInfo *col = &schema->cols[i];
                int is_last = (i == schema->natts - 1);
                csv_write_field(ctx->fp, col->attname, is_last);
            }
            csv_end_row(ctx->fp);
            ctx->header_written = 1;
        }

    }

    if (ctx->op->show_sql == 1) {
        printf("INSERT INTO %s(", ctx->relname);
        for (int i = 0; i < schema->natts; i++) {
            ColumnInfo *col = &schema->cols[i];
            if (strcmp(col->attname, "xmin") == 0 || strcmp(col->attname, "cmin") == 0 || strcmp(col->attname, "xmax") == 0
                || strcmp(col->attname, "cmax") == 0 || strcmp(col->attname, "tableoid") == 0) {
                continue;
            }
            if (i == schema->natts - 1) {
                printf("\"%s\"", col->attname);
            } else {
                printf("\"%s\", ", col->attname);
            }

            if (i == schema->natts - 1) {
                printf(")");
            }
        }
        printf(" VALUES(");
    } else {
        printf("{ ");
    }


    for (int i = 0; i < schema->natts; i++)
    {
        ColumnInfo *col = &schema->cols[i];
        if (ctx->op->show_sql != 1) {
            if (strcmp(col->attname, "xmin") == 0 || strcmp(col->attname, "cmin") == 0 || strcmp(col->attname, "xmax") == 0
                || strcmp(col->attname, "cmax") == 0 || strcmp(col->attname, "tableoid") == 0) {
                continue;
            } else {
                printf("%s: ", col->attname);
            }

        }
        char* val = NULL;
        Datum d = slot_getattr(slot, i);
        if (d == 0 && slot->is_null[i])
        {
            printf("NULL");
        } else {
            val = datum_to_string(ctx, d, col->atttypid);
            printf("'%s'", val);
        }


        if (i != schema->natts - 1) {
            printf(", ");
            if (ctx->op->output_name == 1 && ctx->fp != NULL) {
                csv_write_field(ctx->fp, val, 0);
            }
        } else {
            if (ctx->op->output_name == 1 && ctx->fp != NULL) {
                csv_write_field(ctx->fp, val, 1);
            } else {
                if (ctx->op->show_sql == 1) {
                    printf(");");

                }

            }

        }

        free(val);

    }
    if (ctx->op->show_sql != 1) {
        printf(" }\n");
    }
    printf(" ctid: (%d, %hu)\n", (slot->mvcc.ctid.ip_blkid.bi_hi << 16 | slot->mvcc.ctid.ip_blkid.bi_lo), slot->mvcc.ctid.ip_posid);
}



void toast_cache_add(toastCache *cache, toastChunk chunk) {
    toastChunkList *list = toast_cache_get_list(cache, chunk.chunk_id);

    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->chunks = (toastChunk *) realloc(list->chunks,
                                              sizeof(toastChunk) * list->capacity);
    }

    list->chunks[list->count++] = chunk;

    if (chunk.seq > list->max_seq) {
        list->max_seq = chunk.seq;
    }
}



bool extract_tuple(ScanContext *ctx, ItemIdData *item) {
    char *page = ctx->page_buf;
    uint16_t len = item->lp_len;
    uint16_t offset = item->lp_off;
    uint16_t flags = item->lp_flags;

    // 安全校验
    if (len == 0)
        return false;

//    if (offset + len > ctx->)
//        return;

    char* tuple = page + offset;

    bool is_damaged = decode_tuple(ctx->slot, tuple, len, ctx->vsible_ctx);
    if (!is_damaged) {
        return false;
    }
    switch (ctx->phase)
    {
        case PHASE_PG_DATABASE:
        {
            if (!exec_pg_database_filter(ctx, ctx->slot)) {
                return false;
            }
            map_pg_database(ctx, ctx->slot);
            break;
        }
        case PHASE_PG_CLASS:
        {
            if (!exec_pg_class_filter(ctx, ctx->slot)) {
                return false;
            }
            map_pg_class(ctx, ctx->slot);

            break;
        }
        case PHASE_PG_ATTRIBUTE:
        {
            if (!exec_pg_attribute_filter(ctx, ctx->slot)) {
                return false;
            }
            ColumnInfo col = map_pg_attribute(ctx->slot);
            schema_builder_add(ctx->schema_builder, col);
            break;
        }
        case PHASE_USER_TABLE:
        {
            if (!exec_user_table_filter(ctx, ctx->slot)) {
                return false;
            }
            map_user_table(ctx, ctx->slot);
            break;
        }
        case PHASE_PG_ATTRIBUTE_DONE:
            break;
        case PHASE_DETOAST:
            break;
        case PHASE_DONE:
            break;
        case PHASE_PG_INDEX:
        {
            if (!exec_pg_index_filter(ctx, ctx->slot)) {
                return false;
            }
            map_pg_index(ctx, ctx->slot);
            break;
        }
        case PHASE_PG_TOAST:
        {
            if (!exec_pg_toast_filter(ctx, ctx->slot)) {
                return false;
            }
            map_pg_toast(ctx, ctx->slot);
            break;
        }
        case PHASE_PG_TOAST_DECODE:
        {
            toastChunk c = extract_toast_chunk(ctx->slot);
            toast_cache_add(ctx->toast_cache, c);
            break;
        }
    }

    return true;
    // 3️⃣ 输出
//    print_tuple(ctx->slot);
};


void reset_slot(ScanContext *ctx, tupleSlot *slot)
{
    slot->arena.cur = slot->arena.base;
    slot->nvalid = 0;

    // 不需要 free，只清状态
    memset(slot->is_null, true, sizeof(bool) * ctx->slot->schema->natts);
    memset(slot->values, true, sizeof(Datum) * ctx->slot->schema->natts);
}

void scan_page(ScanContext *ctx) {
    char *page = ctx->page_buf;
    PageHeaderData *header = (PageHeaderData *)page;
    int line_count = (header->pd_lower - sizeof(PageHeaderData)) / sizeof(ItemIdData);
    ItemId items = (ItemId)(page + sizeof(PageHeaderData));
    for (int i = 0; i < line_count; i++) {
        reset_slot(ctx, ctx->slot);
        if (ItemIdIsUsed(&items[i])) {
            bool is_damaged_or_invisible = extract_tuple(ctx, &items[i]);
            if (!is_damaged_or_invisible) {
                continue;
            }
        }
    }

};

void scan_wal_page(ScanContext *ctx) {
    char *page = ctx->page_buf;
    XLogRecPtr hdrsz;
    XLogPageHeader wal_header = (XLogPageHeader) page;
    uint32 rem_len = MAXALIGN(wal_header->xlp_rem_len);
    uint16 xlp_info = wal_header->xlp_info;

    if (xlp_info & ~XLP_ALL_FLAGS)
    {
        LOG_ERROR("invalid xlp_info");
        return;
    }
    if (wal_header->xlp_magic != XLOG_PAGE_MAGIC) {
        LOG_ERROR("page may not wal page");
        return;
    }

    hdrsz =
            (wal_header->xlp_info & XLP_LONG_HEADER)
            ? SizeOfXLogLongPHD
            : SizeOfXLogShortPHD;

    if (wal_header->xlp_info &
        (XLP_FIRST_IS_CONTRECORD |
         XLP_FIRST_IS_OVERWRITE_CONTRECORD)) {
        // record 是上一页延续
        rem_len = MAXALIGN(wal_header->xlp_rem_len);

        if (hdrsz + rem_len > XLOG_BLCKSZ)
        {
            LOG_ERROR("invalid contrecord length");
            return;
        }

        hdrsz += rem_len;
    }

    /* backup block removable */
    if (wal_header->xlp_info & XLP_BKP_REMOVABLE)
    {
        printf("backup removable\n");
    }
    char *recptr = page + hdrsz;
    char *page_end = page + XLOG_BLCKSZ;


    uint32 aligned_len;
    uint32 remaining;

    remaining = page_end - recptr;

    if (remaining < SizeOfXLogRecord)
    {
        return;
    }



//    XLogRecPtr start_ptr = InvalidXLogRecPtr;
//    while (xlog_find_next_record(state, start_ptr) != NULL) {
//        if (RecPtr <= state->ReadRecPtr) {
//            /* Rewind the reader to the beginning of the last record. */
//            found = state->ReadRecPtr;
//            XLogBeginRead(state, found);
//
//        }
//    }

}

XLogRecPtr xlog_find_next_record(XLogReaderState *state, XLogRecPtr RecPtr) {

}

int read_xlog_record() {

}

int read_next_xlog_reocrd() {

}