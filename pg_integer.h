//
// Created by 白杰 on 2026/5/20.
//

#ifndef PG_HEXRETRO_PG_INTEGER_H
#define PG_HEXRETRO_PG_INTEGER_H
#include <stdio.h>
#include <stdlib.h>
#include "pg_c.h"
#include "pg_detoast.h"
#include "pg_core_types.h"
#include "pg_scan.h"
#include "pg_numeric.h"

Datum ns_decode_int2(char *ptr);
Datum ns_decode_int4(char *ptr);
Datum ns_decode_int8(char *ptr);
Datum ns_decode_oid(char *ptr);

Datum ns_decode_float(char *ptr);
Datum ns_decode_double(char *ptr);
Datum ns_decode_ns_numeric(char *ptr);

Datum ns_decode_int4range(char *ptr);
Datum ns_decode_numrange(char *ptr);
Datum ns_decode_tsrange(char *ptr);
Datum ns_decode_tstzrange(char *ptr);
Datum ns_decode_daterange(char *ptr);

char* numeric_to_str(Numeric num);

static void
init_var_from_num(Numeric num, NumericVar *dest);
static char *
get_str_from_var(const NumericVar *var);

#ifdef __cplusplus
extern "C" {
#endif

Numeric DatumGetNumeric(ScanContext *ctx, Datum d);

#ifdef __cplusplus
}
#endif
//static inline Numeric
//DatumGetNumeric(ScanContext *ctx, Datum X);

struct varlena *
pg_detoast_datum(ScanContext *ctx, struct varlena *datum);

#define PG_DETOAST_DATUM(ctx, datum) \
	pg_detoast_datum(ctx, (struct varlena *) DatumGetPointer(datum))
#define PG_DETOAST_DATUM_COPY(datum) \
	pg_detoast_datum_copy((struct varlena *) DatumGetPointer(datum))
#define PG_DETOAST_DATUM_SLICE(datum,f,c) \
		pg_detoast_datum_slice((struct varlena *) DatumGetPointer(datum), \
		(int32) (f), (int32) (c))
/* WARNING -- unaligned pointer */
#define PG_DETOAST_DATUM_PACKED(datum) \
	pg_detoast_datum_packed((struct varlena *) DatumGetPointer(datum))


#endif //PG_HEXRETRO_PG_INTEGER_H
