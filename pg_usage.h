//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_USAGE_H
#define PG_HEXRETRO_PG_USAGE_H



typedef struct DumpOptions {
    char *db_path;
    char *file;        // relfilenode
    char *database;
    char *table;

    int output_name;

    int show_sql;
    int only_visible;
    int dump_all;
    int verbose;
    int none_info;
    char* table_construct;
} DumpOptions;

void print_global_usage();
void print_dump_usage();
void print_inspect_usage();
char* resolve_pgdata(char *input);


#endif //PG_HEXRETRO_PG_USAGE_H
