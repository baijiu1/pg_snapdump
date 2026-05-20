//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_HEAP_H
#define PG_HEXRETRO_PG_HEAP_H

#include "pg_c.h"

typedef struct BlockIdData
{
    uint16_t 		bi_hi;
    uint16_t		bi_lo;
} BlockIdData;

typedef struct ItemPointerData
{
    BlockIdData ip_blkid;
    OffsetNumber ip_posid;
}

/* If compiler understands packed and aligned pragmas, use those */
#if defined(pg_attribute_packed) && defined(pg_attribute_aligned)
pg_attribute_packed()
    pg_attribute_aligned(2)
#endif
        ItemPointerData;

typedef struct HeapTupleFields
{
    TransactionId t_xmin;		/* inserting xact ID */
    TransactionId t_xmax;		/* deleting or locking xact ID */

    union
    {
        CommandId	t_cid;		/* inserting or deleting command ID, or both */
        TransactionId t_xvac;	/* old-style VACUUM FULL xact ID */
    }			t_field3;
} HeapTupleFields;

typedef struct DatumTupleFields
{
    int32_t 		datum_len_;		/* varlena header (do not touch directly!) */

    int32_t		datum_typmod;	/* -1, or identifier of a record type */

    unsigned int			datum_typeid;	/* composite type OID, or RECORDOID */

    /*
     * datum_typeid cannot be a domain over composite, only plain composite,
     * even if the datum is meant as a value of a domain-over-composite type.
     * This is in line with the general principle that CoerceToDomain does not
     * change the physical representation of the base type value.
     *
     * Note: field ordering is chosen with thought that Oid might someday
     * widen to 64 bits.
     */
} DatumTupleFields;

typedef struct HeapTupleHeader {
    union {
        HeapTupleFields t_heap;
        DatumTupleFields t_datum;
    } t_choice;

    ItemPointerData t_ctid; /* current TID of this or newer tuple */

    /* Fields below here must match MinimalTupleData! */

    uint16_t t_infomask2; /* number of attributes + various flags */

    uint16_t t_infomask; /* various flag bits, see below */

    uint8_t t_hoff; /* sizeof header incl. bitmap, padding */

    /* ^ - 23 bytes - ^ */

    bits8 t_bits[FLEXIBLE_ARRAY_MEMBER]; /* bitmap of NULLs -- VARIABLE LENGTH */

    /* MORE DATA FOLLOWS AT END OF STRUCT */
} HeapTupleHeaderData;
typedef struct HeapTupleHeaderData* HeapTupleHeader;

typedef struct nameData
{
    char		data[NAMEDATALEN];
} NameData;




#define LP_UNUSED 0   /* unused (should always have lp_len=0) */
#define LP_NORMAL 1   /* used (should always have lp_len>0) */
#define LP_REDIRECT 2 /* HOT redirect (should have lp_len=0) */
#define LP_DEAD 3     /* dead, may or may not have storage */

#define LP_INDEX_FROZEN 2 /* index tuple's xmin is frozen (used for multi-version btree index only) */
#define PD_HAS_FREE_LINES	0x0001	/* are there any unused line pointers? */
#define PD_PAGE_FULL		0x0002	/* not enough free space for new tuple? */
#define PD_ALL_VISIBLE		0x0004	/* all tuples on page are visible to
									 * everyone */

#define PD_VALID_FLAG_BITS	0x0007	/* OR of all valid pd_flags bits */

#define ItemIdIsUsed(itemId) \
	((itemId)->lp_flags != LP_UNUSED)

#define LWLockMode int
#define LOCKMODE int
#define BlockNumber unsigned int
//#define Oid unsigned int
#define ForkNumber int
//#define bool unsigned char

typedef struct ItemIdData
{
    unsigned	lp_off:15,		/* offset to tuple (from start of page) */
    lp_flags:2,		/* state of line pointer, see below */
    lp_len:15;		/* byte length of tuple */
} ItemIdData;

typedef ItemIdData* ItemId;

typedef struct {
    uint32_t xlogid;  /* high bits */
    uint32_t xrecoff; /* low bits */
} PageXLogRecPtr;


typedef struct PageHeaderData
{
    /* XXX LSN is member of *any* block, not only page-organized ones */
    PageXLogRecPtr pd_lsn;		/* LSN: next byte after last byte of xlog
								 * record for last change to this page */
    uint16_t 		pd_checksum;	/* checksum */
    uint16_t		pd_flags;		/* flag bits, see below */
    LocationIndex pd_lower;		/* offset to start of free space */
    LocationIndex pd_upper;		/* offset to end of free space */
    LocationIndex pd_special;	/* offset to start of special space */
    uint16_t		pd_pagesize_version;
    TransactionId pd_prune_xid; /* oldest prunable XID, or zero if none */
    ItemIdData	pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* line pointer array */
} PageHeaderData;

typedef PageHeaderData *PageHeader;



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

/* Size of a disk block --- this also limits the size of a tuple. You can set
   it bigger if you need bigger tuples (although TOAST should reduce the need
   to have large tuples, since fields can be spread across multiple tuples).
   BLCKSZ must be a power of 2. The maximum possible value of BLCKSZ is
   currently 2^15 (32768). This is determined by the 15-bit widths of the
   lp_off and lp_len fields in ItemIdData (see include/storage/itemid.h).
   Changing BLCKSZ requires an initdb. */
#define BLCKSZ 8192


#define  TYPTYPE_BASE		'b' /* base type (ordinary scalar type) */
#define  TYPTYPE_COMPOSITE	'c' /* composite (e.g., table's rowtype) */
#define  TYPTYPE_DOMAIN		'd' /* domain over another type */
#define  TYPTYPE_ENUM		'e' /* enumerated type */
#define  TYPTYPE_MULTIRANGE	'm' /* multirange type */
#define  TYPTYPE_PSEUDO		'p' /* pseudo-type */
#define  TYPTYPE_RANGE		'r' /* range type */

#define  TYPCATEGORY_INVALID	'\0'	/* not an allowed category */
#define  TYPCATEGORY_ARRAY		'A'
#define  TYPCATEGORY_BOOLEAN	'B'
#define  TYPCATEGORY_COMPOSITE	'C'
#define  TYPCATEGORY_DATETIME	'D'
#define  TYPCATEGORY_ENUM		'E'
#define  TYPCATEGORY_GEOMETRIC	'G'
#define  TYPCATEGORY_NETWORK	'I' /* think INET */
#define  TYPCATEGORY_NUMERIC	'N'
#define  TYPCATEGORY_PSEUDOTYPE 'P'
#define  TYPCATEGORY_RANGE		'R'
#define  TYPCATEGORY_STRING		'S'
#define  TYPCATEGORY_TIMESPAN	'T'
#define  TYPCATEGORY_USER		'U'
#define  TYPCATEGORY_BITSTRING	'V' /* er ... "varbit"? */
#define  TYPCATEGORY_UNKNOWN	'X'
#define  TYPCATEGORY_INTERNAL	'Z'

#define  TYPALIGN_CHAR			'c' /* char alignment (i.e. unaligned) */
#define  TYPALIGN_SHORT			's' /* short alignment (typically 2 bytes) */
#define  TYPALIGN_INT			'i' /* int alignment (typically 4 bytes) */
#define  TYPALIGN_DOUBLE		'd' /* double alignment (often 8 bytes) */

#define  TYPSTORAGE_PLAIN		'p' /* type not prepared for toasting */
#define  TYPSTORAGE_EXTERNAL	'e' /* toastable, don't try to compress */
#define  TYPSTORAGE_EXTENDED	'x' /* fully toastable */
#define  TYPSTORAGE_MAIN		'm' /* like 'x' but try to store inline */

#define TYPEALIGN(ALIGNVAL,LEN)  \
	(((uintptr_t) (LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t) ((ALIGNVAL) - 1)))

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define SHORTALIGN(LEN)			TYPEALIGN(ALIGNOF_SHORT, (LEN))
#define INTALIGN(LEN)			TYPEALIGN(ALIGNOF_INT, (LEN))
#define LONGALIGN(LEN)			TYPEALIGN(ALIGNOF_LONG, (LEN))
#define DOUBLEALIGN(LEN)		TYPEALIGN(ALIGNOF_DOUBLE, (LEN))
#define MAXALIGN(LEN)			TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))
/* MAXALIGN covers only built-in types, not buffers */
#define BUFFERALIGN(LEN)		TYPEALIGN(ALIGNOF_BUFFER, (LEN))
#define CACHELINEALIGN(LEN)		TYPEALIGN(PG_CACHE_LINE_SIZE, (LEN))

#define TYPEALIGN_DOWN(ALIGNVAL,LEN)  \
	(((uintptr_t) (LEN)) & ~((uintptr_t) ((ALIGNVAL) - 1)))

#define SHORTALIGN_DOWN(LEN)	TYPEALIGN_DOWN(ALIGNOF_SHORT, (LEN))
#define INTALIGN_DOWN(LEN)		TYPEALIGN_DOWN(ALIGNOF_INT, (LEN))
#define LONGALIGN_DOWN(LEN)		TYPEALIGN_DOWN(ALIGNOF_LONG, (LEN))
#define DOUBLEALIGN_DOWN(LEN)	TYPEALIGN_DOWN(ALIGNOF_DOUBLE, (LEN))
#define MAXALIGN_DOWN(LEN)		TYPEALIGN_DOWN(MAXIMUM_ALIGNOF, (LEN))
#define BUFFERALIGN_DOWN(LEN)	TYPEALIGN_DOWN(ALIGNOF_BUFFER, (LEN))

/*
 * The above macros will not work with types wider than uintptr_t, like with
 * uint64 on 32-bit platforms.  That's not problem for the usual use where a
 * pointer or a length is aligned, but for the odd case that you need to
 * align something (potentially) wider, use TYPEALIGN64.
 */
#define TYPEALIGN64(ALIGNVAL,LEN)  \
	(((uint64) (LEN) + ((ALIGNVAL) - 1)) & ~((uint64) ((ALIGNVAL) - 1)))

/* we don't currently need wider versions of the other ALIGN macros */
#define MAXALIGN64(LEN)			TYPEALIGN64(MAXIMUM_ALIGNOF, (LEN))
#define Assert(condition)	((void)true)
#define AssertMacro(condition)	((void)true)



#define MAXALIGN(LEN) TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))

#define HEAP_NATTS_MASK 0x07FF
#define HeapTupleHeaderGetNatts(t_infomask2) \
    (t_infomask2 & HEAP_NATTS_MASK)

#define SizeofHeapTupleHeader offsetof(HeapTupleHeaderData, t_bits)

#define att_isnull(attnum, bp) \
    (((((const uint8_t *) (bp))[(attnum) >> 3]) & (1 << ((attnum) & 0x07))) == 0)

#define attr_isnull(i, bp, hasnull) \
    ((hasnull) && ((bp[(i)>>3] & (1<<((i)&7))) == 0))


/* The normal alignment of `long long int', in bytes. */
/* #undef ALIGNOF_LONG_LONG_INT */

/* The normal alignment of `PG_INT128_TYPE', in bytes. */
#define ALIGNOF_PG_INT128_TYPE 16

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

#define HeapTupleHeaderGetRawXmin(tup) \
( \
	(tup)->t_choice.t_heap.t_xmin \
)

#define HeapTupleHeaderXminFrozen(tup) \
( \
	((tup)->t_infomask & (HEAP_XMIN_FROZEN)) == HEAP_XMIN_FROZEN \
)

#define HeapTupleHeaderXminInvalid(tup) \
( \
	((tup)->t_infomask & (HEAP_XMIN_COMMITTED|HEAP_XMIN_INVALID)) == \
		HEAP_XMIN_INVALID \
)

#define HeapTupleHeaderGetXmin(tup) \
( \
	HeapTupleHeaderXminFrozen(tup) ? \
		FrozenTransactionId : HeapTupleHeaderGetRawXmin(tup) \
)

#define HeapTupleHeaderGetRawXmax(tup) \
    ((tup)->t_choice.t_heap.t_xmax)

#define att_align_nominal_ptr(cur_ptr, attalign) \
( \
    ((attalign) == TYPALIGN_INT) ? \
        (char *) INTALIGN(cur_ptr) : \
    ((attalign) == TYPALIGN_CHAR) ? \
        (char *)(cur_ptr) : \
    ((attalign) == TYPALIGN_DOUBLE) ? \
        (char *) DOUBLEALIGN(cur_ptr) : \
    ( \
        AssertMacro((attalign) == TYPALIGN_SHORT), \
        (char *) SHORTALIGN(cur_ptr) \
    ) \
)

#define att_align_pointer_ptr(cur_ptr, attalign, attlen, attptr) \
( \
    ((attlen) == -1 && VARATT_NOT_PAD_BYTE(attptr)) ? \
        (char *)(cur_ptr) : \
        att_align_nominal_ptr(cur_ptr, attalign) \
)

#define att_addlength_pointer_ptr(cur_ptr, attlen, attptr) \
( \
    ((attlen) > 0) ? \
        ((char *)(cur_ptr) + (attlen)) : \
    ((attlen) == -1) ? \
        ((char *)(cur_ptr) + VARSIZE_ANY(attptr)) : \
    ( \
        AssertMacro((attlen) == -2), \
        ((char *)(cur_ptr) + (strlen((char *)(attptr)) + 1)) \
    ) \
)

#define att_addlength_pointer_off(off, attlen, attptr) \
( \
    ((attlen) > 0) ? \
        ((off) + (attlen)) : \
    ((attlen) == -1) ? \
        ((off) + VARSIZE_ANY(attptr)) : \
    ( \
        AssertMacro((attlen) == -2), \
        ((off) + (strlen((char *)(attptr)) + 1)) \
    ) \
)


#define att_align_nominal(cur_offset, attalign) \
( \
	((attalign) == TYPALIGN_INT) ? INTALIGN(cur_offset) : \
	 (((attalign) == TYPALIGN_CHAR) ? (uintptr_t) (cur_offset) : \
	  (((attalign) == TYPALIGN_DOUBLE) ? DOUBLEALIGN(cur_offset) : \
	   ( \
			AssertMacro((attalign) == TYPALIGN_SHORT), \
			SHORTALIGN(cur_offset) \
	   ))) \
)

#define att_align_pointer(cur_offset, attalign, attlen, attptr) \
( \
	((attlen) == -1 && VARATT_NOT_PAD_BYTE(attptr)) ? \
	(uintptr_t) (cur_offset) : \
	att_align_nominal(cur_offset, attalign) \
)

#define att_addlength_pointer(cur_offset, attlen, attptr) \
( \
	((attlen) > 0) ? \
	( \
		(cur_offset) + (attlen) \
	) \
	: (((attlen) == -1) ? \
	( \
		(cur_offset) + VARSIZE_ANY(attptr) \
	) \
	: \
	( \
		AssertMacro((attlen) == -2), \
		(cur_offset) + (strlen((char *) (attptr)) + 1) \
	)) \
)
#endif //PG_HEXRETRO_PG_HEAP_H
