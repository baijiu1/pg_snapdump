//
// Created by 白杰 on 2024/12/16.
//

#include "page.h"
vector<TupleNode*> TupleCtidVec;

bool fetchPage(int fd, int pageNum, char* outPage) {
    void* PageHeap;
    memset(outPage, 0, _PAGESIZE);
    PageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * pageNum);
    if (PageHeap == MAP_FAILED) {
        perror("PageHeap mmap failed.");
        close(fd);
        return false;
    }
    memcpy(outPage, PageHeap, _PAGESIZE);
    munmap(PageHeap, _PAGESIZE);
    return true;
}

off_t fetchFileTotalNum(int fd, int* pageTotalNum) {
    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == 8192) {
        *pageTotalNum = 1; // 有多少页
    } else {
        *pageTotalNum = fileSize / _PAGESIZE;
    }
    return fileSize;
};

vector<string> col;
vector<uint32_t> colType;
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
            if (len == 0) continue;
            char* tuple = new char[len + 1];
            memcpy(tuple, halfPageData + offset, len);
            // pg_class
            if (mode == 0) {
                resolvePgClassHeapData(tuple, tableRelFileNodeId, tableOid);
            } else if (mode == 1) {
                resolvePgAttributeHeapData(tuple, tableOid, colAttr);
            } else if (mode == 2) {
                // table data
                resolveTableHeapTupleData(tuple, pageNum, colAttr, len, offset);
            }
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

#endif

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