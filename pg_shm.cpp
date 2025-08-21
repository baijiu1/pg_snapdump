//
// Created by 白杰 on 2025/8/19.
//

#include "pg_shm.h"

// 创建共享内存
SharedCtidNodeVector* create_shared_vector(size_t capacity) {
    size_t bytes = sizeof(SharedCtidNodeVector) + capacity * sizeof(CtidShardNode);
    SharedCtidNodeVector *vec = (SharedCtidNodeVector*) mmap(NULL, bytes,
                                                             PROT_READ | PROT_WRITE,
                                                             MAP_SHARED | MAP_ANONYMOUS,
                                                             -1, 0);
    if (vec == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    vec->size = 0;
    vec->capacity = capacity;
    return vec;
}
//
//// push_back
//void sharedvec_push_back(SharedCtidNodeVector *vec, CtidNode node) {
//    if (vec->size < vec->capacity) {
//        vec->data[vec->size++].tuple = node;
//    } else {
//        fprintf(stderr, "vector full, cannot push\n");
//    }
//}