//
// Created by 白杰 on 2025/8/9.
//

#include "pg_integer.h"

void decode_smallint(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "smallint: start offset = %d, end offset = %zu, sizeof(int16) = %lu, column sequence = %d", startOffset, off, sizeof(int16), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (off - sizeof(int16)), off);
    CopyAppendFmt("%d", *(int16 *) buffer);
    delete[] buffer;
};

void decode_int(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "lint: start offset = %d, end offset = %zu, sizeof(int32) = %lu, column sequence = %d", startOffset, off, sizeof(int32), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (off - sizeof(int32)), off);
    CopyAppendFmt("%d", *(int32 *) buffer);
    delete[] buffer;
};

void decode_uint(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "uint: start offset = %d, end offset = %zu, sizeof(int32) = %lu, column sequence = %d", startOffset, off, sizeof(int32), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset, off);
    CopyAppendFmt("%d", *(int32 *) buffer);
    delete[] buffer;
};

void decode_bigint(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "bigint: start offset = %d, end offset = %zu, sizeof(int64) = %lu, column sequence = %d", startOffset, off, sizeof(int64), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (off - sizeof(int64)) , off);
    CopyAppendFmt(INT64_FORMAT, *(int64 *) buffer);
    delete[] buffer;
};

void decode_oid(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_float4(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "float4: start offset = %d, end offset = %zu, sizeof(int32) = %lu, column sequence = %d", startOffset, off, sizeof(int32), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (off - sizeof(int32)), off);
    CopyAppendFmt("%.12f", *(float *) buffer);
    delete[] buffer;
};

void decode_float8(CtidNode* tuple, int colSeq, uint32_t * offset){
    uint32 startOffset = *offset;
    size_t off = computeAttAlign(*offset, tuple, colSeq);
    char* buffer = new char [512];
    LOG(LOG_LEVEL_DEBUG, "float8: start offset = %d, end offset = %zu, sizeof(int64) = %lu, column sequence = %d", startOffset, off, sizeof(int64), colSeq);
    memcpy(buffer, tuple->tuple.cache_data + startOffset + (off - sizeof(int64)), off);
    CopyAppendFmt("%.12lf", *(double *) buffer);
    delete[] buffer;
};

void decode_numeric(CtidNode* tuple, int colSeq, uint32_t * offset){
    unsigned char* buffer = (unsigned char*)tuple->tuple.cache_data + *offset;
    LOG(LOG_LEVEL_DEBUG, "numeric: start offset = %d, column sequence = %d", *offset, colSeq);
    extract_data(buffer, &CopyAppendNumeric, tuple, colSeq, offset);
};

void decode_macaddr(CtidNode* tuple, int colSeq, uint32_t * offset){
//    unsigned char macaddr[6];
//    size_t off = computeAttAlign(*offset, tuple, colSeq);
//    const char *new_buffer = (const char *) INTALIGN(buffer);
//    unsigned int delta = (unsigned int) ((uintptr_t) new_buffer - (uintptr_t) buffer);
//
//    memcpy(macaddr, tuple->tuple.cache_data, sizeof(macaddr));
//    CopyAppendFmt("%02x:%02x:%02x:%02x:%02x:%02x",
//                  macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]
//    );
};

void decode_int4range(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_numrange(CtidNode* tuple, int colSeq, uint32_t * offset){

};