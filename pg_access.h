//
// Created by 白杰 on 2026/5/19.
//

#ifndef PG_HEXRETRO_PG_ACCESS_H
#define PG_HEXRETRO_PG_ACCESS_H
#include <string.h>
#include "pg_usage.h"
#include "pg_shm.h"
#include "pg_core_types.h"
#include "pg_scan.h"
#include "pg_page.h"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>


int pg_forge(DumpOptions *op, char flag);
Visibility_ctx* init_visibility_mode(ScanContext *ctx, char*);
void init_file_access(Engine *e);
int file_exists(const char *path);

int open_file(const char *,FileHandle*);
int mmap_file(FileHandle*);
char* get_page(FileHandle*, int);
off_t read_file_size(FileHandle*);
int close_file(FileHandle*);
void init_slot_mvcc(tupleSlot *slot, size_t size);


Relocator* init_relocator();
void build_base_path(Relocator *r, const char *oid_path);
void build_from_path(Engine *, const char*);


Engine* init_engine();
Access* init_access();

tupleSlot* make_slot();

int load_table(ScanContext*);

Schema* build_pg_class_schema();
Schema* build_pg_attribute_schema();
Schema* build_user_schema(ScanContext*);
void slot_bind_schema(tupleSlot *slot, Schema *schema);
SchemaBuilder * schema_builder_init();

int init(char*, char);
Schema *build_pg_database_schema();

#endif //PG_HEXRETRO_PG_ACCESS_H
