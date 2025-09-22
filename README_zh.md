# 指引

[英文版介绍](https://github.com/baijiu1/pg_snapdump/blob/main/README.md)

[pg_snapdump](https://github.com/baijiu1/pg_snapdump) 是一款数据恢复的工具。 它可以直接从postgresql/opengauss的底层数据文件中解析出明文数据。pg_snapdump用C/C++编写，为学习postgresql的数据文件格式、数据恢复等提供帮助。



# 特性

安全： 只需要简单的读取权限

全面： 支持postgresql 9.x/10.x/11.x/12.x/13.x，openguass 3.x/5.x/6.x的数据文件解析

简单： 编译不需要依赖任何第三方库

实用： 可以直接解析数据文件中的mvcc老版本数据，在vacuum之前


# 使用方式

## 编译

**Linux**

```shell
cd pg_snapdump/
cmake .
make
cd bin/
```

**MacOS**

```shell
cd pg_snapdump/
cmake .
make
cd bin/
```

## 执行

**Linux**

```shell
./pg_snapdump [[-f /pgdata/data/base/16384/50000] [-o]]
```

**MacOS**

```shell
./pg_snapdump [[-f /pgdata/data/base/16384/50000] [-o]]
```


# 示例

```shell
SQL> update table_name set name = 'this name 2' where id = 2;

SHELL> ./pg_snapdump -f /path/to/pgdata/base/16384/50000 
id: 1 name: User_1 age: 108 class: 20 desc: This is a description for row 1  列读取完毕 ctid(0,1)
id: 2 name: this name 2 age: 108 class: 20 desc: This is a description for row 2 列读取完毕 ctid(0,3)
  -- id: 2 name: User_2 age: 108 class: 20 desc: This is a description for row 2 列读取完毕 ctid(0,3)


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

support range: postgresql/opengauss
