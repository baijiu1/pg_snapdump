//
// Created by 白杰 on 2025/8/8.
//

#include "pg_data.h"

vector<CtidNode*> CtidVec;


// 创建新节点
CtidNode* create_ctid_node(const chaseCtidList& chaseTuple) {
    CtidNode* node = new CtidNode();
    node->tuple = chaseTuple;
    node->next = nullptr;
    return node;
}

// 插入或构建链表
void append_to_ctid_chain(const chaseCtidList& chaseTupleNode) {
    CtidNode* new_node = create_ctid_node(chaseTupleNode);
    bool inserted = false;

    if (CtidVec.empty()) {
        CtidVec.push_back(new_node);
        return;
    }

    for (size_t i = 0; i < CtidVec.size(); ++i) {
        CtidNode* head = CtidVec[i];
        CtidNode* prev = nullptr;
        CtidNode* current = head;

        while (current != nullptr) {
            // 当前的链表为旧记录
            if (current->tuple.xmax == new_node->tuple.xmin) {
                new_node->next = current->next;
                current->next = new_node;
                inserted = true;
                return;
            }

            // 新节点的 xmax == current 的 xmin，表示 current 更新了 new_node
            if (new_node->tuple.xmax == current->tuple.xmin) {
                if (prev == nullptr) {
                    // 插入为头节点
                    new_node->next = head;
                    CtidVec[i] = new_node;
                } else {
                    new_node->next = current;
                    prev->next = new_node;
                }
                inserted = true;
                return;
            }
            prev = current;
            current = current->next;

        }
    }
    if (!inserted) {
        CtidVec.push_back(new_node);
    }
}


int resolveTableHeapTupleData(char* tuple, int thisPageNum, ColAttribute& colAttr, uint16_t len, uint16_t tupleOffset) {
    char*               t_data;
    uint16_t            t_infomask;
    uint16_t            t_infomask2;
    bool                hasNull;
    uint16_t            natts;
    char*              t_bits;
    uint32_t            t_hoff;
    uint32_t            currentTupleBlockId, currentTupleOffset;
    ItemPointerData ctid;

    chaseCtidList chaseTuple = {0};
    HeapTupleHeaders tup = (HeapTupleHeaders) tuple;
    t_data = tuple + tup->t_hoff;
    t_infomask2 = tup->t_infomask2;
    t_hoff = tup->t_hoff;
    natts = HeapTupleHeaderGetNatts(t_infomask2);
    hasNull = HeapTupleHasNulls(tup);
    ctid = tup->t_ctid;
    currentTupleBlockId = (ctid.ip_blkid.bi_hi << 16) | ctid.ip_blkid.bi_lo;
    currentTupleOffset = ctid.ip_posid;

    t_bits = new char[t_hoff - 23];
    memcpy(t_bits, tuple + 23, t_hoff - 23);
    colAttr.bitMaps.assign(natts, 1);
    if (hasNull) {
        for (int j = 0; j < natts; ++j) {
            if (att_isnull(j, t_bits)) {
                colAttr.bitMaps[j] = 0;
            }
        }
    }
    delete t_bits;
    // sort for colAttr.colAttnum
    sort_with_sync(colAttr.colAttNum, colAttr.colName, colAttr.colType, colAttr.colAttalign, colAttr.colAttlen);
    chaseTuple.self_ctid.ip_blkid.bi_hi = ctid.ip_blkid.bi_hi;
    chaseTuple.self_ctid.ip_blkid.bi_lo = ctid.ip_blkid.bi_lo;
    chaseTuple.tuple_block_id = currentTupleBlockId;
    chaseTuple.self_ctid.ip_posid = ctid.ip_posid;
    chaseTuple.xmax = tup->t_choice.t_heap.t_xmax;
    chaseTuple.xmin = tup->t_choice.t_heap.t_xmin;
    chaseTuple.tuple_nattrs = HeapTupleHeaderGetNatts(t_infomask2);
    chaseTuple.column_name = colAttr.colName;
    chaseTuple.null_bit_map = colAttr.bitMaps;
    chaseTuple.column_type_id = colAttr.colType;
    chaseTuple.colAttlen = colAttr.colAttlen;
    chaseTuple.colAttalign = colAttr.colAttalign;

    if (thisPageNum == currentTupleBlockId) {
        chaseTuple.tuple_length = len - t_hoff;
        chaseTuple.tuple_offset = tupleOffset;
        fileCount < 10 ? chaseTuple.cache_data = t_data : chaseTuple.cache_data = nullptr;
        CtidNode *node = new CtidNode;
        node->tuple = chaseTuple;
        fetchRows(node);
        delete node;
//        append_to_ctid_chain(chaseTuple);
        return 0;
    } else {
        if (currentTupleBlockId != thisPageNum) {
            if (currentTupleBlockId > MAX_PAGE_COUNT) {
                chaseTuple.tuple_length = len - t_hoff;
                chaseTuple.tuple_offset = tupleOffset;
                fileCount < 10 ? chaseTuple.cache_data = t_data : chaseTuple.cache_data = nullptr;
                return 0;
            } else {
                chaseTuple.tuple_length = len - t_hoff;
                chaseTuple.tuple_offset = tupleOffset;
                fileCount < 10 ? chaseTuple.cache_data = t_data : chaseTuple.cache_data = nullptr;
                CtidNode *node = new CtidNode;
                node->tuple = chaseTuple;
                fetchRows(node);
                delete node;
//                append_to_ctid_chain(chaseTuple);
                return 0;
            }
        } else {
            chaseTuple.tuple_length = len - t_hoff;
            chaseTuple.tuple_offset = tupleOffset;
            fileCount < 10 ? chaseTuple.cache_data = t_data : chaseTuple.cache_data = nullptr;
            CtidNode *node = new CtidNode;
            node->tuple = chaseTuple;
            fetchRows(node);
            delete node;
//            append_to_ctid_chain(chaseTuple);
            return 0;
        }
    }

};

CtidNode* reverseList(CtidNode* head) {
    CtidNode* prev = nullptr;
    CtidNode* curr = head;
    while (curr) {
        CtidNode* nextTemp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = nextTemp;
    }
    return prev;
}

void reverseAllChains(std::vector<CtidNode*>& chains) {
    for (size_t i = 0; i < chains.size(); ++i) {
        chains[i] = reverseList(chains[i]);
    }
}

//int fetchRowsForSingle(chaseCtidList& tuples) {
//    uint32_t * offset = new uint32_t;
//    *offset = 0;
//
//    for (int i = 0; i < tuples.tuple_nattrs; ++i) {
//        if (tuples.null_bit_map[i] == 0) {
//            printf(" %s: is null ", tuples.column_name[i].c_str());
//            // if null skip
//            continue;
//        }
//        printf(" %s:", tuples.column_name[i].c_str());
//        tupleFetchType(tuples, i, offset);
//        if (*offset >= tuples.tuple_length) {
//            printf(" 列读取完毕 ");
//            break;
//        }
//    }
//    delete offset;
//}

int fetchRows(CtidNode* tuple) {
    uint32_t * offset = new uint32_t;
    *offset = 0;

    for (int i = 0; i < tuple->tuple.tuple_nattrs; ++i) {
        if (onlyNewTuple) {
            if (tuple->tuple.xmax != 0) {
                continue;
            } else {
                if (tuple->tuple.null_bit_map[i] == 0) {
                    printf(" %s: is null ", tuple->tuple.column_name[i].c_str());
                    // if null skip
                    continue;
                }
                printf(" %s:", tuple->tuple.column_name[i].c_str());
                tupleFetchType(tuple, i, offset);
            }
        } else {
            if (tuple->tuple.null_bit_map[i] == 0) {
                printf(" %s: is null ", tuple->tuple.column_name[i].c_str());
                // if null skip
                continue;
            }
            printf(" %s:", tuple->tuple.column_name[i].c_str());
            tupleFetchType(tuple, i, offset);
        }

        if (*offset >= tuple->tuple.tuple_length) {
            printf(" 列读取完毕 \n");
            break;
        }
    }
    delete offset;

};

int makeCtidBSTNode(ItemPointerData ctid, CtidNode* node, BSTTree* T) {
    BSTinsert(T, ctid, node);
}

int printAllCtidChain() {
    printf("\n+++++++++++++++++\n");
    // 反转数据内容
    reverseAllChains(CtidVec);
    BSTTree* T = createBSTNode();
    for (size_t i = 0; i < CtidVec.size(); ++i) {
        CtidNode* current = CtidVec[i];
        makeCtidBSTNode(current->tuple.self_ctid, current, T);
        while (current != nullptr) {
            if (current->tuple.xmax == 0) {
                printf("");
            } else {
                if (onlyNewTuple) {
                    printf("");
                } else {
                    printf("   -- ");
                }
            }

            if (fileCount < 10) {
                // if table data file is small file(< 10G), then chaseCtidList can cache tuple data
                // fetchRows() read cache data of struct chaseCtidList to find each type to resolve data content.
                fetchRows(current);
            } else {
                // if table data file is large file(more than 10G)
//                makeCtidBSTNode();
            }
            if (onlyNewTuple) {
                if (current->tuple.xmax != 0) {
                    printf("");
                } else {
                    printf("  ctid=(%u,%u) \n",
                           (current->tuple.self_ctid.ip_blkid.bi_hi << 16) | current->tuple.self_ctid.ip_blkid.bi_lo,
                           current->tuple.self_ctid.ip_posid);
                }
            } else {
                printf("  ctid=(%u,%u) \n",
                       (current->tuple.self_ctid.ip_blkid.bi_hi << 16) | current->tuple.self_ctid.ip_blkid.bi_lo,
                       current->tuple.self_ctid.ip_posid);
            }

            current = current->next;
        }
    }
//    inorderWalk(T, T->root);
    printf("\n+++++++++++++++++\n");
}

