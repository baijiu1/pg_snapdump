//
// Created by 白杰 on 2024/12/16.
//

#ifndef MYSQL_REPLICATER_PAGE_H
#define MYSQL_REPLICATER_PAGE_H

#include "pg_class.h"
#include "pg_attribute.h"
#include "pg_data.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <sys/mman.h>
#include <cassert>
#include <map>
#include <unordered_map>

using namespace std;


#define PG

#ifdef PG
#define HeapPageHeaderSize 24
#elif OPENGAUSS
#define HeapPageHeaderSize 40
#endif



const long osPagesize = sysconf(_SC_PAGE_SIZE);

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

/*
 * lp_flags has these possible states.	An UNUSED line pointer is available
 * for immediate re-use, the other states are not.
 */
#define LP_UNUSED 0   /* unused (should always have lp_len=0) */
#define LP_NORMAL 1   /* used (should always have lp_len>0) */
#define LP_REDIRECT 2 /* HOT redirect (should have lp_len=0) */
#define LP_DEAD 3     /* dead, may or may not have storage */

#define LP_INDEX_FROZEN 2 /* index tuple's xmin is frozen (used for multi-version btree index only) */

/*
pd_flags
*/
#define PD_HAS_FREE_LINES	0x0001	/* are there any unused line pointers? */
#define PD_PAGE_FULL		0x0002	/* not enough free space for new tuple? */
#define PD_ALL_VISIBLE		0x0004	/* all tuples on page are visible to
									 * everyone */

#define PD_VALID_FLAG_BITS	0x0007	/* OR of all valid pd_flags bits */



#define LWLockMode int
#define LOCKMODE int
#define BlockNumber unsigned int
//#define Oid unsigned int
#define ForkNumber int
//#define bool unsigned char


typedef struct {
    uint32_t xlogid;  /* high bits */
    uint32_t xrecoff; /* low bits */
} PageXLogRecPtr;

typedef struct ItemIdData {
    uint16_t lp_off; /* offset to tuple (from start of page) */
    uint16_t lp_flags;     /* state of item pointer, see below */
    uint16_t lp_len;      /* byte length of tuple */
} ItemIdData;

typedef struct PdItemIdData
{
    unsigned	lp_off:15,		/* offset to tuple (from start of page) */
    lp_flags:2,		/* state of line pointer, see below */
    lp_len:15;		/* byte length of tuple */
} PdItemIdData;

typedef PdItemIdData *PdItemId;

typedef ItemIdData* ItemId;
typedef uint16_t LocationIndex;


typedef uint32_t ShortTransactionId;












typedef struct {
    /* XXX LSN is member of *any* block, not only page-organized ones */
    PageXLogRecPtr pd_lsn;    /* LSN: next byte after last byte of xlog
                               * record for last change to this page */
    uint16_t pd_checksum;       /* checksum */
    uint16_t pd_flags;          /* flag bits, see below */
    LocationIndex pd_lower;   /* offset to start of free space */
    LocationIndex pd_upper;   /* offset to end of free space */
    LocationIndex pd_special; /* offset to start of special space */
    uint16_t pd_pagesize_version;
    ShortTransactionId pd_prune_xid;           /* oldest prunable XID, or zero if none */
#ifdef OPENGAUSS
    TransactionId pd_xid_base;                 /* base value for transaction IDs on page */
    TransactionId pd_multi_base;               /* base value for multixact IDs on page */
#endif
//    ItemIdData pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* beginning of line pointer array */
    vector<ItemIdData> pd_linp; /* beginning of line pointer array */
    int page_number;

} HeapPageHeaderData;

typedef HeapPageHeaderData *HeapPageHeader;




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
    PdItemIdData	pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* line pointer array */
} PageHeaderData;

typedef PageHeaderData *PageHeader;

typedef struct {
    uint32_t rel_oid;
    uint32_t node_id;
} RelFileNode;

typedef RelFileNode *RelFileNodeData;

typedef struct TableStateData {
    string tableName;
    string tableOID;

} TableStateData;

typedef TableStateData *TableState;

typedef struct HeapTupleData
{
    uint32_t    t_len;   /* length of *t_data */
    ItemPointerData  t_self;   /* SelfItemPointer */
    unsigned int     t_tableOid;  /* table the tuple came from */
#define FIELDNO_HEAPTUPLEDATA_DATA 3
    vector<HeapTupleHeaderData> t_data; /* -> tuple data */
//    HeapTupleHeaders  t_header;   /* -> tuple header */

} HeapTupleData;

typedef HeapTupleData *HeapTuple;


int getHeapHeader();


typedef struct FilenodeMapEntry {
    uint32_t relid;        /* 对应表的 pg_class.oid 值 */
    uint32_t relfilenode;  /* 表的 relfilenode 值 */
} FilenodeMapEntry;








typedef struct NullableDatum
{
#define FIELDNO_NULLABLE_DATUM_DATUM 0
    uintptr_t		value;
#define FIELDNO_NULLABLE_DATUM_ISNULL 1
    bool		isnull;
    /* due to alignment padding this could be used for flags for free */
} NullableDatum;

typedef struct FormExtraData_pg_attribute
{
    NullableDatum attstattarget;
    NullableDatum attoptions;
} FormExtraData_pg_attribute;


//

// 整个表的属性
typedef struct {
    uint32_t oid;
    string tableName;
    uint32_t filenode;
    uint32_t columnNum; // 第几个列
    string columnName;  // 列名
    string columnTypeName;
    uint32_t columnTypeId; // 对应的到pg_type表
    uint32_t columnLength; // 对于固定长度数据类型（如 int、char），它表示字节数。对于变长数据类型（如 text），此字段为 -1。
    uint32_t columnTypeMod; // 数据类型的修饰符，通常用于表示长度限制（如 varchar(50) 中的 50）。
//    vector<uint8_t > columnNdims; // 列的数据维度数。对于普通标量类型，该值为 0；对于数组类型，则是数组的维度数。
//    vector<bool> columnByVal; // 如果列的数据类型按值存储，值为 true，否则为 false。例如，int4 是按值存储的。
//    vector<unsigned char> columnAlign; // 列的对齐方式。常见的值包括：	•	'c'：按字符对齐
    //    'd'：按双字对齐
    //    'i'：按整数对齐
    //    'm'：按更大数据类型对齐
    //    'p'：按指针对齐
    uint8_t columnStorage; // 列的数据存储方式。值包括：
    //    'p'：主存储方式
    //    'm'：压缩存储
    //    'x'：外部存储
    uint8_t columnCompression; // 列的压缩方式。值为 'p'（默认）和 'm'，表示是否进行数据压缩。
    bool columnNotNull; // 是否为 NOT NULL 列。如果列定义了 NOT NULL 约束，则该字段为 true，否则为 false。
    bool columnHasDefault; // 如果列有默认值，则为 true，否则为 false。
//    vector<bool> columnHasmissing; // 是否存在缺失值。如果列有缺失值（用于某些特定列类型，如 JSONB 等），则为 true。
//    vector<unsigned char> columnIdentity; // 列的身份属性类型，表示该列是否是身份列（'a'：自动生成、'd'：显式定义、's'：序列）。
//    vector<unsigned char> columnGenerated; // 列的生成类型。表示列是否是通过 GENERATED 语法定义的自动生成列。
    bool columnIsdropped; // 如果列已被删除，则为 true，否则为 false。此字段通常用于表示逻辑删除的列。
    bool columnIslocal; // 如果列是表的本地列而不是继承列，则为 true，否则为 false。
    uint16_t columnInhcount; // 列继承的次数。如果列是从父表继承的，这个值大于零。
    uint32_t columnCollation; // 列的排序规则 OID，指向 pg_collation 表中的 oid 字段。对于字符类型列，指定排序规则。
//    vector<uint8_t> columnStattarget; // 表示统计目标，通常与列的分析级别相关，用于 PostgreSQL 的查询优化器。


} tableStructData;

typedef tableStructData* tableStruct;


// 整个表的属性
typedef struct tableConstruct{
    string oid;
    string tableName;
    string relFilenode;
    string filenodePath;
    string tableSize;
    string columnNum; // 第几个列
    string columnName;  // 列名
    string columnTypeName;
    string columnTypeId; // 对应的到pg_type表
    string columnLength; // 对于固定长度数据类型（如 int、char），它表示字节数。对于变长数据类型（如 text），此字段为 -1。
    string columnTypeMod; // 数据类型的修饰符，通常用于表示长度限制（如 varchar(50) 中的 50）。
//    vector<uint8_t > columnNdims; // 列的数据维度数。对于普通标量类型，该值为 0；对于数组类型，则是数组的维度数。
//    vector<bool> columnByVal; // 如果列的数据类型按值存储，值为 true，否则为 false。例如，int4 是按值存储的。
//    vector<unsigned char> columnAlign; // 列的对齐方式。常见的值包括：	•	'c'：按字符对齐
    string columnAttalign;
    //    'd'：按双字对齐
    //    'i'：按整数对齐
    //    'm'：按更大数据类型对齐
    //    'p'：按指针对齐
    string columnStorage; // 列的数据存储方式。值包括：
    //    'p'：主存储方式
    //    'm'：压缩存储
    //    'x'：外部存储
    string columnCompression; // 列的压缩方式。值为 'p'（默认）和 'm'，表示是否进行数据压缩。
    string columnNotNull; // 是否为 NOT NULL 列。如果列定义了 NOT NULL 约束，则该字段为 true，否则为 false。
    string columnHasDefault; // 如果列有默认值，则为 true，否则为 false。
//    vector<bool> columnHasmissing; // 是否存在缺失值。如果列有缺失值（用于某些特定列类型，如 JSONB 等），则为 true。
//    vector<unsigned char> columnIdentity; // 列的身份属性类型，表示该列是否是身份列（'a'：自动生成、'd'：显式定义、's'：序列）。
//    vector<unsigned char> columnGenerated; // 列的生成类型。表示列是否是通过 GENERATED 语法定义的自动生成列。
    string columnIsdropped; // 如果列已被删除，则为 true，否则为 false。此字段通常用于表示逻辑删除的列。
    string columnIslocal; // 如果列是表的本地列而不是继承列，则为 true，否则为 false。
    string columnInhcount; // 列继承的次数。如果列是从父表继承的，这个值大于零。
    string columnCollation; // 列的排序规则 OID，指向 pg_collation 表中的 oid 字段。对于字符类型列，指定排序规则。
//    vector<uint8_t> columnStattarget; // 表示统计目标，通常与列的分析级别相关，用于 PostgreSQL 的查询优化器。
    string columnRelnatts; // 表中所有的列数
    uint16_t columnNatts;  // 表中真实列数，不带后面add colum或drop column的
    uint8_t columnContextIsNull = 0; // 标识该字段内容是否为null
    string columnCount;

} tableConstructData;

//typedef tableConstructData* tableConstruct;

// 每行数据填充到t_data里，每行数据的t_xmin和t_xmax填充，一一对应
typedef struct finalTableStruct{
    vector<string > t_data;
    uint32_t t_xmin;
    uint32_t t_xmax;
    struct finalTableStruct *next;

} finalTableData;

// 追踪ctid链，最多追 maxDepth 层
typedef struct TupleVersion {
    uint32_t blockId;
    uint16_t offset;
    std::vector<uint8_t> rawTuple;
    ItemIdData itemId;
} TupleVersion;

//void extractTuple(std::vector<uint8_t>&, const ItemIdData&, const std::vector<uint8_t>&);
//bool readLogicalPage(int, size_t, std::vector<uint8_t>&);
//std::vector<TupleVersion> chaseCTIDChain(int, uint32_t, uint16_t, int);


// ctid链表
typedef struct chaseTupleList {
    ItemPointerData self_ctid;  // 自身标识（页号+偏移）
    uint32_t tuple_nattrs;
//    vector<uint8_t> null_bit_map;
    uint32_t xmin;          // 插入该元组的事务ID
    uint32_t xmax;          // 删除该元组的事务ID（即更新它的事务）
    char *data;             // 模拟元组内容
    size_t tuple_block_id;  // 元组所在的页号
    size_t tuple_offset;    // 元组的偏移量，相对于页的， 为0时代表cache_data有数据，不为0时代表指向了其他页不缓存数据，最后合并查找
    size_t tuple_length;    // 元组长度
    string cache_data;       // 缓存的数据
    string null_bit_map;
} chaseTupleList;

typedef struct TupleNode {
    struct chaseTupleList tuple;
    struct TupleNode *next; // 指向下一个版本
} TupleNode;

// 如果页数过多，那先记录相对的Offset和length，维护一个map
// offset/length/ctid->blockId

TupleNode* reverseList(TupleNode* head);
void reverseAllChains(std::vector<TupleNode*>& chains);
//bool ItemPointerDataEqual(const ItemPointerData&, const ItemPointerData&);
off_t fetchFileTotalNum(int fd, int* pageTotalNum);
bool fetchPage(int fd, int pageNum, char* outPage);
bool fetchPageData(char* pageData, int thisPageNum, const char *, unsigned int*, off_t, int);
bool processHalfPage(char* halfPageData, int pageNum, const char *, unsigned int*, int);




//TupleNode* build_version_chain(TupleNode **tuples, int count, ItemPointerData newest_ctid);
//void append_to_chain(const chaseTupleList);
//void print_all_ctid_chains();

/*
 * ItemIdIsValid
 *		True iff item identifier is valid.
 *		This is a pretty weak test, probably useful only in Asserts.
 */
#define ItemIdIsValid(itemId)	PointerIsValid(itemId)

/*
 * ItemIdIsUsed
 *		True iff item identifier is in use.
 */
#define ItemIdIsUsed(itemId) \
	((itemId).lp_flags != LP_UNUSED)

/*
 * ItemIdIsNormal
 *		True iff item identifier is in state NORMAL.
 */
#define ItemIdIsNormal(itemId) \
	((itemId).lp_flags == LP_NORMAL)

/*
 * ItemIdIsRedirected
 *		True iff item identifier is in state REDIRECT.
 */
#define ItemIdIsRedirected(itemId) \
	((itemId).lp_flags == LP_REDIRECT)

/*
 * ItemIdIsDead
 *		True iff item identifier is in state DEAD.
 */
#define ItemIdIsDead(itemId) \
	((itemId).lp_flags == LP_DEAD)

/*
 * ItemIdHasStorage
 *		True iff item identifier has associated storage.
 */
#define ItemIdHasStorage(itemId) \
	((itemId).lp_len != 0)





#endif //MYSQL_REPLICATER_PAGE_H
