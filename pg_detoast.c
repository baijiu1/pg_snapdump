//
// Created by 白杰 on 2026/5/20.
//

#include "pg_detoast.h"

struct varlena *
pg_detoast_datum_packed(ScanContext *ctx, struct varlena *datum)
{
    if (VARATT_IS_COMPRESSED(datum) || VARATT_IS_EXTERNAL(datum)) {
        printf(" detoast_attr ");
        return detoast_attr(ctx, datum);
    } else {
        return datum;
    }
}




static struct varlena *
toast_fetch_datum(ScanContext *ctx, struct varlena *attr)
{
    struct varlena *result;
    struct varatt_external toast_pointer;
    int32		attrsize;

    if (!VARATT_IS_EXTERNAL_ONDISK(attr))
        printf("toast_fetch_datum shouldn't be called for non-ondisk datums");

    /* Must copy to access aligned fields */
    VARATT_EXTERNAL_GET_POINTER(toast_pointer, attr);

    attrsize = VARATT_EXTERNAL_GET_EXTSIZE(toast_pointer);

    result = (struct varlena *) malloc(attrsize + VARHDRSZ);

    if (VARATT_EXTERNAL_IS_COMPRESSED(toast_pointer))
        SET_VARSIZE_COMPRESSED(result, attrsize + VARHDRSZ);
    else
        SET_VARSIZE(result, attrsize + VARHDRSZ);

    if (attrsize == 0)
        return result;			/* Probably shouldn't happen, but just in
								 * case. */

    char *data = fetch_toast_value(ctx,
                                   toast_pointer.va_toastrelid,
                                   toast_pointer.va_valueid,
                                   attrsize);

    memcpy(VARDATA(result), data, attrsize);

    free(data);

    return result;
}

char *fetch_toast_value(ScanContext *ctx,
                        Oid toast_rel_oid,
                        Oid chunk_id,
                        int32 rawsize)
{
    toastChunkList *list =
            toast_cache_get_list(ctx->toast_cache, chunk_id);

    if (!list || list->count == 0) {
        return NULL;
    }

    int len;
    char *data = toast_assemble(list, &len);

    // 可选校验
    if (rawsize > 0 && len != rawsize) {
        // printf("warning: toast size mismatch %d vs %d\n", len, rawsize);
    }

    return data;
}

char* toast_assemble(toastChunkList *list, int *out_len) {
    if (list->count == 0) return NULL;

    int max_seq = list->max_seq;

    // 建一个 seq → chunk 的映射
    toastChunk **map = (toastChunk**) calloc(max_seq + 1, sizeof(toastChunk*));

    for (int i = 0; i < list->count; i++) {
        map[list->chunks[i].seq] = &list->chunks[i];
    }

    // 校验完整性
    for (int i = 0; i <= max_seq; i++) {
        if (map[i] == NULL) {
            free(map);
            return NULL; // 数据缺失
        }
    }

    // 计算总长度
    int total = 0;
    for (int i = 0; i <= max_seq; i++) {
        total += map[i]->len;
    }

    char *buf = (char*) malloc(total);
    int pos = 0;

    for (int i = 0; i <= max_seq; i++) {
        memcpy(buf + pos, map[i]->data, map[i]->len);
        pos += map[i]->len;
    }

    free(map);
    *out_len = total;
    return buf;
}

toastChunkList* toast_cache_get_list(toastCache *cache, Oid chunk_id) {
    for (int i = 0; i < cache->count; i++) {
        if (cache->lists[i].chunk_id == chunk_id) {
            return &cache->lists[i];
        }
    }

    // 不存在 → 新建
    if (cache->count == cache->capacity) {
        cache->capacity *= 2;
        cache->lists = (toastChunkList *) realloc(cache->lists,
                                                  sizeof(toastChunkList) * cache->capacity);
    }

    toastChunkList *list = &cache->lists[cache->count++];
    list->chunk_id = chunk_id;
    list->count = 0;
    list->capacity = 8;
    list->max_seq = -1;
    list->chunks = (toastChunk *) malloc(sizeof(toastChunk) * list->capacity);

    return list;
}

struct varlena *
detoast_attr(ScanContext *ctx, struct varlena *attr)
{
    if (VARATT_IS_EXTERNAL_ONDISK(attr))
    {
        printf(" ON DISK! \n");
        /*
         * This is an externally stored datum --- fetch it back from there
         */
        attr = toast_fetch_datum(ctx, attr);
        /* If it's compressed, decompress it */
        if (VARATT_IS_COMPRESSED(attr))
        {
            struct varlena *tmp = attr;

            attr = toast_decompress_datum(tmp);
            free(tmp);
        }
    }
    else if (VARATT_IS_EXTERNAL_INDIRECT(attr))
    {
        printf(" IN DIRECT! \n");
        /*
         * This is an indirect pointer --- dereference it
         */
        struct varatt_indirect redirect;

        VARATT_EXTERNAL_GET_POINTER(redirect, attr);
        attr = (struct varlena *) redirect.pointer;

        /* nested indirect Datums aren't allowed */
        Assert(!VARATT_IS_EXTERNAL_INDIRECT(attr));

        /* recurse in case value is still extended in some other way */
        attr = detoast_attr(ctx, attr);

        /* if it isn't, we'd better copy it */
        if (attr == (struct varlena *) redirect.pointer)
        {
            struct varlena *result;

            result = (struct varlena *) malloc(VARSIZE_ANY(attr));
            memcpy(result, attr, VARSIZE_ANY(attr));
            attr = result;
        }
    }
    else if (VARATT_IS_EXTERNAL_EXPANDED(attr))
    {
        printf(" EXPANDED! \n");
        /*
         * This is an expanded-object pointer --- get flat format
         */
//        attr = detoast_external_attr(attr);
        /* flatteners are not allowed to produce compressed/short output */
        Assert(!VARATT_IS_EXTENDED(attr));
    }
    else if (VARATT_IS_COMPRESSED(attr))
    {
        printf(" COMPRESSED! \n");
        /*
         * This is a compressed value inside of the main tuple
         */
        attr = toast_decompress_datum(attr);
    }
    else if (VARATT_IS_SHORT(attr))
    {
        /*
         * This is a short-header varlena --- convert to 4-byte header format
         */
        Size		data_size = VARSIZE_SHORT(attr) - VARHDRSZ_SHORT;
        Size		new_size = data_size + VARHDRSZ;
        struct varlena *new_attr;

        new_attr = (struct varlena *) malloc(new_size);
        SET_VARSIZE(new_attr, new_size);
        memcpy(VARDATA(new_attr), VARDATA_SHORT(attr), data_size);
        attr = new_attr;
    }

    return attr;
}

static struct varlena *
toast_decompress_datum(struct varlena *attr)
{
    ToastCompressionId cmid;

    Assert(VARATT_IS_COMPRESSED(attr));

    /*
     * Fetch the compression method id stored in the compression header and
     * decompress the data using the appropriate decompression routine.
     */
    cmid = (ToastCompressionId) TOAST_COMPRESS_METHOD(attr);
    switch (cmid)
    {
        case TOAST_PGLZ_COMPRESSION_ID:
            return pglz_decompress_datum(attr);
        case TOAST_LZ4_COMPRESSION_ID:
            return lz4_decompress_datum(attr);
        default:
            printf("invalid compression method id %d", cmid);
//            elog(ERROR, "invalid compression method id %d", cmid);
            return NULL;		/* keep compiler quiet */
    }
}

struct varlena *
pglz_decompress_datum(const struct varlena *value)
{
    struct varlena *result;
    int32		rawsize;

    /* allocate memory for the uncompressed data */
    result = (struct varlena *) malloc(VARDATA_COMPRESSED_GET_EXTSIZE(value) + VARHDRSZ);

    /* decompress the data */
    rawsize = pglz_decompress((char *) value + VARHDRSZ_COMPRESSED,
                              VARSIZE(value) - VARHDRSZ_COMPRESSED,
                              VARDATA(result),
                              VARDATA_COMPRESSED_GET_EXTSIZE(value), true);
    if (rawsize < 0)
        printf("compressed pglz data is corrupt");

    SET_VARSIZE(result, rawsize + VARHDRSZ);

    return result;
}

struct varlena *
lz4_decompress_datum(const struct varlena *value)
{
#ifndef USE_LZ4
    NO_LZ4_SUPPORT();
    return NULL;				/* keep compiler quiet */
#else
    int32		rawsize;
	struct varlena *result;

	/* allocate memory for the uncompressed data */
	result = (struct varlena *) malloc(VARDATA_COMPRESSED_GET_EXTSIZE(value) + VARHDRSZ);

	/* decompress the data */
	rawsize = LZ4_decompress_safe((char *) value + VARHDRSZ_COMPRESSED,
								  VARDATA(result),
								  VARSIZE(value) - VARHDRSZ_COMPRESSED,
								  VARDATA_COMPRESSED_GET_EXTSIZE(value));
	if (rawsize < 0)
//		ereport(ERROR,
//				(errcode(ERRCODE_DATA_CORRUPTED),
//				 errmsg_internal("compressed lz4 data is corrupt")));


	SET_VARSIZE(result, rawsize + VARHDRSZ);

	return result;
#endif
}