# INTRODUCE

[中文版介绍](https://github.com/baijiu1/pg_hexretro/blob/main/README_zh.md)

[pg_hexretro](https://github.com/baijiu1/pg_hexretro) is tool of transform postgrelsql data file to data. It can be parse data files. pg_hexretro written with C is commonly used to learn postgresql data file construct and data record recovery. can be parser meta data from pg_class/pg_attribute/pg_type.



# FEATURE

~~Simple and useful !~~

Security: as long as the file has **read** permission.

Comprehensiveness: **all** column types in postgresql 9.x 10.x 11.x 12.x 13.x 14.x 15.x 16.x 17.x 18.x, opengauss3.x 5.x 6.x. support all column types for parser.

Simple: compile execute file and do it without third-party dependencies.

Useful: parse data with mark of deleted.







# USAGE


## compile

**Linux/MacOS**

1. configure PG_VERSION_NUM in pg_config.h to match your postgresql/opengauss version, default postgresql 17.2.
```c
cd pg_snapdump/
vim pg_config.h
647 #define PG_VERSION_NUM 170002
```

2. compile
```shell
cmake .
make
cd bin/
```

## usage

**Linux/MacOS**

```shell
SHELL> ./pg_hexretro 
Version 1.1 (for PostgreSQL 8.x .. 17.x opengauss 3.x .. 5.x)
Display formatted contents of a PostgreSQL heap fileUsage:
  pg_hexretro dump [options]

Options:
  -D, --db_path <path>
        PostgreSQL data directory

  -f, --file <relfilenode>
        Target relation file

  -d, --database <name>
        Database name

  -t, --table <name>
        Table name

  -e, --export
        Output file name, default format csv

  -s, --sql-mode 
        Only show SQL statement

  -o, --only-visible
        Only dump visible (latest) tuples

  -T, --table-construct
        Only know table construct when pg_class or pg_attribute was dropped

  -v, --verbose
        Enable verbose output

Examples:
  pg_hexretro dump -D $PGDATA -f 66274
  pg_hexretro dump -D $PGDATA -d mydb -t my_table
  pg_hexretro dump -D $PGDATA -d mydb -t users -s
  pg_hexretro dump -D $PGDATA -d mydb -t users -e

```

# Example

/*
  show all tuple version
*/
```shell
SHELL> ./pg_hexretro dump -D $PGDATA -d database_name -t my_table -s
INSERT INTO mixed_data_table("id", "name", "age", "created_at", "description", "score", "created_date", "notes", "updated_at", "rating", "flag", "extra_data", "data_blob", "event_time", "category", "big_count", "amount") VALUES('1009', 'Alice', '25', '2026-04-17 21:18:46.633143+08', 'first record', '88.500000', '2026-04-17', 'note A', '2026-04-17 21:18:46.633143', '4.500000', 'true', '{"city": "Singapore", "tags": ["a", "b"]}', '\xdeadbeef', '12:30:00.000000', 'A001 ', '1234567890123', '999.99'); ctid: (0, 1)
INSERT INTO mixed_data_table("id", "name", "age", "created_at", "description", "score", "created_date", "notes", "updated_at", "rating", "flag", "extra_data", "data_blob", "event_time", "category", "big_count", "amount") VALUES('1007', 'Alice', '25', '2026-05-08 14:05:11.180500+08', 'test description', '95.500000', '2026-05-08', NULL, '2026-05-08 14:05:11.180500', '99.123456', 'true', '{"key": "value"}', '\xdeadbeef', '12:30:45.000000', 'A001 ', '123456789', '9999.88'); ctid: (0, 2)

/*
  only display new tuple version
*/
SQL> DELETE from my_table where id = 1009;
SHELL> ./pg_hexretro dump -D $PGDATA -d database_name -t my_table -o
INSERT INTO mixed_data_table("id", "name", "age", "created_at", "description", "score", "created_date", "notes", "updated_at", "rating", "flag", "extra_data", "data_blob", "event_time", "category", "big_count", "amount") VALUES('1007', 'Alice', '25', '2026-05-08 14:05:11.180500+08', 'test description', '95.500000', '2026-05-08', NULL, '2026-05-08 14:05:11.180500', '99.123456', 'true', '{"key": "value"}', '\xdeadbeef', '12:30:45.000000', 'A001 ', '123456789', '9999.88'); ctid: (0, 2)
```


# CHANGE LOG

| VERSION | UPDATE     | NOTE                                     |
| ------- | ---------- | ---------------------------------------- |
| v0.1    | 2024.8.21  | first version                            |
| v0.2    | 2026.5.21  | support all column types                 |




# REQUIRE & SUPPORT

require: gcc

support range: postgresql/opengauss

# CONTACT ME

email: baijiu1100@hotmail.com



