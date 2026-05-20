//
// Created by 白杰 on 2026/5/20.
//

#include "pg_xact.h"

static inline uint32_t
xact_hash(int segno, int pageindex)
{
    return ((uint32_t)segno * 131u +
            (uint32_t)pageindex);
}

xact_ctl *
xact_cache_create(int max_pages)
{
    xact_ctl *ctl =
            (xact_ctl *)malloc(sizeof(xact_ctl));

    if (ctl == NULL)
        return NULL;

    memset(ctl, 0, sizeof(xact_ctl));

    ctl->max_pages = max_pages;

    ctl->bucket_count = BUCKET_COUNT;

    ctl->buckets =
            (xact_cache **)calloc(
                    ctl->bucket_count,
                    sizeof(xact_cache *)
            );

    if (ctl->buckets == NULL)
    {
        free(ctl);
        return NULL;
    }

    return ctl;
}

xact_cache *
xact_cache_lookup(xact_ctl *ctl,
                  int segno,
                  int pageindex)
{
    uint32_t hash =
            xact_hash(segno, pageindex);

    uint32_t bucket =
            hash % ctl->bucket_count;

    xact_cache *cur =
            ctl->buckets[bucket];

    while (cur)
    {
        if (cur->segno == segno &&
            cur->pageindex == pageindex)
        {
            cur->hit_count++;

            return cur;
        }

        cur = cur->next;
    }

    return NULL;
}


xact_cache *
xact_cache_insert(xact_ctl *ctl,
                  int segno,
                  int pageindex,
                  const char *pagebuf)
{
    if (ctl->current_pages >= ctl->max_pages)
    {
        printf("cache full\n");
        return NULL;
    }

    uint32_t hash =
            xact_hash(segno, pageindex);

    uint32_t bucket =
            hash % ctl->bucket_count;

    xact_cache *entry =
            (xact_cache *)malloc(sizeof(xact_cache));

    if (entry == NULL)
        return NULL;

    memset(entry, 0, sizeof(xact_cache));

    entry->segno = segno;

    entry->pageindex = pageindex;

    entry->hit_count = 1;

    entry->valid = true;

    memcpy(entry->page, pagebuf, PAGE_SIZE);

    entry->next =
            ctl->buckets[bucket];

    ctl->buckets[bucket] =
            entry;

    ctl->current_pages++;

    return entry;
}

void
xact_cache_destroy(xact_ctl *ctl)
{
    if (ctl == NULL)
        return;

    for (int i = 0; i < ctl->bucket_count; i++)
    {
        xact_cache *cur =
                ctl->buckets[i];

        while (cur)
        {
            xact_cache *tmp =
                    cur->next;

            free(cur);

            cur = tmp;
        }
    }

    free(ctl->buckets);

    free(ctl);
}

static inline int64
TransactionIdToPage(TransactionId xid)
{
    return xid / (int64) CLOG_XACTS_PER_PAGE;
}

XidStatus pg_xact_lookup(TransactionId transactionId, Visibility_ctx* vctx)
{
    if (!TransactionIdIsNormal(transactionId))
    {
        if (TransactionIdEquals(transactionId, BootstrapTransactionId))
            return TRANSACTION_STATUS_COMMITTED;
        if (TransactionIdEquals(transactionId, FrozenTransactionId))
            return TRANSACTION_STATUS_COMMITTED;
        return TRANSACTION_STATUS_ABORTED;
    }

    // 1. 全局 page
    int64 pageno = TransactionIdToPage(transactionId);

// 2. segment 号（文件）
    int segno = TransactionIdToSegment(pageno);

    // segment 内 page 号
    int page_in_seg =
            pageno % CLOG_XACTS_SEG;


// 3. segment 内 page
    int slotno =
            TransactionIdToPgIndex(transactionId);

// 4. page 内 byte
    int byteno = TransactionIdToByte(transactionId);

// 5. byte 内 bit 偏移
    int bshift = TransactionIdToBIndex(transactionId) * CLOG_BITS_PER_XACT;

    xact_cache *entry =
            xact_cache_lookup(
                    vctx->xact_ctl,
                    segno,
                    slotno
            );
    if (entry != NULL) {
        unsigned char* byteptr = (unsigned char*)entry->page + byteno;
        int status =
                (*byteptr >> bshift) & CLOG_XACT_BITMASK;

        return status;
    } else {
        snprintf(vctx->e->relocator->absolute_path, sizeof(vctx->e->relocator->absolute_path), "%s/%04X",
                 vctx->e->relocator->pg_xact_path, segno);
        vctx->init_file_access(vctx->e);

        char* addr = vctx->e->access->get_page(&vctx->e->access->file, page_in_seg);
        xact_cache_insert(
                vctx->xact_ctl,
                segno,
                slotno,
                addr
        );
        unsigned char* byteptr = (unsigned char*) addr + byteno;
        int status =
                (*byteptr >> bshift) & CLOG_XACT_BITMASK;

        return status;
    }

}

//uint32_t get_page(uint32_t xid)
//{
//    return xid / CLOG_XACTS_PER_PAGE;  // 32768
//}

uint32_t get_segment(uint32_t page)
{
    return page / 32;
}

uint32_t get_page_in_segment(uint32_t page)
{
    return page % 32;
}

void get_filename(uint32_t segment, char *buf)
{
    sprintf(buf, "%04u", segment);
}