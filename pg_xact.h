//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_XACT_H
#define PG_HEXRETRO_PG_XACT_H

#include "pg_c.h"
#include "pg_scan.h"

#define BUCKET_COUNT 128

typedef int XidStatus;

#define TRANSACTION_STATUS_IN_PROGRESS		0x00
#define TRANSACTION_STATUS_COMMITTED		0x01
#define TRANSACTION_STATUS_ABORTED			0x02
#define TRANSACTION_STATUS_SUB_COMMITTED	0x03

#define CLOG_BITS_PER_XACT	2
#define CLOG_XACTS_PER_BYTE 4
#define CLOG_XACTS_SEG 32
#define CLOG_XACTS_PER_PAGE (PAGE_SIZE * CLOG_XACTS_PER_BYTE)
#define CLOG_XACT_BITMASK	((1 << CLOG_BITS_PER_XACT) - 1)

#define TransactionIdToSegment(pageno) (pageno / CLOG_XACTS_SEG)
#define TransactionIdToPgIndex(xid) ((xid) % (TransactionId) CLOG_XACTS_PER_PAGE)
#define TransactionIdToByte(xid) (TransactionIdToPgIndex(xid) / CLOG_XACTS_PER_BYTE)
#define TransactionIdToBIndex(xid)	((xid) % (TransactionId) CLOG_XACTS_PER_BYTE)

#define InvalidTransactionId		((TransactionId) 0)
#define BootstrapTransactionId		((TransactionId) 1)
#define FrozenTransactionId			((TransactionId) 2)
#define FirstNormalTransactionId	((TransactionId) 3)
#define MaxTransactionId			((TransactionId) 0xFFFFFFFF)

/* ----------------
 *		transaction ID manipulation macros
 * ----------------
 */
#define TransactionIdIsValid(xid)		((xid) != InvalidTransactionId)
#define TransactionIdIsNormal(xid)		((xid) >= FirstNormalTransactionId)
#define TransactionIdEquals(id1, id2)	((id1) == (id2))
#define TransactionIdStore(xid, dest)	(*(dest) = (xid))
#define StoreInvalidTransactionId(dest) (*(dest) = InvalidTransactionId)

#define EpochFromFullTransactionId(x)	((uint32) ((x).value >> 32))
#define XidFromFullTransactionId(x)		((uint32) (x).value)
#define U64FromFullTransactionId(x)		((x).value)
#define FullTransactionIdEquals(a, b)	((a).value == (b).value)
#define FullTransactionIdPrecedes(a, b)	((a).value < (b).value)
#define FullTransactionIdPrecedesOrEquals(a, b) ((a).value <= (b).value)
#define FullTransactionIdFollows(a, b) ((a).value > (b).value)
#define FullTransactionIdFollowsOrEquals(a, b) ((a).value >= (b).value)
#define FullTransactionIdIsValid(x)		TransactionIdIsValid(XidFromFullTransactionId(x))
#define InvalidFullTransactionId		FullTransactionIdFromEpochAndXid(0, InvalidTransactionId)
#define FirstNormalFullTransactionId	FullTransactionIdFromEpochAndXid(0, FirstNormalTransactionId)
#define FullTransactionIdIsNormal(x)	FullTransactionIdFollowsOrEquals(x, FirstNormalFullTransactionId)

#define TRANSACTION_STATUS_IN_PROGRESS		0x00
#define TRANSACTION_STATUS_COMMITTED		0x01
#define TRANSACTION_STATUS_ABORTED			0x02
#define TRANSACTION_STATUS_SUB_COMMITTED	0x03

typedef int XidStatus;


extern Engine e;
extern void init_file_access(Engine *e);
XidStatus pg_xact_lookup(TransactionId, Visibility_ctx*);
//uint32_t get_page_in_segment(uint32_t page);


typedef struct xact_cache {
    int segno;
    int pageindex;
    uint64 hit_count;
    bool valid;
    char page[PAGE_SIZE];
    struct xact_cache* next;
} xact_cache;


typedef struct xact_ctl {
    int max_pages;
    int current_pages;
    int bucket_count;
    xact_cache** buckets;
} xact_ctl;

xact_ctl *
xact_cache_create(int max_pages);
static inline uint32_t
xact_hash(int segno, int pageindex);





#endif //PG_HEXRETRO_PG_XACT_H
