//
// Created by 白杰 on 2024/12/17.
// 拿到header的全局map后，根据pd_linp的偏移量，取具体的元组
// 元组中计算该元组是否是老版本
//

#include "access.h"


using namespace std;

int fileCount = 0;
extern bool parserAllFiles;

int InitAccessForDBAndTableName(const string& dbName, const string& tableName) {
//    vector<string> tabList;
//    tabList.clear();
////    InitTable(tabList, dbName, tableName);
//    InitTable(tabList, dbName, tableName);
}

static int starts_with(const char *name, const char *prefix) {
    return strncmp(name, prefix, strlen(prefix)) == 0;
}

/*
 * according to give oid to find file count for data file. like oid.1 oid.2 ...
 */
int getTableFileCount(char* oidOath, const char* oidName) {
    DIR *dir = opendir(oidOath);
    if (!dir) {
        LOG(LOG_LEVEL_FATAL, "open dir failed: %s", oidOath);
        return -1;
    }

    struct dirent *ent;
    long count = 0;
    char pathbuf[4096];

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;

        if (!starts_with(ent->d_name, oidName))
            continue;

        int n = snprintf(pathbuf, sizeof(pathbuf), "%s/%s", oidOath, ent->d_name);
        if (n < 0 || n >= (int)sizeof(pathbuf)) continue;

        struct stat st;
        if (lstat(pathbuf, &st) == 0 && S_ISREG(st.st_mode)) {
            ++count;
        }
    }

    closedir(dir);
    return count;
};

int tableOnDiskOpen(const char* DBFilePath, int lockMode) {
    int fd = open(DBFilePath, O_RDONLY);
    if (fd == -1) {
        LOG(LOG_LEVEL_FATAL, "error open file: %s, exit...", DBFilePath);
        return -1;
    }
    return fd;
}

int tableOnDiskStates(int fd, int lockMode) {
    // 检查文件描述符状态
    if (fd >= 0) {
//        std::cout << "File descriptor " << fd << " is valid." << std::endl;
        return 1;
    } else {
//        std::cout << "File descriptor " << fd << " is invalid." << std::endl;
        return fd;
    }
}

int tableOnDiskClose(int fd, int lockMode) {
    return close(fd);
}

static inline int get_default_concurrency(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n < 1) n = 1;
    // 可根据负载调整并发倍数；通常 = CPU 核心数 或 2x
    if (n > 64) n = 64;          // 保险上限，防止意外
    return (int)n;
}

int process_files_concurrent(const char* basePath,
                             const char* baseOidName,
                             unsigned fileCount,
                             int groupSize,           // 每个子进程处理多少个文件
                             int maxProcs_opt)        // 最大并发进程数（<=0 则自动）
{
    if (groupSize <= 0) groupSize = 10;
    int maxProcs = (maxProcs_opt > 0) ? maxProcs_opt : get_default_concurrency();

    // 总分组数
    unsigned totalGroups = (fileCount + groupSize - 1u) / (unsigned)groupSize;

    unsigned nextGroup = 0;   // 下一组索引
    int active = 0;           // 正在运行的子进程数
    int exit_code = 0;        // 父进程最终返回码

    while (nextGroup < totalGroups || active > 0) {
        // 还能再起进程，就fork一批
        if (active < maxProcs && nextGroup < totalGroups) {
            unsigned g = nextGroup++;
            pid_t pid = fork();
            if (pid < 0) {
                // fork 失败：可以选择稍等或降并发；这里直接报错并进入回收阶段
                perror("fork");
                exit_code = 1;
                break;
            }
            if (pid == 0) {
                // ---------- 子进程 ----------
                unsigned start = g * (unsigned)groupSize;
                unsigned end   = start + (unsigned)groupSize;
                if (end > fileCount) end = fileCount;

                // 子进程自己处理 [start, end)
                for (unsigned j = start; j < end; ++j) {
                    // 组装路径：basePath + "." + j
                    char oid_path[PATH_MAX];
                    int n = snprintf(oid_path, sizeof(oid_path), "%s/%s.%u", basePath, baseOidName, j);
                    if (n < 0 || (size_t)n >= sizeof(oid_path)) {
                        fprintf(stderr, "[child %d] path too long for j=%u\n", getpid(), j);
                        // 出错可选择继续/跳过/退出，这里继续
                        continue;
                    }

                    int fd = tableOnDiskOpen(oid_path, 10);
                    if (fd < 0) {
                        fprintf(stderr, "[child %d] open failed: %s\n", getpid(), oid_path);
                        continue;
                    }

                    findTableData(fd, "1", NULL, 2);
                    tableOnDiskClose(fd, 100);
                }
                _exit(0); // 使用 _exit 避免 stdio 缓冲在多进程下重复刷新
            } else {
                // ---------- 父进程 ----------
                active++;
                continue; // 尝试继续拉起更多子进程，直到达到并发上限
            }
        }

        // 没法继续拉起进程（已达并发上限或没有剩余任务），就回收一个
        int status = 0;
        pid_t w = waitpid(-1, &status, 0);
        if (w == -1) {
            if (errno == EINTR) continue; // 被信号打断，重来
            perror("waitpid");
            exit_code = 1;
            break;
        }
        active--;
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) {
                fprintf(stderr, "Child pid=%d exit code=%d\n", (int)w, code);
                exit_code = 1;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child pid=%d killed by signal %d\n", (int)w, WTERMSIG(status));
            exit_code = 1;
        }
    }

    // 把可能还存活的子进程都收掉（极端错误情况下）
    while (active > 0) {
        int status;
        pid_t w = waitpid(-1, &status, 0);
        if (w <= 0) break;
        active--;
    }

    if (exit_code == 0) {
        printf("All files processed! groups=%u, groupSize=%d, maxProcs=%d\n",
               totalGroups, groupSize, maxProcs);
    }
    return exit_code;
}


int InitAccessForProcessRecover(char* oid) {
    int fd;
    void* FilenodeMap;
    char* fileNodeStream;
    char* path;
    size_t pathLen;
    const char* tableRelFileNodeId;
    uint32_t pgClassNode, pgAttributeNode;
    unsigned int* tableOid;

    tableOid = new unsigned int(0);
    const char *lastSlash = strrchr(oid, '/');
    path = new char[300];
    memset(path, 0, 300);
    if (lastSlash != NULL) {
        // lastSlash 指向最后一个 '/'，取后面一个字符开始的子串
        tableRelFileNodeId = lastSlash + 1;
        pathLen = lastSlash - oid; // 计算长度，不包含最后的 '/'
        memcpy(path, oid, pathLen);
    }
    strcat(path, "/pg_filenode.map");
    LOG(LOG_LEVEL_DEBUG, "open file %s ", path);
    fd = tableOnDiskOpen(path, 10);
    struct stat st;
    if (fstat(fd, &st) != 0) {
        LOG(LOG_LEVEL_FATAL, "fstat file %s failed, exit... ", path);
        return -1;
    }
    size_t mapSize = st.st_size < _PAGESIZE ? st.st_size : _PAGESIZE;
    LOG(LOG_LEVEL_DEBUG, "mmap file %s, file size: %zu ", path, mapSize);
    FilenodeMap = mmap(nullptr, mapSize, PROT_READ, MAP_PRIVATE, fd, 0);
    fileNodeStream = new char[_PAGESIZE];
    memset(fileNodeStream, 0, _PAGESIZE);
    memcpy(fileNodeStream, FilenodeMap, mapSize);
    munmap(FilenodeMap, _PAGESIZE);
    tableOnDiskClose(fd, 10);
    LOG(LOG_LEVEL_DEBUG, "closed pg_filenode.map file %s ", path);

    RelFileNodeData fileNode = (RelFileNodeData) fileNodeStream;
    for (int i = 0; i < 60; ++i) {
        if (fileNode[i].rel_oid == 1259) {
            pgClassNode = fileNode[i].node_id;
            LOG(LOG_LEVEL_DEBUG, "find pg_class oid: %d ", pgClassNode);
        } else if (fileNode[i].rel_oid == 1249) {
            pgAttributeNode = fileNode[i].node_id;
            LOG(LOG_LEVEL_DEBUG, "find pg_attribute oid: %d ", pgAttributeNode);
        }
    }
    delete[] fileNodeStream;

    char pg_class_path[130];
    char buf[30];
    memcpy(pg_class_path, path, pathLen);
    pg_class_path[pathLen] = '\0';
    snprintf(buf, sizeof(buf), "/%u", pgClassNode);
    snprintf(pg_class_path, sizeof(pg_class_path), "%.*s/%u", pathLen, path, pgClassNode);
    LOG(LOG_LEVEL_DEBUG, "find pg_class path: %s ", pg_class_path);

    LOG(LOG_LEVEL_DEBUG, "open pg_class data file %s ", pg_class_path);
    fd = tableOnDiskOpen(pg_class_path, 10);
    LOG(LOG_LEVEL_DEBUG, "find table_oid from pg_class file: %s ", pg_class_path);
    findTableData(fd, tableRelFileNodeId, tableOid, 0);
    if (*tableOid) {
        LOG(LOG_LEVEL_DEBUG, "find table_oid: %d from pg_class data file: %s ", *tableOid, pg_class_path);
    }
    tableOnDiskClose(fd, 100);
    LOG(LOG_LEVEL_DEBUG, "closed pg_class data file %s ", pg_class_path);


    char pg_attribute_path[330];
    char buf1[30];
    memcpy(pg_attribute_path, path, pathLen);
    pg_attribute_path[pathLen] = '\0';
    snprintf(buf1, sizeof(buf1), "/%u", pgAttributeNode);
    snprintf(pg_attribute_path, sizeof(pg_attribute_path), "%.*s/%u", pathLen, path, pgAttributeNode);
    LOG(LOG_LEVEL_DEBUG, "find pg_attribute path: %s ", pg_class_path);

    LOG(LOG_LEVEL_DEBUG, "open pg_attribute data file %s ", pg_attribute_path);
    fd = tableOnDiskOpen(pg_attribute_path, 10);
    LOG(LOG_LEVEL_DEBUG, "find table attribute from pg_attribute data file: %s ", pg_attribute_path);
    findTableData(fd, "0", tableOid, 1);
    tableOnDiskClose(fd, 100);
    LOG(LOG_LEVEL_DEBUG, "closed pg_attribute data file %s ", pg_attribute_path);

    int groupSize1 = 10;
    int forkCount1 = (15 + groupSize1 - 1) / groupSize1;

    char oid_path[300];
    memcpy(oid_path, path, pathLen);
    char* oidName = oid + pathLen + 1;
    LOG(LOG_LEVEL_DEBUG, "get parser table data file count.");
    fileCount = getTableFileCount(oid_path, oidName);
    LOG(LOG_LEVEL_DEBUG, "data file count: %d", fileCount);

//    SharedCtidNodeVector* shmCtidBase = create_shared_vector(fileCount * 131072);

//    if (parserAllFiles) {
//        if (fileCount == 1) {
//            fd = tableOnDiskOpen(oid, 10);
//            findTableData(fd, "1", nullptr, 2);
//            tableOnDiskClose(fd, 100);
//        } else {
//            if (fileCount < 10) {
//                for (int i = 1; i <= fileCount; ++i) {
//                    char buff[100];
//                    memcpy(oid_path, path, pathLen);
//                    oidName =  oid + pathLen;
//                    snprintf(buff, sizeof(buff), ".%u", i);
//                    strcat(oidName, buff);
//                    strcat(oid_path, oidName);
//                    fd = tableOnDiskOpen(oid_path, 10);
//                    findTableData(fd, "1", nullptr, 2);
//                    tableOnDiskClose(fd, 100);
//                }
//            } else {
//                process_files_concurrent(oid_path, oidName, fileCount, 10, 0);
//
//                printf("All files processed!\n");
//
//            }
//        }
//    } else {
    LOG(LOG_LEVEL_DEBUG, "open parser table data file: %s.", oid);
        fd = tableOnDiskOpen(oid, 10);
        findTableData(fd, "1", nullptr, 2);
        tableOnDiskClose(fd, 100);
    LOG(LOG_LEVEL_DEBUG, "closed parser table data file: %s.", oid);
//    }
//    printAllCtidChain();
    delete tableOid;
    delete[] path;
    return 1;

};

int findTableData(int fd, const char *tableRelFileNodeId, unsigned int* tableOid, int mode){
    char* pageData;
    int* pageTotalNum;
#if defined(__APPLE__)
    pageData = new char[_PAGESIZE];
#elif (defined(__linux__) && defined(__aarch64__)) || (defined(__linux__) && defined(__x86_64__)) || (defined(__linux__) && defined(__i386__))
    pageData = new char[_PAGESIZE * 2];
#endif
    pageTotalNum = new int(0);
    off_t fileSize = fetchFileTotalNum(fd, pageTotalNum);
    LOG(LOG_LEVEL_DEBUG, "get data file page count: %lld.", fileSize);

#if defined(__APPLE__)
    for (int i = 0; *pageTotalNum == 1 ? i < *pageTotalNum : i <= *pageTotalNum; ++i) {
#elif (defined(__linux__) && defined(__aarch64__)) || (defined(__linux__) && defined(__x86_64__)) || (defined(__linux__) && defined(__i386__))
        for (int i = 0; *pageTotalNum == 2 ? i < 1 : i <= (*pageTotalNum / 2); i += 2) {
#endif
        if (i * _PAGESIZE >= fileSize) {
            LOG(LOG_LEVEL_DEBUG, "finished reading data file page.");
            return 0;
        }
        fetchPage(fd, i, pageData, fileSize);
        fetchPageData(pageData, i, tableRelFileNodeId, tableOid, fileSize, mode);
    }
    delete pageTotalNum;
    delete[] pageData;
    return 0;
};