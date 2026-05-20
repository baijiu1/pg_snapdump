//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_JSONB_H
#define PG_HEXRETRO_PG_JSONB_H

#include "pg_c.h"
#include "pg_common.h"
#include "pg_integer.h"

typedef enum
{
    WJB_DONE,
    WJB_KEY,
    WJB_VALUE,
    WJB_ELEM,
    WJB_BEGIN_ARRAY,
    WJB_END_ARRAY,
    WJB_BEGIN_OBJECT,
    WJB_END_OBJECT,
    WJB_ERROR,
} JsonbIteratorToken;

/* Strategy numbers for GIN index opclasses */
#define JsonbContainsStrategyNumber		7
#define JsonbExistsStrategyNumber		9
#define JsonbExistsAnyStrategyNumber	10
#define JsonbExistsAllStrategyNumber	11
#define JsonbJsonpathExistsStrategyNumber		15
#define JsonbJsonpathPredicateStrategyNumber	16

#define JGINFLAG_KEY	0x01	/* key (or string array element) */
#define JGINFLAG_NULL	0x02	/* null value */
#define JGINFLAG_BOOL	0x03	/* boolean value */
#define JGINFLAG_NUM	0x04	/* numeric value */
#define JGINFLAG_STR	0x05	/* string value (if not an array element) */
#define JGINFLAG_HASHED 0x10	/* OR'd into flag if value was hashed */
#define JGIN_MAXLENGTH	125		/* max length of text part before hashing */

typedef struct JsonbPair JsonbPair;
typedef struct JsonbValue JsonbValue;


#define JENTRY_OFFLENMASK		0x0FFFFFFF
#define JENTRY_TYPEMASK			0x70000000
#define JENTRY_HAS_OFF			0x80000000

/* values stored in the type bits */
#define JENTRY_ISSTRING			0x00000000
#define JENTRY_ISNUMERIC		0x10000000
#define JENTRY_ISBOOL_FALSE		0x20000000
#define JENTRY_ISBOOL_TRUE		0x30000000
#define JENTRY_ISNULL			0x40000000
#define JENTRY_ISCONTAINER		0x50000000	/* array or object */

/* Access macros.  Note possible multiple evaluations */
#define JBE_OFFLENFLD(je_)		((je_) & JENTRY_OFFLENMASK)
#define JBE_HAS_OFF(je_)		(((je_) & JENTRY_HAS_OFF) != 0)
#define JBE_ISSTRING(je_)		(((je_) & JENTRY_TYPEMASK) == JENTRY_ISSTRING)
#define JBE_ISNUMERIC(je_)		(((je_) & JENTRY_TYPEMASK) == JENTRY_ISNUMERIC)
#define JBE_ISCONTAINER(je_)	(((je_) & JENTRY_TYPEMASK) == JENTRY_ISCONTAINER)
#define JBE_ISNULL(je_)			(((je_) & JENTRY_TYPEMASK) == JENTRY_ISNULL)
#define JBE_ISBOOL_TRUE(je_)	(((je_) & JENTRY_TYPEMASK) == JENTRY_ISBOOL_TRUE)
#define JBE_ISBOOL_FALSE(je_)	(((je_) & JENTRY_TYPEMASK) == JENTRY_ISBOOL_FALSE)
#define JBE_ISBOOL(je_)			(JBE_ISBOOL_TRUE(je_) || JBE_ISBOOL_FALSE(je_))

/* Macro for advancing an offset variable to the next JEntry */
#define JBE_ADVANCE_OFFSET(offset, je) \
	do { \
		JEntry	je_ = (je); \
		if (JBE_HAS_OFF(je_)) \
			(offset) = JBE_OFFLENFLD(je_); \
		else \
			(offset) += JBE_OFFLENFLD(je_); \
	} while(0)

#define JB_OFFSET_STRIDE		32
typedef struct JsonbContainer
{
    uint32		header;			/* number of elements or key/value pairs, and
								 * flags */
    JEntry		children[FLEXIBLE_ARRAY_MEMBER];

    /* the data for each child node follows. */
} JsonbContainer;

/* flags for the header-field in JsonbContainer */
#define JB_CMASK				0x0FFFFFFF	/* mask for count field */
#define JB_FSCALAR				0x10000000	/* flag bits */
#define JB_FOBJECT				0x20000000
#define JB_FARRAY				0x40000000

/* convenience macros for accessing a JsonbContainer struct */
#define JsonContainerSize(jc)		((jc)->header & JB_CMASK)
#define JsonContainerIsScalar(jc)	(((jc)->header & JB_FSCALAR) != 0)
#define JsonContainerIsObject(jc)	(((jc)->header & JB_FOBJECT) != 0)
#define JsonContainerIsArray(jc)	(((jc)->header & JB_FARRAY) != 0)

/* The top-level on-disk format for a jsonb datum. */
typedef struct
{
    int32		vl_len_;		/* varlena header (do not touch directly!) */
    JsonbContainer root;
} Jsonb;

/* convenience macros for accessing the root container in a Jsonb datum */
#define JB_ROOT_COUNT(jbp_)		(*(uint32 *) VARDATA(jbp_) & JB_CMASK)
#define JB_ROOT_IS_SCALAR(jbp_) ((*(uint32 *) VARDATA(jbp_) & JB_FSCALAR) != 0)
#define JB_ROOT_IS_OBJECT(jbp_) ((*(uint32 *) VARDATA(jbp_) & JB_FOBJECT) != 0)
#define JB_ROOT_IS_ARRAY(jbp_)	((*(uint32 *) VARDATA(jbp_) & JB_FARRAY) != 0)


enum jbvType
{
    /* Scalar types */
    jbvNull = 0x0,
    jbvString,
    jbvNumeric,
    jbvBool,
    /* Composite types */
    jbvArray = 0x10,
    jbvObject,
    /* Binary (i.e. struct Jsonb) jbvArray/jbvObject */
    jbvBinary,

    /*
     * Virtual types.
     *
     * These types are used only for in-memory JSON processing and serialized
     * into JSON strings when outputted to json/jsonb.
     */
    jbvDatetime = 0x20,
};

/*
 * JsonbValue:	In-memory representation of Jsonb.  This is a convenient
 * deserialized representation, that can easily support using the "val"
 * union across underlying types during manipulation.  The Jsonb on-disk
 * representation has various alignment considerations.
 */
struct JsonbValue
{
    enum jbvType type;			/* Influences sort order */

    union
    {
        Numeric numeric;
        bool		boolean;
        struct
        {
            int			len;
            char	   *val;	/* Not necessarily null-terminated */
        }			string;		/* String primitive type */

        struct
        {
            int			nElems;
            JsonbValue *elems;
            bool		rawScalar;	/* Top-level "raw scalar" array? */
        }			array;		/* Array container type */

        struct
        {
            int			nPairs; /* 1 pair, 2 elements */
            JsonbPair  *pairs;
        }			object;		/* Associative container type */

        struct
        {
            int			len;
            JsonbContainer *data;
        }			binary;		/* Array or object, in on-disk format */

        struct
        {
            Datum		value;
            Oid			typid;
            int32		typmod;
            int			tz;		/* Numeric time zone, in seconds, for
								 * TimestampTz data type */
        }			datetime;
    }			val;
};

#define IsAJsonbScalar(jsonbval)	(((jsonbval)->type >= jbvNull && \
									  (jsonbval)->type <= jbvBool) || \
									  (jsonbval)->type == jbvDatetime)

/*
 * Key/value pair within an Object.
 *
 * This struct type is only used briefly while constructing a Jsonb; it is
 * *not* the on-disk representation.
 *
 * Pairs with duplicate keys are de-duplicated.  We store the originally
 * observed pair ordering for the purpose of removing duplicates in a
 * well-defined way (which is "last observed wins").
 */
struct JsonbPair
{
    JsonbValue	key;			/* Must be a jbvString */
    JsonbValue	value;			/* May be of any type */
    uint32		order;			/* Pair's index in original sequence */
};

/* Conversion state used when parsing Jsonb from text, or for type coercion */
typedef struct JsonbParseState
{
    JsonbValue	contVal;
    Size		size;
    struct JsonbParseState *next;
    bool		unique_keys;	/* Check object key uniqueness */
    bool		skip_nulls;		/* Skip null object fields */
} JsonbParseState;

/*
 * JsonbIterator holds details of the type for each iteration. It also stores a
 * Jsonb varlena buffer, which can be directly accessed in some contexts.
 */
typedef enum
{
    JBI_ARRAY_START,
    JBI_ARRAY_ELEM,
    JBI_OBJECT_START,
    JBI_OBJECT_KEY,
    JBI_OBJECT_VALUE,
} JsonbIterState;

typedef struct JsonbIterator
{
    /* Container being iterated */
    JsonbContainer *container;
    uint32		nElems;			/* Number of elements in children array (will
								 * be nPairs for objects) */
    bool		isScalar;		/* Pseudo-array scalar value? */
    JEntry	   *children;		/* JEntrys for child nodes */
    /* Data proper.  This points to the beginning of the variable-length data */
    char	   *dataProper;

    /* Current item in buffer (up to nElems) */
    int			curIndex;

    /* Data offset corresponding to current item */
    uint32		curDataOffset;

    /*
     * If the container is an object, we want to return keys and values
     * alternately; so curDataOffset points to the current key, and
     * curValueOffset points to the current value.
     */
    uint32		curValueOffset;

    /* Private state */
    JsonbIterState state;

    struct JsonbIterator *parent;
} JsonbIterator;


/* Convenience macros */
Jsonb *
DatumGetJsonbP(ScanContext *ctx, Datum d)
{
    return (Jsonb *) PG_DETOAST_DATUM(ctx, d);
}

//static inline Jsonb *
//DatumGetJsonbPCopy(ScanContext *ctx, Datum d)
//{
//    return (Jsonb *) PG_DETOAST_DATUM_COPY(ctx, d);
//}
//
//static inline Datum
//JsonbPGetDatum(const Jsonb *p)
//{
//    return PointerGetDatum(p);
//}

#define PG_GETARG_JSONB_P(x)	DatumGetJsonbP(PG_GETARG_DATUM(x))
#define PG_GETARG_JSONB_P_COPY(x)	DatumGetJsonbPCopy(PG_GETARG_DATUM(x))
#define PG_RETURN_JSONB_P(x)	PG_RETURN_POINTER(x)

/* jsonb.c support functions */
char *JsonbToCString(StringInfo out, JsonbContainer *in,
                     int estimated_len);
static char *
JsonbToCStringWorker(StringInfo out, JsonbContainer *in, int estimated_len, bool indent);
StringInfo
makeStringInfo(void);
void
initStringInfo(StringInfo str);
void
resetStringInfo(StringInfo str);
void
enlargeStringInfo(StringInfo str, int needed);
JsonbIterator *
iteratorFromContainer(JsonbContainer *container, JsonbIterator *parent);
JsonbIterator *
JsonbIteratorInit(JsonbContainer *container);
JsonbIteratorToken
JsonbIteratorNext(JsonbIterator **it, JsonbValue *val, bool skipNested);
JsonbIterator *
freeAndGetParent(JsonbIterator *it);
void
fillJsonbValue(JsonbContainer *container, int index,
               char *base_addr, uint32 offset,
               JsonbValue *result);
void
appendBinaryStringInfo(StringInfo str, const void *data, int datalen);
void
add_indent(StringInfo out, bool indent, int level);

void
appendStringInfoChar(StringInfo str, char ch);
#define appendStringInfoCharMacro(str,ch) \
	(((str)->len + 1 >= (str)->maxlen) ? \
	 appendStringInfoChar(str, ch) : \
	 (void)((str)->data[(str)->len] = (ch), (str)->data[++(str)->len] = '\0'))

void
appendStringInfoSpaces(StringInfo str, int count);
void
jsonb_put_escaped_value(StringInfo out, JsonbValue *scalarVal);
char *
pnstrdup(const char *in, Size len);
void
escape_json(StringInfo buf, const char *str);
void
appendStringInfoString(StringInfo str, const char *s);
void
appendStringInfo(StringInfo str, const char *fmt,...);
int
appendStringInfoVA(StringInfo str, const char *fmt, va_list args);
size_t
pvsnprintf(char *buf, size_t len, const char *fmt, va_list args);


#endif //PG_HEXRETRO_PG_JSONB_H
