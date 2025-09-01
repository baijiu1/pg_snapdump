//
// Created by 白杰 on 2024/12/17.
//

#ifndef MYSQL_REPLICATER_ACCESS_H
#define MYSQL_REPLICATER_ACCESS_H
#include "string"
#include "pg_shm.h"
#include "page.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;


//extern int InitTable(vector<string>&, const string&, const string&);
//extern int processRecover(string&);
int processConstructRecover(string& stream_parser_file_name);
int tableOnDiskOpen(string& DBFilePath, int lockMode);
int tableOnDiskStates(int fd, int lockMode);
int tableOnDiskClose(int fd, int lockMode);
int InitAccess();
int InitAccessForFile(const string&);

int InitAccessForDBAndTableName(const string&, const string&);
int InitAccessForProcessRecover(char*);
int findTableData(int fd, const char *, unsigned int*, int);
int InitAccessForCParserFile(string);

int process_files_concurrent(const char* basePath,
                             const char*,
                             unsigned fileCount,
                             int groupSize,           // 每个子进程处理多少个文件
                             int maxProcs_opt);        // 最大并发进程数（<=0 则自动


#endif //MYSQL_REPLICATER_ACCESS_H
