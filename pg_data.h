//
// Created by 白杰 on 2025/8/8.
//

#ifndef PG_SNPDUMP_PG_DATA_H
#define PG_SNPDUMP_PG_DATA_H


#include "pg_common.h"
#include "pg_type.h"
#include "pg_data.h"
#include "pg_bst.h"
#include <cstdio>
#include <iostream>
#include <cstdlib>
using namespace std;
extern int fileCount;
extern bool onlyNewTuple;

#define MAXIMUM_ALIGNOF 4
#define TYPEALIGN(ALIGNVAL,LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t)((ALIGNVAL) - 1)))

#define MAXALIGN(LEN) TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))

#define HEAP_NATTS_MASK 0x07FF
#define HeapTupleHeaderGetNatts(t_infomask2) \
    (t_infomask2 & HEAP_NATTS_MASK)

#define att_isnull(attnum, bp) \
    (((((const uint8_t *) (bp))[(attnum) >> 3]) & (1 << ((attnum) & 0x07))) == 0)

/* The normal alignment of `double', in bytes. */
#define ALIGNOF_DOUBLE 8

/* The normal alignment of `int', in bytes. */
#define ALIGNOF_INT 4

/* The normal alignment of `long', in bytes. */
#define ALIGNOF_LONG 8

/* The normal alignment of `long long int', in bytes. */
/* #undef ALIGNOF_LONG_LONG_INT */

/* The normal alignment of `PG_INT128_TYPE', in bytes. */
#define ALIGNOF_PG_INT128_TYPE 16

/* The normal alignment of `short', in bytes. */
#define ALIGNOF_SHORT 2
#define AssertMacro(condition) ((void)true)
#define SHORTALIGN(LEN) TYPEALIGN(ALIGNOF_SHORT, (LEN))
#define DOUBLEALIGN(LEN) TYPEALIGN(ALIGNOF_DOUBLE, (LEN))
#define INTALIGN(LEN) TYPEALIGN(ALIGNOF_INT, (LEN))
#define VARATT_NOT_PAD_BYTE(PTR) ((PTR) != 0)
#define VARATT_IS_EXTERNAL(PTR) VARATT_IS_1B_E(PTR)
#define VARATT_IS_4B(Len) \
	((Len & 0x01) == 0x00)
#define VARATT_IS_4B_U(Len) \
	((Len & 0x03) == 0x00)
#define VARATT_IS_4B_C(Len) \
	((Len & 0x03) == 0x02)
#define VARATT_IS_1B(Len) \
	((Len & 0x01) == 0x01)
#define VARATT_IS_1B_E(Len) \
	(Len == 0x01)
// 长度操作
#define VARSIZE_4B(Len) \
	((Len >> 2) & 0x3FFFFFFF)

#define VARSIZE_4B1(PTR) ((((varattrib_4b*)(PTR))->va_4byte.va_header >> 2) & 0x3FFFFFFF)

#define VARSIZE_1B(Len) \
    ((Len >> 1) & 0x7F)
/*
 * These structs describe the header of a varlena object that may have been
 * TOASTed.  Generally, don't reference these structs directly, but use the
 * macros below.
 *
 * We use separate structs for the aligned and unaligned cases because the
 * compiler might otherwise think it could generate code that assumes
 * alignment while touching fields of a 1-byte-header varlena.
 */


//#define VARTAG_1B_E(PTR) (( (PTR))->va_tag)
#define VARTAG_1B_E(PTR) (( (PTR)))

#define VARTAG_EXTERNAL(PTR) VARTAG_1B_E(PTR)
#define VARTAG_IS_EXPANDED(tag) (((tag) & ~1) == VARTAG_EXPANDED_RO)

#define VARTAG_SIZE(tag) ((tag) == VARTAG_INDIRECT ? sizeof(varatt_indirect) : VARTAG_IS_EXPANDED(tag) ? sizeof(varatt_expanded) : (tag) == VARTAG_ONDISK ? sizeof(varatt_external) : (AssertMacro(false), 0))

#define VARHDRSZ_EXTERNAL offsetof(varattrib_1b_e, va_data)

#define VARSIZE_EXTERNAL(PTR)				(VARHDRSZ_EXTERNAL + VARTAG_SIZE(VARTAG_EXTERNAL(PTR)))
#define VARSIZE_ANY(PTR) \
	(VARATT_IS_1B_E(PTR) ? VARSIZE_EXTERNAL(PTR) : \
	 (VARATT_IS_1B(PTR) ? VARSIZE_1B(PTR) : \
	  VARSIZE_4B(PTR)))


#define BITMAPLEN(NATTS)  (((NATTS) + 7) / 8)
/*
* information stored in t_infomask:
*/
#define HEAP_HASNULL			0x0001	/* has null attribute(s) */
#define HEAP_HASVARWIDTH		0x0002	/* has variable-width attribute(s) */
#define HEAP_HASEXTERNAL		0x0004	/* has external stored attribute(s) */
#define HEAP_HASOID_OLD			0x0008	/* has an object-id field */
#define HEAP_XMAX_KEYSHR_LOCK	0x0010	/* xmax is a key-shared locker */
#define HEAP_COMBOCID			0x0020	/* t_cid is a combo CID */
#define HEAP_XMAX_EXCL_LOCK		0x0040	/* xmax is exclusive locker */
#define HEAP_XMAX_LOCK_ONLY		0x0080	/* xmax, if valid, is only a locker */

/* xmax is a shared locker */
#define HEAP_XMAX_SHR_LOCK	(HEAP_XMAX_EXCL_LOCK | HEAP_XMAX_KEYSHR_LOCK)

#define HEAP_LOCK_MASK	(HEAP_XMAX_SHR_LOCK | HEAP_XMAX_EXCL_LOCK | \
						 HEAP_XMAX_KEYSHR_LOCK)
#define HEAP_XMIN_COMMITTED		0x0100	/* t_xmin committed */
#define HEAP_XMIN_INVALID		0x0200	/* t_xmin invalid/aborted */
#define HEAP_XMIN_FROZEN		(HEAP_XMIN_COMMITTED|HEAP_XMIN_INVALID)
#define HEAP_XMAX_COMMITTED		0x0400	/* t_xmax committed */
#define HEAP_XMAX_INVALID		0x0800	/* t_xmax invalid/aborted */
#define HEAP_XMAX_IS_MULTI		0x1000	/* t_xmax is a MultiXactId */
#define HEAP_UPDATED			0x2000	/* this is UPDATEd version of row */
#define HEAP_MOVED_OFF			0x4000	/* moved to another place by pre-9.0
										 * VACUUM FULL; kept for binary
										 * upgrade support */
#define HEAP_MOVED_IN			0x8000	/* moved from another place by pre-9.0
										 * VACUUM FULL; kept for binary
										 * upgrade support */
#define HEAP_MOVED (HEAP_MOVED_OFF | HEAP_MOVED_IN)

#define HEAP_XACT_MASK			0xFFF0	/* visibility-related bits */

#define HeapTupleIsValid(tuple) PointerIsValid(tuple)

#define HeapTupleHasNulls(tuple) (((tuple)->t_infomask & HEAP_HASNULL) != 0)

#define HeapTupleNoNulls(tuple) (!((tuple)->t_infomask & HEAP_HASNULL))

#define HeapTupleHasVarWidth(tuple) (((tuple)->t_infomask & HEAP_HASVARWIDTH) != 0)

#define HeapTupleAllFixed(tuple) (!((tuple)->t_infomask & HEAP_HASVARWIDTH))

#define HeapTupleHasExternal(tuple) (((tuple)->t_infomask & HEAP_HASEXTERNAL) != 0)


void append_to_ctid_chain(const chaseCtidList& chaseTupleNode);

int resolveTableHeapTupleData(char*, int, ColAttribute&, uint16_t, uint16_t);
int printAllCtidChain();
int fetchRows(CtidNode*);

#endif //PG_SNPDUMP_PG_DATA_H
