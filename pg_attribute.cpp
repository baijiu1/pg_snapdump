//
// Created by 白杰 on 2025/8/8.
//

#include "pg_attribute.h"


using namespace std;
//
//int parserPgSysTableHeaderStruct(char* pageData, unsigned int* tableOid) {
//    uint16_t len, offset, flags;
//    PageHeader HeapHeader = (PageHeader) pageData;
//    int line_count = (HeapHeader->pd_lower - sizeof(PageHeaderData)) / sizeof(PdItemIdData);
//    PdItemId items = (PdItemId)(pageData + sizeof(PageHeaderData));
//    for (int i = 0; i < line_count; i++) {
//        if (ItemIdIsUsed(items[i])) {
//            len = items[i].lp_len;
//            offset = items[i].lp_off;
//            flags = items[i].lp_flags;
//            char* tuple = new char[len + 1];
//            memcpy(tuple, pageData + offset, len);
//            if (len == 0) {
//                continue;
//            }
//            resolvePgAttributeHeapData(tuple, tableOid);
//            delete[] tuple;
//        }
//    }
//    return 0;
//}
//
//int findTableAttribute(int fd, unsigned int * tableOid) {
//    char* HeapPageData;
//    char* HeapPageDataFirst;
//    char* HeapPageDataSecond;
//    void* recoverPageHeap;
//    int num;
//    off_t fileSize = lseek(fd, 0, SEEK_END);
//#ifdef __APPLE__
//    if (fileSize == 8192) {
//        num = 1; // 有多少页
//        HeapPageData = new char[_PAGESIZE];
//    } else {
//        num = fileSize / _PAGESIZE;
//        HeapPageData = new char[_PAGESIZE];
//        HeapPageDataFirst = new char[_PAGESIZE / 2];
//        HeapPageDataSecond = new char[_PAGESIZE / 2];
//    }
//    printf("\n 共读取 %d页 \n", num);
//    for (int i = 0; num == 1 ? i < num : i <= num; ++i) {
////        printf("\n 正在读取 %d页\n", i);
//        if (fileSize == _PAGESIZE * i) {
//            printf(" already read max file size ");
//            break;
//        }
//        recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
//        if (recoverPageHeap == MAP_FAILED) {
//            perror("PageHeaderDATA mmap failed.");
//            close(fd);
//            return 1;
//        }
//        memset(HeapPageData, 0, _PAGESIZE);
//        memset(HeapPageDataFirst, 0, _PAGESIZE / 2);
//        memset(HeapPageDataSecond, 0, _PAGESIZE / 2);
//        memcpy(HeapPageData, recoverPageHeap, _PAGESIZE);
//        memcpy(HeapPageDataFirst, HeapPageData, _PAGESIZE / 2);
//        memcpy(HeapPageDataSecond, HeapPageData + 8192, _PAGESIZE / 2);
//        if (i == 0 && num == 1) {
//            // 解析pg_class
//            parserPgSysTableHeaderStruct(HeapPageDataSecond, tableOid);
//        } else {
//            // 解析pg_class
//            parserPgSysTableHeaderStruct(HeapPageDataFirst, tableOid);
//            parserPgSysTableHeaderStruct(HeapPageDataSecond, tableOid);
//        }
//        if (i == num) {
//            munmap(recoverPageHeap, _PAGESIZE);
//        }
//    }
//    delete[] HeapPageData;
//    delete[] HeapPageDataFirst;
//    delete[] HeapPageDataSecond;
//
//
//#elif defined(__linux__)
//    int num;
//    if (fileSize == 8192) {
//        num = 1; // 有多少页
//        HeapPageData = new char[_PAGESIZE];
//    } else {
//        num = fileSize / _PAGESIZE;
//        HeapPageData = new char[_PAGESIZE];
//        HeapPageDataFirst = new char[_PAGESIZE / 2];
//        HeapPageDataSecond = new char[_PAGESIZE / 2];
//
//    }
//    printf("\n 共读取 %d页 \n", num);
//    for (int i = 0; num == 1 ? i < num : i <= num; ++i) {
//        printf("\n 正在读取 %d页\n", i);
//        if (fileSize == 8192) {
//            recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE);
//            if (recoverPageHeap == MAP_FAILED) {
//                perror("PageHeaderDATA mmap failed.");
//                close(fd);
//                return 1;
//            }
//            memset(HeapPageData, 0, _PAGESIZE);
//            memcpy(HeapPageData, recoverPageHeap, _PAGESIZE);
//            parserHeaderStruct(HeapPageData, fd, tableRelFileNodeId);
//        } else {
//            if (num % 2 != 0 && i == num) {
//                if (fileSize == _PAGESIZE * i) {
//                    printf(" already read max file size ");
//                    break;
//                }
//                recoverPageHeap = mmap(nullptr, 8192, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
//                memset(HeapPageData, 0, _PAGESIZE / 2);
//                memset(HeapPageDataFirst, 0, _PAGESIZE / 2);
////                memset(HeapPageDataSecond, 0, _PAGESIZE / 2);
//                memcpy(HeapPageData, recoverPageHeap, _PAGESIZE / 2);
//                memcpy(HeapPageDataFirst, HeapPageData, _PAGESIZE / 2);
//            } else {
//                recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
//                memset(HeapPageData, 0, _PAGESIZE);
//                memset(HeapPageDataFirst, 0, _PAGESIZE / 2);
//                memset(HeapPageDataSecond, 0, _PAGESIZE / 2);
//                memcpy(HeapPageData, recoverPageHeap, _PAGESIZE);
//                memcpy(HeapPageDataFirst, HeapPageData, _PAGESIZE / 2);
//                memcpy(HeapPageDataSecond, HeapPageData + 8192, _PAGESIZE / 2);
//            }
//            if (recoverPageHeap == MAP_FAILED) {
//                perror("PageHeaderDATA mmap failed.");
//                close(fd);
//                return 1;
//            }
//
//
////            if ((i * 2 + 1) > (num * 2)) {
////                printf("\n读取完毕，读取页数： %u ", num * 2);
////                return 0;
////            }
//
//            if (i == 0 && num == 1) {
//                parserHeaderStruct(HeapPageDataSecond, fd, tableRelFileNodeId);
//            } else {
//                parserHeaderStruct(HeapPageDataFirst, fd, tableRelFileNodeId);
//                parserHeaderStruct(HeapPageDataSecond, fd, tableRelFileNodeId);
//            }
//        }
//
//        if (i == num) {
//            munmap(recoverPageHeap, _PAGESIZE);
//        }
//    }
//    delete[] HeapPageData;
//    delete[] HeapPageDataFirst;
//    delete[] HeapPageDataSecond;
//    tableOnDiskClose(fd, 100);
//#endif
//    return 0;
//
//};

/*
 * 解析pg_class/pg_attribute的header数据
 * parser pg_class to get table oid
 */
int resolvePgAttributeHeapData(char* tuple, unsigned int* tableOid, ColAttribute& colAttr) {
    HeapTupleHeaders tup = (HeapTupleHeaders)tuple;
    char* t_data = tuple + tup->t_hoff;
    Form_pg_attribute pgAttributeData = (Form_pg_attribute) t_data;
    if (pgAttributeData->attrelid == *tableOid && tup->t_choice.t_heap.t_xmax == 0) {
        if (strcmp("ctid", pgAttributeData->attname.data) != 0 &&
            strcmp("xmin", pgAttributeData->attname.data) != 0 &&
            strcmp("cmin", pgAttributeData->attname.data) != 0 &&
            strcmp("xmax", pgAttributeData->attname.data) != 0 &&
            strcmp("cmax", pgAttributeData->attname.data) != 0 &&
            strcmp("tableoid", pgAttributeData->attname.data) != 0) {
            // 记录好attnum，然后排序

//            for (int i = 0; i < colAttr.colName.size(); ++i) {
//                if (colAttr.colName[i] == pgAttributeData->attname.data) {
//                    continue;
//                } else {
                    colAttr.colAttNum.emplace_back(pgAttributeData->attnum);
                    colAttr.colName.emplace_back(pgAttributeData->attname.data);
                    colAttr.colType.emplace_back(pgAttributeData->atttypid);
                    colAttr.colAttlen.emplace_back(pgAttributeData->attlen);
                    colAttr.colAttalign.emplace_back(pgAttributeData->attalign);
//                }
//            }

        }
    }
    return 0;
};