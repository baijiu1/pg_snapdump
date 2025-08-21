//
// Created by 白杰 on 2025/8/19.
//

#ifndef PG_SNAPDUMP_PG_SHM_H
#define PG_SNAPDUMP_PG_SHM_H
#include "cstdio"
#include "cstdint"
#include "cstdlib"
#include "pg_common.h"

using namespace std;

typedef struct chaseCtidShardList {
    size_t cache_data_offset;
    size_t column_name_offset;
    size_t column_type_id_offset;
    size_t col_attlen_offset;
    size_t col_attalign_offset;
    size_t null_bit_map_offset;
    size_t tuple_block_id;  // 元组所在的页号
    ItemPointerData self_ctid;  // 自身标识（页号+偏移）
    uint32_t tuple_nattrs;
    uint32_t xmin;          // 插入该元组的事务ID
    uint32_t xmax;          // 删除该元组的事务ID（即更新它的事务）
    uint16_t tuple_offset;    // 元组的偏移量，相对于页的， 为0时代表cache_data有数据，不为0时代表指向了其他页不缓存数据，最后合并查找
    uint16_t tuple_length;    // 元组长度
} chaseCtidShardList;

typedef struct CtidShardNode {
    chaseCtidShardList tuple;
    int next_index;     // -1为header节点，-2为tail节点
} CtidShardNode;

typedef struct {
    size_t size;
    size_t capacity;
    CtidShardNode data[];
} SharedCtidNodeVector;

SharedCtidNodeVector* create_shared_vector(size_t capacity);

#endif //PG_SNAPDUMP_PG_SHM_H
