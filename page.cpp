//
// Created by baijiu1 on 2024/12/16.
//

#include "page.h"


bool fetchPage(int fd, int pageNum, char* outPage, off_t fileSize) {
    void* PageHeap;
    if (fd < 0 || outPage == nullptr) return false;
    if (_PAGESIZE * pageNum >= fileSize) return false;

#if defined(__APPLE__)
    PageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * pageNum);
    if (PageHeap == MAP_FAILED) {
        perror("PageHeap mmap failed.");
//        close(fd);
        return false;
    }
    memset(outPage, 0, _PAGESIZE);
    memcpy(outPage, PageHeap, _PAGESIZE);
    munmap(PageHeap, _PAGESIZE);
#elif (defined(__linux__) && defined(__aarch64__)) || (defined(__linux__) && defined(__x86_64__)) || (defined(__linux__) && defined(__i386__))
    PageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * pageNum);
    if (PageHeap == MAP_FAILED) {
        perror("PageHeap mmap failed.");
//        close(fd);
        return false;
    }
    memset(outPage, 0, _PAGESIZE * 2);
    memcpy(outPage, PageHeap, _PAGESIZE);
    munmap(PageHeap, _PAGESIZE);

    PageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * (pageNum + 1));
    if (PageHeap == MAP_FAILED) {
        perror("PageHeap mmap failed.");
//        close(fd);
        return false;
    }
    memcpy(outPage + _PAGESIZE, PageHeap, _PAGESIZE);
    munmap(PageHeap, _PAGESIZE);
#endif

    return true;
}

off_t fetchFileTotalNum(int fd, int* pageTotalNum) {
    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == 8192) {
#if defined(__APPLE__)
        *pageTotalNum = 1; // 有多少页
#elif (defined(__linux__) && defined(__aarch64__)) || (defined(__linux__) && defined(__x86_64__)) || (defined(__linux__) && defined(__i386__))
        *pageTotalNum = 2; // 有多少页
#endif
    } else {
        *pageTotalNum = fileSize / _PAGESIZE;
    }
    return fileSize;
};

ColAttribute colAttr = {};
bool processHalfPage(char* halfPageData, int pageNum, const char * tableRelFileNodeId, unsigned int* tableOid, int mode) {
    PageHeader header = (PageHeader)halfPageData;
    int line_count = (header->pd_lower - sizeof(PageHeaderData)) / sizeof(PdItemIdData);
    PdItemId items = (PdItemId)(halfPageData + sizeof(PageHeaderData));
    for (int i = 0; i < line_count; i++) {
        if (ItemIdIsUsed(items[i])) {
            uint16_t len = items[i].lp_len;
            uint16_t offset = items[i].lp_off;
            uint16_t flags = items[i].lp_flags;
            if ((len == 0) || (len >= 8192)) continue;
            if (offset >= 8192) continue;
            char* tuple = new char[len + 1];
            memset(tuple, 0, len + 1);
            memcpy(tuple, halfPageData + offset, len);
            // pg_class
            if (mode == 0) {
                resolvePgClassHeapData(tuple, tableRelFileNodeId, tableOid);
            } else if (mode == 1) {
                resolvePgAttributeHeapData(tuple, tableOid, colAttr);
            } else if (mode == 2) {
                if (!colAttr.colName.empty()) {
                    resolveTableHeapTupleData(tuple, pageNum, colAttr, len, offset);
                } else {
                    LOG(LOG_LEVEL_FATAL, "resolve pg_attribute column attr failed. exit...");
                    exit(1);
                }
            }
            delete[] tuple;
        }
    }
    return true;
}

bool fetchPageData(char* pageData, int thisPageNum, const char * tableRelFileNodeId, unsigned int* tableOid, off_t fileSize, int mode) {
#ifdef __APPLE__
    char* pageDataFirst = new char[_PAGESIZE / 2];
    memcpy(pageDataFirst, pageData, _PAGESIZE / 2);
    processHalfPage(pageDataFirst, thisPageNum * 2, tableRelFileNodeId, tableOid, mode);
    if (fileSize != 8192) {
        char* pageDataSecond = new char[_PAGESIZE / 2];
        memcpy(pageDataSecond, pageData + (_PAGESIZE / 2), _PAGESIZE / 2);
        processHalfPage(pageDataSecond, thisPageNum * 3, tableRelFileNodeId, tableOid, mode);
        delete[] pageDataSecond;
    }
    delete[] pageDataFirst;

#elif defined(__linux__)
    processHalfPage(pageData, thisPageNum, tableRelFileNodeId, tableOid, mode);
#endif
    return true;
};


TupleNode* reverseList(TupleNode* head) {
    TupleNode* prev = nullptr;
    TupleNode* curr = head;
    while (curr) {
        TupleNode* nextTemp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = nextTemp;
    }
    return prev;
}

void reverseAllChains(std::vector<TupleNode*>& chains) {
    for (size_t i = 0; i < chains.size(); ++i) {
        chains[i] = reverseList(chains[i]);
    }
}