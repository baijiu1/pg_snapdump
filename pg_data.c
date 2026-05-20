//
// Created by 白杰 on 2026/5/20.
//

#include "pg_data.h"

void deform_tuple(tupleSlot *slot) {

    bits8 *t_bits = NULL;
    bits8	   *bp = slot->tup->t_bits;
    bool		hasnulls = HeapTupleHasNulls(slot->tup);
    Schema *schema = slot->schema;
    slot->schema_natts = schema->natts;
    char *end = slot->tuple_data + slot->tuple_len;
    char *t_data = (char *) slot->tup + slot->tup->t_hoff;
    int off = 0;
    for (int i = 0; i < schema->natts; i++) {
        ColumnInfo *col = &schema->cols[i];
        switch (col->attnum) {
            case -1:
                // ctid
                memcpy(&slot->values[i], &slot->tup->t_ctid, sizeof(slot->tup->t_ctid));
                return;
            case -2:
                // xmin
                memcpy(&slot->values[i], &slot->tup->t_choice.t_heap.t_xmin, sizeof(slot->tup->t_choice.t_heap.t_xmin));
                return;
            case -3:
                // cmin
                memcpy(&slot->values[i], &slot->tup->t_choice.t_heap.t_field3.t_cid, sizeof(slot->tup->t_choice.t_heap.t_field3.t_cid));
                return;
            case -4:
                // xmax
                memcpy(&slot->values[i], &slot->tup->t_choice.t_heap.t_xmax, sizeof(slot->tup->t_choice.t_heap.t_xmax));
                return;
            case -5:
                // cmax
                memcpy(&slot->values[i], &slot->tup->t_choice.t_heap.t_field3.t_cid, sizeof(slot->tup->t_choice.t_heap.t_field3.t_cid));
                return;
            case -6:
                // tableoid
                memcpy(&slot->values[i], &slot->relid, sizeof(slot->relid));
                return;
        }
        slot->is_null[i] = false;
        if (i >= slot->tuple_natts)
        {
            slot->is_null[i] = true;
            slot->values[i] = (Datum) 0;
            continue;
        }
        if (hasnulls && att_isnull(i, bp)) {
            slot->is_null[i] = true;
            slot->values[i] = (Datum) 0;
            continue;
        }

        slot->nvalid = i;
        slot->is_null[i] = false;

        if (col->attlen == -1)
        {
            off = att_align_pointer(off,
                                    col->attalign,
                                    -1,
                                    t_data + off);
        }
        else
        {
            off = att_align_nominal(off,
                                    col->attalign);

        }

        if (t_data > end) {
            LOG_ERROR("overflow at col %d", i);
            break;
        }

        if (i >= schema->natts) {
            LOG_ERROR("overflow at col %d", i);
            break;
        }
        if (i == slot->tuple_natts) {
            break;
        }

        char *ptr = t_data + off;


        slot->values[i] = decode_value(ptr, col);

        off = att_addlength_pointer(off, col->attlen,  ptr);
    }
}

bool is_visible(tupleSlot *slot, int xmin_status, int xmax_status)
{
    if (xmax_status != -1) {
        // 1️⃣ 看 xmin
        if (xmin_status == TRANSACTION_STATUS_ABORTED || xmin_status == TRANSACTION_STATUS_IN_PROGRESS)
            return false;

        // 2️⃣ xmin 必须 committed
        if (xmin_status != TRANSACTION_STATUS_COMMITTED)
            return false;

        // 3️⃣ 看 xmax
        if (xmax_status == 0)   // INVALID
            return true;

        if (xmax_status == TRANSACTION_STATUS_ABORTED)
            return true;

        if (xmax_status == TRANSACTION_STATUS_IN_PROGRESS)
            return true; // 简化

        if (xmax_status == TRANSACTION_STATUS_COMMITTED)
            return false;

        return false;
    } else {
        // xmin 判定
        if (slot->tup->t_infomask & HEAP_XMIN_INVALID)
            return false;

        if (!(slot->tup->t_infomask & HEAP_XMIN_COMMITTED))
            return false; // 或 UNKNOWN

        // xmax 判定
        if (slot->mvcc.xmax == 0)
            return true;

        if (slot->tup->t_infomask & HEAP_XMAX_INVALID)
            return true;

        if (slot->tup->t_infomask & HEAP_XMAX_COMMITTED)
            return false;

        // 未知状态
        return false;
    }
}

bool decode_tuple(tupleSlot *slot, char *tuple, uint16_t len, Visibility_ctx* v_ctx)
{
    HeapTupleHeaderData *htup = (HeapTupleHeaderData *)tuple;
    slot->tuple_data = tuple;
    slot->tuple_len = len;
    slot->tup = htup;
    slot->tuple_natts = HeapTupleHeaderGetNatts(htup->t_infomask2);
    slot->mvcc.xmin = HeapTupleHeaderGetRawXmin(htup);
    slot->mvcc.xmax = HeapTupleHeaderGetRawXmax(htup);
    int bitmap_size = (slot->tuple_natts + 7) / 8;
    int min_hoff =
            MAXALIGN(
                    sizeof(HeapTupleHeaderData) +
                    (HeapTupleHasNulls(htup) ? bitmap_size : 0)
            );
    if (htup->t_hoff < min_hoff || htup->t_hoff % 8 != 0 || htup->t_hoff < sizeof(HeapTupleHeader) || htup->t_hoff >= len) {
        printf("tuple may be damaged. skip.");
        return false;
    }

    if (v_ctx->only_visible == 1 && v_ctx->phase == PHASE_USER_TABLE) {
        v_ctx->xmin_snapshot = slot->mvcc.xmin;
        v_ctx->xmax_snapshot = slot->mvcc.xmax;
        if (v_ctx->visible_mode == VISIBILITY_FULL) {
            int status_xmax = 0, status_xmin = 0;
            // pg_xact
            if (v_ctx->xmax_snapshot != 0)
            {
                status_xmax =
                        pg_xact_lookup(v_ctx->xmax_snapshot, v_ctx);
            }
            status_xmin = pg_xact_lookup(v_ctx->xmin_snapshot, v_ctx);
            bool is_v = is_visible(slot, status_xmin, status_xmax);
            if (!is_v) {
                return false;
            }
        } else {
            // hint only
            bool is_v = is_visible(slot, -1, -1);
            if (!is_v) {
                return false;
            }
        }


    }
    slot->mvcc.infomask = htup->t_infomask;
    slot->mvcc.infomask2 = htup->t_infomask2;
    slot->mvcc.ctid = htup->t_ctid;
    return true;
}

Datum slot_getattr(tupleSlot *slot, int attnum)
{
    // already available
    if (attnum < slot->nvalid)
        return slot->values[attnum];

    deform_tuple(slot);

    return slot->values[attnum];
}