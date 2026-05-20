//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_STRING_H
#define PG_HEXRETRO_PG_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "pg_c.h"
#include "pg_integer.h"
#include "pg_heap.h"
#include "pg_string_info.h"

Datum ns_decode_str(char *ptr);
Datum ns_decode_char(char *ptr);

Datum ns_decode_bool(char *ptr);
Datum ns_decode_name(char *ptr);

//Datum ns_decode_jsonb(char *ptr);

Datum ns_decode_uuid(char *ptr);
Datum ns_decode_inet(char *ptr);
Datum ns_decode_cidr(char *ptr);
Datum ns_decode_macaddr(char *ptr);
Datum ns_decode_hstore(char *ptr);


char* ByteaDatumToCString(ScanContext *ctx, Datum d);

typedef uint32_t JEntry;

/* ===== JEntry 位结构 ===== */
#define JENTRY_TYPE_MASK   0xF0000000
#define JENTRY_OFFLEN_MASK 0x0FFFFFFF

#define JENTRY_HAS_OFF     0x80000000


/* container header */
#define JB_FOBJECT 0x20000000
#define JB_FARRAY  0x40000000

#define JB_CMASK   0x0FFFFFFF

typedef uint32 JEntry;
typedef struct JsonbContainer
{
    uint32		header;			/* number of elements or key/value pairs, and
								 * flags */
    JEntry		children[FLEXIBLE_ARRAY_MEMBER];

    /* the data for each child node follows. */
} JsonbContainer;
typedef struct
{
    int32		vl_len_;		/* varlena header (do not touch directly!) */
    JsonbContainer root;
} Jsonb;
Jsonb*
DatumGetJsonb(ScanContext *ctx, Datum X);
char* JsonbToCString(StringInfo out, JsonbContainer *container, int);
static inline uint32_t jentry_get_len(JEntry e);
static void parse_string(char *ptr, JEntry e, char *out, int *pos);
static inline int jentry_has_off(JEntry e);
static char *
JsonbToCStringWorker(StringInfo out, JsonbContainer *container, int estimated_len, bool indent);



static inline uint32_t jentry_type(JEntry e) ;
void parse_value(char *ptr, JEntry e, char *out, int *pos);
void
appendStringInfoChar(StringInfo str, char ch);
void
appendStringInfoSpaces(StringInfo str, int count);
static void
add_indent(StringInfo out, bool indent, int level);
char *
pnstrdup(const char *in, Size len);
void
escape_json(StringInfo buf, const char *str);
void
appendStringInfo(StringInfo str, const char *fmt,...);
int
appendStringInfoVA(StringInfo str, const char *fmt, va_list args);
size_t
pvsnprintf(char *buf, size_t len, const char *fmt, va_list args);

#define appendStringInfoCharMacro(str,ch) \
	(((str)->len + 1 >= (str)->maxlen) ? \
	 appendStringInfoChar(str, ch) : \
	 (void)((str)->data[(str)->len] = (ch), (str)->data[++(str)->len] = '\0'))

#define JB_CMASK				0x0FFFFFFF	/* mask for count field */
#define JB_FSCALAR				0x10000000	/* flag bits */
#define JB_FOBJECT				0x20000000
#define JB_FARRAY				0x40000000

/* convenience macros for accessing a JsonbContainer struct */
#define JsonContainerSize(jc)		((jc)->header & JB_CMASK)
#define JsonContainerIsScalar(jc)	(((jc)->header & JB_FSCALAR) != 0)
#define JsonContainerIsObject(jc)	(((jc)->header & JB_FOBJECT) != 0)
#define JsonContainerIsArray(jc)	(((jc)->header & JB_FARRAY) != 0)

typedef enum {
    JBI_DONE,
    JBI_ARRAY_START,
    JBI_ARRAY_ELEM,
    JBI_OBJECT_START,
    JBI_OBJECT_KEY,
    JBI_OBJECT_VALUE
} JsonbIterState;


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
} JsonbIteratorToken;


typedef struct JsonbIterator
{
    /* Container being iterated */
    JsonbContainer *container;
    uint32		nElems;			/* Number of elements in children array (will
								 * be nPairs for objects) */
    bool		isScalar;		/* Pseudo-array scalar value? */
    JEntry	   *entries;		/* JEntrys for child nodes */
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

typedef struct JsonbPair JsonbPair;
typedef struct JsonbValue JsonbValue;
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



#define IsAJsonbScalar(jsonbval)	(((jsonbval)->type >= jbvNull && \
									  (jsonbval)->type <= jbvBool) || \
									  (jsonbval)->type == jbvDatetime)




#define JGINFLAG_KEY	0x01	/* key (or string array element) */
#define JGINFLAG_NULL	0x02	/* null value */
#define JGINFLAG_BOOL	0x03	/* boolean value */
#define JGINFLAG_NUM	0x04	/* numeric value */
#define JGINFLAG_STR	0x05	/* string value (if not an array element) */
#define JGINFLAG_HASHED 0x10	/* OR'd into flag if value was hashed */
#define JGIN_MAXLENGTH	125		/* max length of text part before hashing */

#define JENTRY_OFFLENMASK		0x0FFFFFFF
#define JENTRY_TYPEMASK			0x70000000
#define JENTRY_HAS_OFF			0x80000000

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

/* Strategy numbers for GIN index opclasses */
#define JsonbContainsStrategyNumber		7
#define JsonbExistsStrategyNumber		9
#define JsonbExistsAnyStrategyNumber	10
#define JsonbExistsAllStrategyNumber	11
#define JsonbJsonpathExistsStrategyNumber		15
#define JsonbJsonpathPredicateStrategyNumber	16


JsonbIterator* JsonbIteratorInit(JsonbContainer* container);
JsonbIteratorToken
JsonbIteratorNext(JsonbIterator **it, JsonbValue *val, bool skipNested);
JsonbIterator *
freeAndGetParent(JsonbIterator *it);
void
fillJsonbValue(JsonbContainer *container, int index,
               char *base_addr, uint32 offset,
               JsonbValue *result);
#define JBE_ADVANCE_OFFSET(offset, je) \
	do { \
		JEntry	je_ = (je); \
		if (JBE_HAS_OFF(je_)) \
			(offset) = JBE_OFFLENFLD(je_); \
		else \
			(offset) += JBE_OFFLENFLD(je_); \
	} while(0)
uint32
getJsonbOffset(const JsonbContainer *jc, int index);

static void
jsonb_put_escaped_value(StringInfo out, JsonbValue *scalarVal);
static JsonbIterator *
iteratorFromContainer(JsonbContainer *container, JsonbIterator *parent);
uint32
getJsonbLength(const JsonbContainer *jc, int index);
#endif //PG_HEXRETRO_PG_STRING_H
