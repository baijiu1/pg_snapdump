//
// Created by 白杰 on 2025/8/10.
//

#ifndef PG_SNAPDUMP_PG_COMMON_H
#define PG_SNAPDUMP_PG_COMMON_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include "string"
#include "vector"
#include "pg_basic.h"
#include <cassert>
#include <sys/mman.h>
#include <algorithm>
#include <cstring>

using namespace std;


#define LocalTransactionId unsigned int
typedef uint32_t TransactionId;
#define MAX_PAGE_COUNT 131072
#define NAMEDATALEN 64


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



typedef struct {
    vector<string> colName;
    vector<uint8_t> colAttNum;
    vector<uint32_t> colType;
    vector<int> colAttlen;
    vector<char> colAttalign;
    vector<uint8_t> bitMaps;
} ColAttribute;

typedef ColAttribute* ColAttributes;

typedef struct nameData
{
    char		data[NAMEDATALEN];
} NameData;

typedef uint16_t OffsetNumber;
typedef uint32_t BlockNumber;
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
typedef unsigned char bits8;
typedef uint32_t TransactionId;
typedef uint32_t CommandId;

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
//    unsigned char* t_bits1;

    /* MORE DATA FOLLOWS AT END OF STRUCT */
} HeapTupleHeaderData;
typedef HeapTupleHeader* HeapTupleHeaders;

// ctid链表
typedef struct chaseCtidList {
    ItemPointerData self_ctid;  // 自身标识（页号+偏移）
    uint32_t tuple_nattrs;
    vector<uint8_t> null_bit_map;
    uint32_t xmin;          // 插入该元组的事务ID
    uint32_t xmax;          // 删除该元组的事务ID（即更新它的事务）
    size_t tuple_block_id;  // 元组所在的页号
    uint16_t tuple_offset;    // 元组的偏移量，相对于页的， 为0时代表cache_data有数据，不为0时代表指向了其他页不缓存数据，最后合并查找
    uint16_t tuple_length;    // 元组长度
    char* cache_data;       // 缓存的数据
    vector<string> column_name;
    vector<uint32_t> column_type_id;
    vector<int> colAttlen;
    vector<char> colAttalign;
} chaseCtidList;

typedef struct CtidNode {
    struct chaseCtidList tuple;
    struct CtidNode *next; // 指向下一个版本
} CtidNode;




// BST node for ctid
typedef struct BSTNode {
    size_t block_id;
    uint16_t offset;
    CtidNode* value;       // 版本链头
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;




typedef struct BSTCtidNode {
    ItemPointerData key;
    CtidNode* value;       // 版本链头
    struct BSTCtidNode* left;
    struct BSTCtidNode* right;
    struct BSTCtidNode* parent;
} BSTCtidNode;

typedef struct BSTTree {
    BSTCtidNode *root;
    BSTCtidNode *nil;   // 全局哨兵叶子节点（黑色）
} BSTTree;

size_t computeAttAlign(uint32_t&, CtidNode*&, int);

char* CopyAppend(const char *str);

int CopyAppendEncode(const unsigned char *str, int orig_len);

#define MaxAllocSize	((int) 0x3fffffff) /* 1 gigabyte - 1 */
#define CopyAppendFmt(fmt, ...) do { \
	  char __copy_format_buff[512]; \
	  snprintf(__copy_format_buff, sizeof(__copy_format_buff), fmt, ##__VA_ARGS__); \
	  CopyAppend(__copy_format_buff); \
  } while(0)
#define MAXDATELEN 128

#ifdef __cplusplus
extern "C" {
#endif

#if PG_VERSION_NUM >= 120000
int32 pglz_decompress(const char *source, int32 slen, char *dest,
                int32 rawsize, bool check_complete);
#else
int32 pglz_decompress(const char *source, int32 slen, char *dest,
                      int32 rawsize);
#endif

#ifdef __cplusplus
}
#endif
int
CopyAppendNumeric(const unsigned char *buffer, int num_size);
int CopyAppendNumericValue(const char *buffer, int num_size);

typedef enum blockSwitches
{
    BLOCK_ABSOLUTE = 0x00000001,		/* -a: Absolute(vs Relative) addressing */
    BLOCK_BINARY = 0x00000002,			/* -b: Binary dump of block */
    BLOCK_FORMAT = 0x00000004,			/* -f: Formatted dump of blocks / control file */
    BLOCK_FORCED = 0x00000008,			/* -S: Block size forced */
    BLOCK_NO_INTR = 0x00000010,			/* -d: Dump straight blocks */
    BLOCK_RANGE = 0x00000020,			/* -R: Specific block range to dump */
    BLOCK_CHECKSUMS = 0x00000040,		/* -k: verify block checksums */
    BLOCK_DECODE = 0x00000080,			/* -D: Try to decode tuples */
    BLOCK_DECODE_TOAST = 0x00000100,	/* -t: Try to decode TOAST values */
    BLOCK_IGNORE_OLD = 0x00000200		/* -o: Decode old values */
} blockSwitches;

#define VARATT_EXTERNAL_GET_POINTER(toast_pointer, attr) \
do { \
	varattrib_1b_e *attre = (varattrib_1b_e *) (attr); \
	Assert(VARATT_IS_EXTERNAL(attre)); \
	Assert(VARSIZE_EXTERNAL(attre) == sizeof(toast_pointer) + VARHDRSZ_EXTERNAL); \
	memcpy(&(toast_pointer), VARDATA_EXTERNAL(attre), sizeof(toast_pointer)); \
} while (0)

int extract_data(const unsigned char *buffer, int (*parse_value)(const unsigned char *, int), CtidNode*, int, uint32_t *);

template<typename T, typename... Vectors>
void sort_with_sync(std::vector<T>& key, Vectors&... others) {
    size_t n = key.size();
    // 检查所有 vector 大小一致（可加断言或异常处理）

    // 生成索引数组
    std::vector<size_t> idx(n);
    for (size_t i = 0; i < n; ++i) idx[i] = i;

    // 根据 key 排序索引
    std::sort(idx.begin(), idx.end(),
              [&](size_t a, size_t b) { return key[a] < key[b]; });

    // 使用lambda展开包，针对每个 vector 做元素重新排列
    auto reorder = [&](auto& vec) {
        std::vector<typename std::decay<decltype(vec[0])>::type> tmp;
        tmp.reserve(n);
        for (size_t i : idx) tmp.push_back(vec[i]);
        vec = std::move(tmp);
    };

    // 先调整 key 自身
    reorder(key);
    // 递归调整其它 vectors
    (reorder(others), ...);
}

#endif //PG_SNAPDUMP_PG_COMMON_H
