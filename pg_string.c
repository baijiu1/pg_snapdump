//
// Created by 白杰 on 2026/5/20.
//

#include "pg_string.h"

Datum ns_decode_str(char *ptr)
{
    return (Datum)ptr;
}

Datum ns_decode_char(char *ptr)
{

    return (Datum)(*ptr);
}

Datum ns_decode_name(char *ptr)
{
    return (Datum)(ptr);
}



Datum ns_decode_jsonb(char *ptr)
{
    return (Datum)(ptr);
}

Datum ns_decode_uuid(char *ptr)
{

    return (Datum)ptr;
}

Datum ns_decode_inet(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_cidr(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

Datum ns_decode_macaddr(char *ptr)
{

    return (Datum)ptr;
}

Datum ns_decode_hstore(char *ptr)
{
    int32_t varlen = *(int32_t *)ptr;

    return (Datum)(ptr + 4);
}

char* ByteaDatumToCString(ScanContext *ctx, Datum d)
{
    struct varlena *v = (struct varlena *) PG_DETOAST_DATUM(ctx, d);

    char *data;
    int len;

    if (VARATT_IS_SHORT(v)) {
        data = VARDATA_SHORT(v);
        len  = VARSIZE_SHORT(v) - VARHDRSZ_SHORT;
    } else {
        data = VARDATA(v);
        len  = VARSIZE(v) - VARHDRSZ;
    }

    // 转 hex：\x...
    char *buf = (char*) malloc(len * 2 + 3);
    char *p = buf;

    *p++ = '\\';
    *p++ = 'x';

    for (int i = 0; i < len; i++) {
        sprintf(p, "%02x", (unsigned char)data[i]);
        p += 2;
    }

    *p = '\0';
    return buf;
}

Jsonb*
DatumGetJsonb(ScanContext *ctx, Datum X)
{
    struct varlena *v = PG_DETOAST_DATUM(ctx, X);

    return (Jsonb *) v;
}

JsonbIterator* JsonbIteratorInit(JsonbContainer* container) {
    return iteratorFromContainer(container, NULL);
};

static JsonbIterator *
iteratorFromContainer(JsonbContainer *container, JsonbIterator *parent)
{
    JsonbIterator *it;

    it = (JsonbIterator*) malloc(sizeof(JsonbIterator));
    it->container = container;
    it->parent = parent;
    it->nElems = JsonContainerSize(container);

    /* Array starts just after header */
    it->entries = container->children;

    switch (container->header & (JB_FARRAY | JB_FOBJECT))
    {
        case JB_FARRAY:
            it->dataProper =
                    (char *) it->entries + it->nElems * sizeof(JEntry);
            it->isScalar = JsonContainerIsScalar(container);
            /* This is either a "raw scalar", or an array */
            Assert(!it->isScalar || it->nElems == 1);

            it->state = JBI_ARRAY_START;
            break;

        case JB_FOBJECT:
            it->dataProper =
                    (char *) it->entries + it->nElems * sizeof(JEntry) * 2;
            it->state = JBI_OBJECT_START;
            break;

        default:
            printf("unknown type of jsonb container\n");
    }

    return it;
}

static char *
JsonbToCStringWorker(StringInfo out, JsonbContainer *container, int estimated_len, bool indent) {
    char* dataProper;
    JsonbIterator *it;
    JsonbValue	v;
    JsonbIteratorToken type = WJB_DONE;
    int			level = 0;
    bool		first = true;
    int			ispaces = indent ? 1 : 2;
    bool		use_indent = false;
    bool		raw_scalar = false;
    bool		last_was_key = false;
    bool		redo_switch = false;

    if (out == NULL)
        out = makeStringInfo();

    enlargeStringInfo(out, (estimated_len >= 0) ? estimated_len : 64);

    it = JsonbIteratorInit(container);

    while (redo_switch || (type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE) {
        if (type == -1) {
            printf("invalid state.");
            exit(1);
        }
        redo_switch = false;
        switch (type)
        {
            case WJB_BEGIN_ARRAY:
                if (!first)
                    appendBinaryStringInfo(out, ", ", ispaces);

                if (!v.val.array.rawScalar)
                {
                    add_indent(out, use_indent && !last_was_key, level);
                    appendStringInfoCharMacro(out, '[');
                }
                else
                    raw_scalar = true;

                first = true;
                level++;
                break;
            case WJB_BEGIN_OBJECT:
                if (!first)
                    appendBinaryStringInfo(out, ", ", ispaces);

                add_indent(out, use_indent && !last_was_key, level);
                appendStringInfoCharMacro(out, '{');

                first = true;
                level++;
                break;
            case WJB_KEY:
                if (!first)
                    appendBinaryStringInfo(out, ", ", ispaces);
                first = true;

                add_indent(out, use_indent, level);

                jsonb_put_escaped_value(out, &v);
                appendBinaryStringInfo(out, ": ", 2);
                type = JsonbIteratorNext(&it, &v, false);
                if (type == WJB_VALUE)
                {
                    first = false;
                    jsonb_put_escaped_value(out, &v);
                }
                else
                {
                    Assert(type == WJB_BEGIN_OBJECT || type == WJB_BEGIN_ARRAY);
                    redo_switch = true;
                }
                break;
            case WJB_ELEM:
                if (!first)
                    appendBinaryStringInfo(out, ", ", ispaces);
                first = false;

                if (!raw_scalar)
                    add_indent(out, use_indent, level);
                jsonb_put_escaped_value(out, &v);
                break;
            case WJB_END_ARRAY:
                level--;
                if (!raw_scalar)
                {
                    add_indent(out, use_indent, level);
                    appendStringInfoCharMacro(out, ']');
                }
                first = false;
                break;
            case WJB_END_OBJECT:
                level--;
                add_indent(out, use_indent, level);
                appendStringInfoCharMacro(out, '}');
                first = false;
                break;
            default:
                printf("unknown jsonb iterator token type");
        }
        use_indent = indent;
        last_was_key = redo_switch;
    }
    Assert(level == 0);

    return out->data;
}

char* JsonbToCString(StringInfo out, JsonbContainer *container, int estimated_len) {
    return JsonbToCStringWorker(out, container, estimated_len, false);
}

char *
pnstrdup(const char *in, Size len)
{
    char	   *out;

    len = strnlen(in, len);

    out = (char*) malloc(len + 1);
    memcpy(out, in, len);
    out[len] = '\0';

    return out;
}

size_t
pvsnprintf(char *buf, size_t len, const char *fmt, va_list args)
{
    int			nprinted;

    nprinted = vsnprintf(buf, len, fmt, args);

    /* We assume failure means the fmt is bogus, hence hard failure is OK */
    if (unlikely(nprinted < 0))
    {
#ifndef FRONTEND
        printf("vsnprintf failed: %m with format string \"%s\"", fmt);
#else
        fprintf(stderr, "vsnprintf failed: %m with format string \"%s\"\n",
				fmt);
		exit(EXIT_FAILURE);
#endif
    }

    if ((size_t) nprinted < len)
    {
        /* Success.  Note nprinted does not include trailing null. */
        return (size_t) nprinted;
    }

    /*
     * We assume a C99-compliant vsnprintf, so believe its estimate of the
     * required space, and add one for the trailing null.  (If it's wrong, the
     * logic will still work, but we may loop multiple times.)
     *
     * Choke if the required space would exceed MaxAllocSize.  Note we use
     * this palloc-oriented overflow limit even when in frontend.
     */
    if (unlikely((size_t) nprinted > MaxAllocSize - 1))
    {
#ifndef FRONTEND
        printf("out of memory");
#else
        fprintf(stderr, _("out of memory\n"));
		exit(EXIT_FAILURE);
#endif
    }

    return nprinted + 1;
}

int
appendStringInfoVA(StringInfo str, const char *fmt, va_list args)
{
    int			avail;
    size_t		nprinted;

    Assert(str != NULL);

    /*
     * If there's hardly any space, don't bother trying, just fail to make the
     * caller enlarge the buffer first.  We have to guess at how much to
     * enlarge, since we're skipping the formatting work.
     */
    avail = str->maxlen - str->len;
    if (avail < 16)
        return 32;

    nprinted = pvsnprintf(str->data + str->len, (size_t) avail, fmt, args);

    if (nprinted < (size_t) avail)
    {
        /* Success.  Note nprinted does not include trailing null. */
        str->len += (int) nprinted;
        return 0;
    }

    /* Restore the trailing null so that str is unmodified. */
    str->data[str->len] = '\0';

    /*
     * Return pvsnprintf's estimate of the space needed.  (Although this is
     * given as a size_t, we know it will fit in int because it's not more
     * than MaxAllocSize.)
     */
    return (int) nprinted;
}

void
appendStringInfo(StringInfo str, const char *fmt,...)
{
    int			save_errno = errno;

    for (;;)
    {
        va_list		args;
        int			needed;

        /* Try to format the data. */
        errno = save_errno;
        va_start(args, fmt);
        needed = appendStringInfoVA(str, fmt, args);
        va_end(args);

        if (needed == 0)
            break;				/* success */

        /* Increase the buffer size and try again. */
        enlargeStringInfo(str, needed);
    }
}

void
escape_json(StringInfo buf, const char *str)
{
    const char *p;

    appendStringInfoCharMacro(buf, '"');
    for (p = str; *p; p++)
    {
        switch (*p)
        {
            case '\b':
                appendStringInfoString(buf, "\\b");
                break;
            case '\f':
                appendStringInfoString(buf, "\\f");
                break;
            case '\n':
                appendStringInfoString(buf, "\\n");
                break;
            case '\r':
                appendStringInfoString(buf, "\\r");
                break;
            case '\t':
                appendStringInfoString(buf, "\\t");
                break;
            case '"':
                appendStringInfoString(buf, "\\\"");
                break;
            case '\\':
                appendStringInfoString(buf, "\\\\");
                break;
            default:
                if ((unsigned char) *p < ' ')
                    appendStringInfo(buf, "\\u%04x", (int) *p);
                else
                    appendStringInfoCharMacro(buf, *p);
                break;
        }
    }
    appendStringInfoCharMacro(buf, '"');
}
//
//static void
//appendStringInfoString(StringInfo str, const char *s)
//{
//    appendBinaryStringInfo(str, s, strlen(s));
//}



void
appendStringInfoChar(StringInfo str, char ch)
{
    /* Make more room if needed */
    if (str->len + 1 >= str->maxlen)
        enlargeStringInfo(str, 1);

    /* OK, append the character */
    str->data[str->len] = ch;
    str->len++;
    str->data[str->len] = '\0';
}
//
//StringInfo
//makeStringInfo(void)
//{
//    StringInfo	res;
//
//    res = (StringInfo) malloc(sizeof(StringInfoData));
//
//    initStringInfo(res);
//
//    return res;
//}
//
//static void
//initStringInfo(StringInfo str)
//{
//    int			size = 1024;	/* initial default buffer size */
//
//    str->data = (char *) malloc(size);
//    str->maxlen = size;
//    resetStringInfo(str);
//}
//
//static void
//resetStringInfo(StringInfo str)
//{
//    /* don't allow resets of read-only StringInfos */
//    Assert(str->maxlen != 0);
//
//    str->data[0] = '\0';
//    str->len = 0;
//    str->cursor = 0;
//}

JsonbIteratorToken
JsonbIteratorNext(JsonbIterator **it, JsonbValue *val, bool skipNested)
{
    if (*it == NULL)
        return WJB_DONE;

    recurse:
    switch ((*it)->state)
    {
        case JBI_ARRAY_START:
            val->type = jbvArray;
            val->val.array.nElems = (*it)->nElems;

            val->val.array.rawScalar = (*it)->isScalar;
            (*it)->curIndex = 0;
            (*it)->curDataOffset = 0;
            (*it)->curValueOffset = 0;	/* not actually used */

            (*it)->state = JBI_ARRAY_ELEM;
            return WJB_BEGIN_ARRAY;

        case JBI_ARRAY_ELEM:
            if ((*it)->curIndex >= (*it)->nElems)
            {
                *it = freeAndGetParent(*it);
                return WJB_END_ARRAY;
            }

            fillJsonbValue((*it)->container, (*it)->curIndex,
                           (*it)->dataProper, (*it)->curDataOffset,
                           val);

            JBE_ADVANCE_OFFSET((*it)->curDataOffset,
                               (*it)->entries[(*it)->curIndex]);
            (*it)->curIndex++;

            if (!IsAJsonbScalar(val) && !skipNested)
            {
                /* Recurse into container. */
                *it = iteratorFromContainer(val->val.binary.data, *it);
                goto recurse;
            }
            else
            {
                /*
                 * Scalar item in array, or a container and caller didn't want
                 * us to recurse into it.
                 */
                return WJB_ELEM;
            }

        case JBI_OBJECT_START:
            /* Set v to object on first object call */
            val->type = jbvObject;
            val->val.object.nPairs = (*it)->nElems;

            /*
             * v->val.object.pairs is not actually set, because we aren't
             * doing a full conversion
             */
            (*it)->curIndex = 0;
            (*it)->curDataOffset = 0;
            (*it)->curValueOffset = getJsonbOffset((*it)->container,
                                                   (*it)->nElems);

            /* Set state for next call */
            (*it)->state = JBI_OBJECT_KEY;
            return WJB_BEGIN_OBJECT;

        case JBI_OBJECT_KEY:
            if ((*it)->curIndex >= (*it)->nElems)
            {
                *it = freeAndGetParent(*it);
                return WJB_END_OBJECT;
            }
            else
            {
                fillJsonbValue((*it)->container, (*it)->curIndex,
                               (*it)->dataProper, (*it)->curDataOffset,
                               val);
                if (val->type != jbvString) {
                    printf("unexpected jsonb type as object key");
                    exit(1);
                }

                /* Set state for next call */
                (*it)->state = JBI_OBJECT_VALUE;
                return WJB_KEY;
            }

        case JBI_OBJECT_VALUE:
            /* Set state for next call */
            (*it)->state = JBI_OBJECT_KEY;

            fillJsonbValue((*it)->container, (*it)->curIndex + (*it)->nElems,
                           (*it)->dataProper, (*it)->curValueOffset,
                           val);

            JBE_ADVANCE_OFFSET((*it)->curDataOffset,
                               (*it)->entries[(*it)->curIndex]);
            JBE_ADVANCE_OFFSET((*it)->curValueOffset,
                               (*it)->entries[(*it)->curIndex + (*it)->nElems]);
            (*it)->curIndex++;

            if (!IsAJsonbScalar(val) && !skipNested)
            {
                *it = iteratorFromContainer(val->val.binary.data, *it);
                goto recurse;
            }
            else
                return WJB_VALUE;
    }

    printf("invalid iterator state");
    return (JsonbIteratorToken) -1;
}

JsonbIterator *
freeAndGetParent(JsonbIterator *it)
{
    JsonbIterator *v = it->parent;

    free(it);
    return v;
}

uint32
getJsonbOffset(const JsonbContainer *jc, int index)
{
    uint32		offset = 0;
    int			i;

    for (i = index - 1; i >= 0; i--)
    {
        offset += JBE_OFFLENFLD(jc->children[i]);
        if (JBE_HAS_OFF(jc->children[i]))
            break;
    }

    return offset;
}

void
fillJsonbValue(JsonbContainer *container, int index,
               char *base_addr, uint32 offset,
               JsonbValue *result)
{

    JEntry		entry = container->children[index];

    if (JBE_ISNULL(entry))
    {
        result->type = jbvNull;
    }
    else if (JBE_ISSTRING(entry))
    {
        result->type = jbvString;
        result->val.string.val = base_addr + offset;
        result->val.string.len = getJsonbLength(container, index);
        Assert(result->val.string.len >= 0);
    }
    else if (JBE_ISNUMERIC(entry))
    {
        result->type = jbvNumeric;
        result->val.numeric = (Numeric) (base_addr + INTALIGN(offset));
    }
    else if (JBE_ISBOOL_TRUE(entry))
    {
        result->type = jbvBool;
        result->val.boolean = true;
    }
    else if (JBE_ISBOOL_FALSE(entry))
    {
        result->type = jbvBool;
        result->val.boolean = false;
    }
    else
    {
        Assert(JBE_ISCONTAINER(entry));
        result->type = jbvBinary;
        result->val.binary.data = (JsonbContainer *) (base_addr + INTALIGN(offset));
        result->val.binary.len = getJsonbLength(container, index) -
                                 (INTALIGN(offset) - offset);
    }
}

uint32
getJsonbLength(const JsonbContainer *jc, int index)
{
    uint32		off;
    uint32		len;

    if (JBE_HAS_OFF(jc->children[index]))
    {
        off = getJsonbOffset(jc, index);
        len = JBE_OFFLENFLD(jc->children[index]) - off;
    }
    else
        len = JBE_OFFLENFLD(jc->children[index]);

    return len;
}
//
//static void
//appendBinaryStringInfo(StringInfo str, const void *data, int datalen)
//{
//    Assert(str != NULL);
//
//    /* Make more room if needed */
//    enlargeStringInfo(str, datalen);
//
//    /* OK, append the data */
//    memcpy(str->data + str->len, data, datalen);
//    str->len += datalen;
//
//
//    str->data[str->len] = '\0';
//}
//
//static void
//enlargeStringInfo(StringInfo str, int needed)
//{
//    int			newlen;
//
//    Assert(str->maxlen != 0);
//
//    if (needed < 0)				/* should not happen */
//    {
//#ifndef FRONTEND
//        printf("invalid string enlargement request size: %d", needed);
//#else
//        fprintf(stderr, "invalid string enlargement request size: %d\n", needed);
//		exit(EXIT_FAILURE);
//#endif
//    }
//    if (((Size) needed) >= (MaxAllocSize - (Size) str->len))
//    {
//#ifndef FRONTEND
//        printf("Cannot enlarge string buffer containing %d bytes by %d more bytes.");
//#else
//        fprintf(stderr,
//				_("out of memory\n\nCannot enlarge string buffer containing %d bytes by %d more bytes.\n"),
//				str->len, needed);
//		exit(EXIT_FAILURE);
//#endif
//    }
//
//    needed += str->len + 1;		/* total space required now */
//
//    if (needed <= str->maxlen)
//        return;					/* got enough space already */
//
//    newlen = 2 * str->maxlen;
//    while (needed > newlen)
//        newlen = 2 * newlen;
//
//    if (newlen > (int) MaxAllocSize)
//        newlen = (int) MaxAllocSize;
//
//    str->data = (char *) realloc(str->data, newlen);
//
//    str->maxlen = newlen;
//}

static void
add_indent(StringInfo out, bool indent, int level)
{
    if (indent)
    {
        appendStringInfoCharMacro(out, '\n');
        appendStringInfoSpaces(out, level * 4);
    }
}

void
appendStringInfoSpaces(StringInfo str, int count)
{
    if (count > 0)
    {
        /* Make more room if needed */
        enlargeStringInfo(str, count);

        /* OK, append the spaces */
        memset(&str->data[str->len], ' ', count);
        str->len += count;
        str->data[str->len] = '\0';
    }
}

static void
jsonb_put_escaped_value(StringInfo out, JsonbValue *scalarVal)
{
    switch (scalarVal->type)
    {
        case jbvNull:
            appendBinaryStringInfo(out, "null", 4);
            break;
        case jbvString:
            escape_json(out, pnstrdup(scalarVal->val.string.val, scalarVal->val.string.len));
            break;
        case jbvNumeric:
//            appendStringInfoString(out,
//                                   DatumGetCString(DirectFunctionCall1(numeric_out,
//                                                                       PointerGetDatum(scalarVal->val.numeric))));
            break;
        case jbvBool:
            if (scalarVal->val.boolean)
                appendBinaryStringInfo(out, "true", 4);
            else
                appendBinaryStringInfo(out, "false", 5);
            break;
        default:
            printf("unknown jsonb scalar type");
    }
}