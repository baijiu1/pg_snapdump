//
// Created by 白杰 on 2026/4/22.
//

#include "pg_usage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_global_usage()
{
//    printf(
//            "Usage:\n"
//            "  pg_hexretro <command> [options]\n\n"
//
//            "Commands:\n"
//            "  dump        Dump table data (by table name or relfilenode)\n"
//            "  inspect     Inspect page / tuple / jsonb\n"
//            "  catalog     Explore system catalogs\n\n"
//
//            "Run 'pg_hexretro <command> -h' for more details.\n\n"
//
//            "Examples:\n"
//            "  pg_hexretro dump -D $PGDATA -f 66274\n"
//            "  pg_hexretro dump -D $PGDATA -d mydb -t users\n"
//            "  pg_hexretro inspect -D $PGDATA -f 66274 -p 0\n"
//            "  pg_hexretro inspect -D $PGDATA -f 66274 -p 0 -T 3 --jsonb\n"
//            "  pg_hexretro catalog -D $PGDATA --list-db\n"
//            "  pg_hexretro catalog -D $PGDATA --list-table mydb\n"
//            "  pg_hexretro catalog -D $PGDATA --describe mydb.users\n"
//    );
    print_dump_usage();
}

void print_dump_usage()
{
    printf(
            "Version 1.1 (for PostgreSQL 8.x .. 17.x)\n"
            "Display formatted contents of a PostgreSQL heap/index/control file"

            "Usage:\n"
            "  pg_hexretro dump [options]\n\n"

            "Options:\n"
            "  -D, --db_path <path>\n"
            "        PostgreSQL data directory\n\n"

            "  -f, --file <relfilenode>\n"
            "        Target relation file\n\n"

            "  -d, --database <name>\n"
            "        Database name\n\n"

            "  -t, --table <name>\n"
            "        Table name\n\n"

            "  -e, --export\n"
            "        Output file name, default format csv\n\n"

            "  -s, --sql-mode \n"
            "        Only show SQL statement\n\n"

            "  -o, --only-visible\n"
            "        Only dump visible (latest) tuples\n\n"

            "  -T, --table-construct\n"
            "        Only know table construct when pg_class or pg_attribute was dropped\n\n"

            "  -v, --verbose\n"
            "        Enable verbose output\n\n"

            "Examples:\n"
            "  pg_hexretro dump -D $PGDATA -f 66274\n"
            "  pg_hexretro dump -D $PGDATA -d mydb -t my_table\n"
            "  pg_hexretro dump -D $PGDATA -d mydb -t users -s\n"
            "  pg_hexretro dump -D $PGDATA -d mydb -t users -e\n"
    );
}

void print_inspect_usage()
{
    printf(
            "Usage:\n"
            "  pg_hexretro inspect [options]\n\n"

            "Options:\n"
            "  -D, --db_path <path>\n"
            "        PostgreSQL data directory\n\n"

            "  -f, --file <relfilenode>\n"
            "        Target relation file\n\n"

            "  -p, --page <number>\n"
            "        Page number\n\n"

            "  -T, --tuple <offset>\n"
            "        Tuple offset in page\n\n"

            "  -R, --raw\n"
            "        Show raw hex data\n\n"

            "      --jsonb\n"
            "        Decode JSONB field\n\n"

            "  -v, --verbose\n"
            "        Enable verbose output\n\n"

            "Examples:\n"
            "  pg_hexretro inspect -D $PGDATA -f 66274 -p 0\n"
            "  pg_hexretro inspect -D $PGDATA -f 66274 -p 0 -T 3 --jsonb\n"
    );
}

void print_catalog_usage()
{
    printf(
            "Usage:\n"
            "  pg_hexretro catalog [options]\n\n"

            "Options:\n"
            "  -D, --db_path <path>\n"
            "        PostgreSQL data directory\n\n"

            "  -l, --list-db\n"
            "        List all databases\n\n"

            "  -L, --list-table <db>\n"
            "        List tables in database\n\n"

            "  -s, --describe <db.table>\n"
            "        Show table schema\n\n"

            "  -v, --verbose\n"
            "        Enable verbose output\n\n"

            "Examples:\n"
            "  pg_hexretro catalog -D $PGDATA --list-db\n"
            "  pg_hexretro catalog -D $PGDATA --list-table mydb\n"
            "  pg_hexretro catalog -D $PGDATA --describe mydb.users\n"
    );
}


char* resolve_pgdata(char *input)
{
    if (!input) {
        fprintf(stderr,
                "Error: missing data path (-D or PGDATA)\n");
        exit(1);
    }

    const char *tag = "$PGDATA";
    const char *env = NULL;

    if (strstr(input, tag)) {

        env = getenv("PGDATA");
        if (!env || strlen(env) == 0) {
            fprintf(stderr,
                    "Error: PGDATA not set\n");
            exit(1);
        }

        size_t len = strlen(env) + strlen(input) + 1;

        char *result = (char*) malloc(len);
        if (!result) {
            perror("malloc");
            exit(1);
        }

        snprintf(result, len, "%s%s", env, input + strlen(tag));

        return result;
    }

    // 直接 copy，避免返回 argv 内存
    return strdup(input);
}