# INTRODUCE

[中文版介绍](https://github.com/baijiu1/pg_snapdump/blob/main/README_zh.md)

[pg_snapdump](https://github.com/baijiu1/pg_snapdump) is tool of transform postgrelsql data file to data. It can be parse data files. pg_snapdump written with C/C++ is commonly used to learn postgresql data file construct and data record recovery.



# FEATURE

~~Simple and useful !~~

Security: as long as the file has **read** permission.

Comprehensiveness: **all** column types in postgresql 9.x 10.x 11.x 12.x 13.x, opengauss3.x 5.x 6.x, PanWeiDB

Simple: compile execute file and do it without third-party dependencies.

Useful: parse data with mark of deleted.





# USAGE


## compile

**Linux**

1. configure PG_VERSION_NUM in pg_config.h to match your postgresql/opengauss version, default postgresql 17.2.
```c
cd pg_snapdump/
vim pg_config.h
647 #define PG_VERSION_NUM 170002
```

2. configure log level for more detail info, default INFO.
// log level
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 3
```c
vim CMakeList.txt
add_compile_definitions(LOG_LEVEL=1)
```

3. compile
```shell
cmake .
make
cd bin/
```

**MacOS**

1. configure PG_VERSION_NUM in pg_config.h to match your postgresql/opengauss version.
```c
cd pg_snapdump/
vim pg_config.h
647 #define PG_VERSION_NUM 170002
```

2. configure log level for more detail info, default INFO.
// log level
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 3
```c
vim CMakeList.txt
add_compile_definitions(LOG_LEVEL=1)
```

3. compile
```shell
cmake .
make
cd bin/
```

## usage

**Linux**

```shell
./pg_snapdump [[-f /pgdata/data/base/16384/50000] [-o]]
```

**MacOS**

```shell
./pg_snapdump [[-f /pgdata/data/base/16384/50000] [-o]]
```


# Example

env linux:

```shell
SQL> update table_name set name = 'this name 2' where id = 2;

SHELL> ./pg_snapdump -f /path/to/pgdata/base/16384/50000 
id: 1 name: User_1 age: 108 class: 20 desc: This is a description for row 1  Column read completed. ctid(0,1)
id: 2 name: this name 2 age: 108 class: 20 desc: This is a description for row 2  Column read completed. ctid(0,3)
  -- id: 2 name: User_2 age: 108 class: 20 desc: This is a description for row 2  Column read completed. ctid(0,3)


SHELL> ./pg_snapdump -f /path/to/pgdata/base/16384/50000 -o
id: 1 name: User_1 age: 108 class: 20 desc: This is a description for row 1  Column read completed. ctid(0,1)
id: 2 name: this name 2 age: 108 class: 20 desc: This is a description for row 2  Column read completed. ctid(0,3)
```


# CHANGE LOG

| VERSION | UPDATE     | NOTE                                     |
| ------- | ---------- | ---------------------------------------- |
| v0.1    | 2024.8.21  | first version                            |





# REQUIRE & SUPPORT

require: gcc

support range: postgresql/opengauss
