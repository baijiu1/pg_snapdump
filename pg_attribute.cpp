//
// Created by 白杰 on 2025/8/8.
//

#include "pg_attribute.h"


using namespace std;


/*
 * parser pg_attribute data of tuple header.
 * find out attnum, attname, attlen, attalign into struct ColAttribute from tuple data.
 */
int resolvePgAttributeHeapData(char* tuple, unsigned int* tableOid, ColAttribute& colAttr) {
    HeapTupleHeaders tup = (HeapTupleHeaders)tuple;
    char* t_data = tuple + tup->t_hoff;
    Form_pg_attribute pgAttributeData = (Form_pg_attribute) t_data;
    if (pgAttributeData->attrelid == *tableOid && tup->t_choice.t_heap.t_xmax == 0) {
        if (strcmp("ctid", pgAttributeData->attname.data) != 0 &&
            strcmp("xmin", pgAttributeData->attname.data) != 0 &&
            strcmp("cmin", pgAttributeData->attname.data) != 0 &&
            strcmp("xmax", pgAttributeData->attname.data) != 0 &&
            strcmp("cmax", pgAttributeData->attname.data) != 0 &&
            strcmp("tableoid", pgAttributeData->attname.data) != 0) {
                colAttr.colAttNum.emplace_back(pgAttributeData->attnum);
                colAttr.colName.emplace_back(pgAttributeData->attname.data);
                colAttr.colType.emplace_back(pgAttributeData->atttypid);
                colAttr.colAttlen.emplace_back(pgAttributeData->attlen);
                colAttr.colAttalign.emplace_back(pgAttributeData->attalign);
        }
    }
    return 0;
};