//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_WAL_H
#define PG_HEXRETRO_PG_WAL_H
#include "pg_c.h"
#include <stdbool.h>

#define XLOG_PAGE_MAGIC 0xD116	/* can be used as WAL version indicator */

typedef struct XLogPageHeaderData
{
    uint16		xlp_magic;		/* magic value for correctness checks */
    uint16		xlp_info;		/* flag bits, see below */
    TimeLineID	xlp_tli;		/* TimeLineID of first record on page */
    XLogRecPtr	xlp_pageaddr;	/* XLOG address of this page */

    /*
     * When there is not enough space on current page for whole record, we
     * continue on the next page.  xlp_rem_len is the number of bytes
     * remaining from a previous page; it tracks xl_tot_len in the initial
     * header.  Note that the continuation data isn't necessarily aligned.
     */
    uint32		xlp_rem_len;	/* total len of remaining data for record */
} XLogPageHeaderData;

#define SizeOfXLogShortPHD	MAXALIGN(sizeof(XLogPageHeaderData))

typedef XLogPageHeaderData *XLogPageHeader;

typedef struct XLogLongPageHeaderData
{
    XLogPageHeaderData std;		/* standard header fields */
    uint64		xlp_sysid;		/* system identifier from pg_control */
    uint32		xlp_seg_size;	/* just as a cross-check */
    uint32		xlp_xlog_blcksz;	/* just as a cross-check */
} XLogLongPageHeaderData;

#define SizeOfXLogLongPHD	MAXALIGN(sizeof(XLogLongPageHeaderData))

typedef XLogLongPageHeaderData *XLogLongPageHeader;

#define XLP_FIRST_IS_CONTRECORD		0x0001
/* This flag indicates a "long" page header */
#define XLP_LONG_HEADER				0x0002
/* This flag indicates backup blocks starting in this page are optional */
#define XLP_BKP_REMOVABLE			0x0004
/* Replaces a missing contrecord; see CreateOverwriteContrecordRecord */
#define XLP_FIRST_IS_OVERWRITE_CONTRECORD 0x0008
/* All defined flag bits in xlp_info (used for validity checking of header) */
#define XLP_ALL_FLAGS				0x000F

#define XLogPageHeaderSize(hdr)		\
	(((hdr)->xlp_info & XLP_LONG_HEADER) ? SizeOfXLogLongPHD : SizeOfXLogShortPHD)


typedef struct XLogRecord
{
    uint32		xl_tot_len;		/* total len of entire record */
    TransactionId xl_xid;		/* xact id */
    XLogRecPtr	xl_prev;		/* ptr to previous record in log */
    uint8		xl_info;		/* flag bits, see below */
    RmgrId		xl_rmid;		/* resource manager for this record */
    /* 2 bytes of padding here, initialize to zero */
    pg_crc32c	xl_crc;			/* CRC for this record */

    /* XLogRecordBlockHeaders and XLogRecordDataHeader follow, no padding */

} XLogRecord;

#define SizeOfXLogRecord	(offsetof(XLogRecord, xl_crc) + sizeof(pg_crc32c))


typedef struct XLogRecordBlockHeader
{
    uint8		id;				/* block reference ID */
    uint8		fork_flags;		/* fork within the relation, and flags */
    uint16		data_length;	/* number of payload bytes (not including page
								 * image) */

    /* If BKPBLOCK_HAS_IMAGE, an XLogRecordBlockImageHeader struct follows */
    /* If BKPBLOCK_SAME_REL is not set, a RelFileLocator follows */
    /* BlockNumber follows */
} XLogRecordBlockHeader;

#define SizeOfXLogRecordBlockHeader (offsetof(XLogRecordBlockHeader, data_length) + sizeof(uint16))


typedef struct XLogRecordBlockImageHeader
{
    uint16		length;			/* number of page image bytes */
    uint16		hole_offset;	/* number of bytes before "hole" */
    uint8		bimg_info;		/* flag bits, see below */

    /*
     * If BKPIMAGE_HAS_HOLE and BKPIMAGE_COMPRESSED(), an
     * XLogRecordBlockCompressHeader struct follows.
     */
} XLogRecordBlockImageHeader;

#define SizeOfXLogRecordBlockImageHeader	\
	(offsetof(XLogRecordBlockImageHeader, bimg_info) + sizeof(uint8))

/* Information stored in bimg_info */
#define BKPIMAGE_HAS_HOLE		0x01	/* page image has "hole" */
#define BKPIMAGE_APPLY			0x02	/* page image should be restored
										 * during replay */
/* compression methods supported */
#define BKPIMAGE_COMPRESS_PGLZ	0x04
#define BKPIMAGE_COMPRESS_LZ4	0x08
#define BKPIMAGE_COMPRESS_ZSTD	0x10


#define	BKPIMAGE_COMPRESSED(info) \
	((info & (BKPIMAGE_COMPRESS_PGLZ | BKPIMAGE_COMPRESS_LZ4 | \
			  BKPIMAGE_COMPRESS_ZSTD)) != 0)
typedef struct XLogRecordBlockCompressHeader
{
    uint16		hole_length;	/* number of bytes in "hole" */
} XLogRecordBlockCompressHeader;

#define SizeOfXLogRecordBlockCompressHeader \
	sizeof(XLogRecordBlockCompressHeader)

#define MaxSizeOfXLogRecordBlockHeader \
	(SizeOfXLogRecordBlockHeader + \
	 SizeOfXLogRecordBlockImageHeader + \
	 SizeOfXLogRecordBlockCompressHeader + \
	 sizeof(RelFileLocator) + \
	 sizeof(BlockNumber))

#define BKPBLOCK_FORK_MASK	0x0F
#define BKPBLOCK_FLAG_MASK	0xF0
#define BKPBLOCK_HAS_IMAGE	0x10	/* block data is an XLogRecordBlockImage */
#define BKPBLOCK_HAS_DATA	0x20
#define BKPBLOCK_WILL_INIT	0x40	/* redo will re-init the page */
#define BKPBLOCK_SAME_REL	0x80	/* RelFileLocator omitted, same as
									 * previous */

typedef struct XLogRecordDataHeaderShort
{
    uint8		id;				/* XLR_BLOCK_ID_DATA_SHORT */
    uint8		data_length;	/* number of payload bytes */
}			XLogRecordDataHeaderShort;

#define SizeOfXLogRecordDataHeaderShort (sizeof(uint8) * 2)

typedef struct XLogRecordDataHeaderLong
{
    uint8		id;				/* XLR_BLOCK_ID_DATA_LONG */
    /* followed by uint32 data_length, unaligned */
}			XLogRecordDataHeaderLong;

#define SizeOfXLogRecordDataHeaderLong (sizeof(uint8) + sizeof(uint32))

/*
 * The high 4 bits in xl_info may be used freely by rmgr. The
 * XLR_SPECIAL_REL_UPDATE and XLR_CHECK_CONSISTENCY bits can be passed by
 * XLogInsert caller. The rest are set internally by XLogInsert.
 */
#define XLR_INFO_MASK			0x0F
#define XLR_RMGR_INFO_MASK		0xF0


#define XLR_MAX_BLOCK_ID			32

#define XLR_BLOCK_ID_DATA_SHORT		255
#define XLR_BLOCK_ID_DATA_LONG		254
#define XLR_BLOCK_ID_ORIGIN			253
#define XLR_BLOCK_ID_TOPLEVEL_XID	252

typedef struct
{
    /* Is this block ref in use? */
    bool		in_use;

    /* Identify the block this refers to */
//    RelFileLocator rlocator;
//    ForkNumber	forknum;
    BlockNumber blkno;

    /* Prefetching workspace. */
    Buffer		prefetch_buffer;

    /* copy of the fork_flags field from the XLogRecordBlockHeader */
    uint8		flags;

    /* Information on full-page image, if any */
    bool		has_image;		/* has image, even for consistency checking */
    bool		apply_image;	/* has image that should be restored */
    char	   *bkp_image;
    uint16		hole_offset;
    uint16		hole_length;
    uint16		bimg_len;
    uint8		bimg_info;

    /* Buffer holding the rmgr-specific data associated with this block */
    bool		has_data;
    char	   *data;
    uint16		data_len;
    uint16		data_bufsz;
} DecodedBkpBlock;

typedef struct DecodedXLogRecord
{
    /* Private member used for resource management. */
    size_t		size;			/* total size of decoded record */
    bool		oversized;		/* outside the regular decode buffer? */
    struct DecodedXLogRecord *next; /* decoded record queue link */

    /* Public members. */
    XLogRecPtr	lsn;			/* location */
    XLogRecPtr	next_lsn;		/* location of next record */
    XLogRecord	header;			/* header */
    RepOriginId record_origin;
    TransactionId toplevel_xid; /* XID of top-level transaction */
    char	   *main_data;		/* record's main data portion */
    uint32		main_data_len;	/* main data portion's length */
    int			max_block_id;	/* highest block_id in use (-1 if none) */
    DecodedBkpBlock blocks[FLEXIBLE_ARRAY_MEMBER];
} DecodedXLogRecord;

typedef struct WALSegmentContext
{
    char		ws_dir[1024];
    int			ws_segsize;
} WALSegmentContext;
typedef struct WALOpenSegment
{
    int			ws_file;		/* segment file descriptor */
    XLogSegNo	ws_segno;		/* segment number */
    TimeLineID	ws_tli;			/* timeline ID of the currently open file */
} WALOpenSegment;

struct FileHandle;

struct Access;
struct XLogReaderState
{
    struct Access* routine;

    uint64		system_identifier;

    void	   *private_data;

    XLogRecPtr	ReadRecPtr;		/* start of last record read */
    XLogRecPtr	EndRecPtr;		/* end+1 of last record read */

    XLogRecPtr	abortedRecPtr;
    XLogRecPtr	missingContrecPtr;
    XLogRecPtr	overwrittenRecPtr;

    XLogRecPtr	DecodeRecPtr;	/* start of last record decoded */
    XLogRecPtr	NextRecPtr;		/* end+1 of last record decoded */
    XLogRecPtr	PrevRecPtr;		/* start of previous record decoded */

    DecodedXLogRecord *record;

    char	   *decode_buffer;
    size_t		decode_buffer_size;
    bool		free_decode_buffer; /* need to free? */
    char	   *decode_buffer_head; /* data is read from the head */
    char	   *decode_buffer_tail; /* new data is written at the tail */

    DecodedXLogRecord *decode_queue_head;	/* oldest decoded record */
    DecodedXLogRecord *decode_queue_tail;	/* newest decoded record */

    char	   *readBuf;
    uint32		readLen;

    WALSegmentContext segcxt;
    WALOpenSegment seg;
    uint32		segoff;

    XLogRecPtr	latestPagePtr;
    TimeLineID	latestPageTLI;

    /* beginning of the WAL record being read. */
    XLogRecPtr	currRecPtr;
    /* timeline to read it from, 0 if a lookup is required */
    TimeLineID	currTLI;

    XLogRecPtr	currTLIValidUntil;

    TimeLineID	nextTLI;

    char	   *readRecordBuf;
    uint32		readRecordBufSize;

    /* Buffer to hold error message */
    char	   *errormsg_buf;
    bool		errormsg_deferred;

    bool		nonblocking;
};

typedef struct XLogReaderState XLogReaderState;
XLogRecPtr xlog_find_next_record(XLogReaderState *state, XLogRecPtr RecPtr);


#define InvalidXLogRecPtr	0
#define XLogRecPtrIsInvalid(r)	((r) == InvalidXLogRecPtr)
#define MCXT_ALLOC_NO_OOM 0x02
#define MCXT_ALLOC_ZERO 0x04

#endif //PG_HEXRETRO_PG_WAL_H
