//
// Created by 白杰 on 2025/8/8.
//

#include "pg_class.h"

using namespace std;

/*
 * parser pg_class data of tuple header.
 * find out table_oid from tuple data
 */
int resolvePgClassHeapData(char* tuple, const char* tableRelFileNodeId, unsigned int* tableOid) {
    HeapTupleHeaders tup = (HeapTupleHeaders)tuple;
    char* t_data = tuple + tup->t_hoff;
    Form_pg_class pgClassData = (Form_pg_class) t_data;
    if (pgClassData->relfilenode == stoi(tableRelFileNodeId) && tup->t_choice.t_heap.t_xmax == 0) {
        LOG(LOG_LEVEL_DEBUG, "finished find table oid from pg_class data file. resolve table: %s ", pgClassData->relname.data);
        *tableOid = pgClassData->oid;
        return 0;
    }
    return 1;
};

