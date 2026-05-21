# INTRODUCE

[英文版介绍](https://github.com/baijiu1/pg_hexretro/blob/main/README.md)

[pg_hexretro](https://github.com/baijiu1/pg_hexretro) 是一个postgresql/opengauss的底层数据文件解析工具，它可以直接硬解析数据文件，转换为可导入、可查阅、可执行的文本。pg_hexretro由C语言编写，可以用作研究postgresql/opengauss的数据文件结构和作为灾难数据恢复使用。同时也可以解析pg_class/pg_attribute/pg_type等元数据。


# FEATURE

~~Simple and useful !~~

安全: 永远只需要读权限。

全面: 支持所有的列类型，支持postgresql9.x .. 18.x的所有版本，opengauss3.x .. 5.x的所有版本。支持在删除元数据的情况下，只有表结构的恢复。支持无表结构的情况下，类型自动推测，自动推测目前成功率有待提升。

简单: 不依赖任何第三方库，编译后可直接执行。

易用: 通过dump命令，输入数据库路径、数据库名、表名或数据文件号可以直接解析




# USAGE


## 编译

**Linux/MacOS**

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

# 示例

/*
  显示所有的数据版本
*/
```shell
SHELL> ./pg_hexretro dump -D $PGDATA -d database_name -t my_table -s
INSERT INTO mixed_data_table("id", "name", "age", "created_at", "description", "score", "created_date", "notes", "updated_at", "rating", "flag", "extra_data", "data_blob", "event_time", "category", "big_count", "amount") VALUES('1009', 'Alice', '25', '2026-04-17 21:18:46.633143+08', 'first record', '88.500000', '2026-04-17', 'note A', '2026-04-17 21:18:46.633143', '4.500000', 'true', '{"city": "Singapore", "tags": ["a", "b"]}', '\xdeadbeef', '12:30:00.000000', 'A001 ', '1234567890123', '999.99'); ctid: (0, 1)
INSERT INTO mixed_data_table("id", "name", "age", "created_at", "description", "score", "created_date", "notes", "updated_at", "rating", "flag", "extra_data", "data_blob", "event_time", "category", "big_count", "amount") VALUES('1007', 'Alice', '25', '2026-05-08 14:05:11.180500+08', 'test description', '95.500000', '2026-05-08', NULL, '2026-05-08 14:05:11.180500', '99.123456', 'true', '{"key": "value"}', '\xdeadbeef', '12:30:45.000000', 'A001 ', '123456789', '9999.88'); ctid: (0, 2)

/*
  只显示可见版本
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



