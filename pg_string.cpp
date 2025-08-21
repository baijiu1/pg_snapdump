//
// Created by 白杰 on 2025/8/9.
//

#include "pg_string.h"

void decode_bool(CtidNode* tuple, int colSeq, uint32_t * offset){
    char* buffer = tuple->tuple.cache_data + *offset;
    CopyAppend(*(bool *) buffer ? "t" : "f");
    *offset += 1;
};

void decode_uuid(CtidNode* tuple, int colSeq, uint32_t * offset){
    unsigned char uuid[16];
    memcpy(uuid, tuple->tuple.cache_data + *offset, sizeof(uuid));
    CopyAppendFmt("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                  uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
                  uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
    );
};

void decode_string(CtidNode* tuple, int colSeq, uint32_t * offset){
    unsigned char* buffer = (unsigned char*)tuple->tuple.cache_data + *offset;
    extract_data(buffer, &CopyAppendEncode, tuple, colSeq, offset);
};

void decode_char(CtidNode* tuple, int colSeq, uint32_t * offset){
    unsigned char *buffer = (unsigned char *)tuple->tuple.cache_data + *offset;
    CopyAppendEncode(buffer, 1);
    *offset += 1;
};

void decode_name(CtidNode* tuple, int colSeq, uint32_t * offset){
    unsigned char* buffer = (unsigned char *)tuple->tuple.cache_data + *offset;
    CopyAppendEncode(buffer, strnlen((char*)buffer, NAMEDATALEN));
    *offset += NAMEDATALEN;
};

void decode_json(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_jsonb(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_inet(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_cidr(CtidNode* tuple, int colSeq, uint32_t * offset){

};

void decode_hstore(CtidNode* tuple, int colSeq, uint32_t * offset){

};