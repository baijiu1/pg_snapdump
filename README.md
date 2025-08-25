# INTRODUCE


[pg_snapdump](https://github.com/baijiu1/pg_snapdump) is tool of transform postgrelsql data file to data. It can be parse data files. pg_snapdump written with C/C++ is commonly used to learn data files and data recovery.



# FEATURE

~~Simple and useful !~~

Security: as long as the file has **read** permission.

Comprehensiveness: **all** column types in postgresql 9.x 10.x 11.x 12.x 13.x

Simple: compile execute file and do it without third-party dependencies.

Useful: parse data with mark of deleted (--delete).





# USAGE


## compile

**Linux**

```shell
cmake .
make
cd bin/
```

**MacOS**

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
id: 1 name: User_1 age: 108 class: 20 desc: This is a description for row 1  列读取完毕 ctid(0,1)
id: 2 name: this name 2 age: 108 class: 20 desc: This is a description for row 2 列读取完毕 ctid(0,3)
  -- id: 2 name: User_2 age: 108 class: 20 desc: This is a description for row 2 列读取完毕 ctid(0,2)


SHELL> ./pg_snapdump -f /path/to/pgdata/base/16384/50000 -o
id: 1 name: User_1 age: 108 class: 20 desc: This is a description for row 1 列读取完毕 ctid(0,1)
id: 2 name: this name 2 age: 108 class: 20 desc: This is a description for row 2 列读取完毕 ctid(0,3)
```


# CHANGE LOG

| VERSION | UPDATE     | NOTE                                     |
| ------- | ---------- | ---------------------------------------- |
| v0.1    | 2024.8.21  | first version                            |





# REQUIRE & SUPPORT

require: gcc

support range: postgresql/opengauss/磐维
