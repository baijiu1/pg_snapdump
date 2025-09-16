//
// Created by 白杰 on 2025/8/13.
//

#include "pg_bst.h"

BSTNode* insertBST(BSTNode* root, size_t block_id, uint16_t offset, CtidNode* value) {
    if (!root) {
        BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
        node->block_id = block_id;
        node->offset = offset;
        node->value = value;
        node->left = node->right = NULL;
        return node;
    }
    if (block_id < root->block_id || (block_id == root->block_id && offset < root->offset)) {
        root->left = insertBST(root->left, block_id, offset, value);
    } else {
        root->right = insertBST(root->right, block_id, offset, value);
    }
    return root;
}


void inorderBST(BSTNode* root) {
    if (!root) return;
    inorderBST(root->left);
    printf("block_id=%zu offset=%u\n", root->block_id, root->offset);
    inorderBST(root->right);
}

BSTTree* createBSTNode() {
    BSTCtidNode *new_node = new BSTCtidNode;
    BSTTree *T = new BSTTree;
    T->nil = new BSTCtidNode;
    T->nil->left = T->nil->right = T->nil->parent = T->nil;
    T->root = T->nil;
    return T;
}
//using BlockNumber = uint32_t;    // 32-bit
//using OffsetNumber = uint16_t;   // 16-bit
BlockNumber ItemPointerGetBlockNumber(const ItemPointerData &ptr) {
    return (ptr.ip_blkid.bi_hi << 16 | ptr.ip_blkid.bi_lo);
}

OffsetNumber ItemPointerGetOffsetNumber(const ItemPointerData &ptr) {
    return ptr.ip_posid;
}
/*
 * a: current ctid
 * b: x->key ctid
 */
static inline int compareItemPointer(const ItemPointerData &a, const ItemPointerData &b) {
    BlockNumber ab = ItemPointerGetBlockNumber(a);
    BlockNumber bb = ItemPointerGetBlockNumber(b);
    OffsetNumber ap = ItemPointerGetOffsetNumber(a);
    OffsetNumber bp = ItemPointerGetOffsetNumber(b);
    if (ab == bb) {
        // block id 相同的情况，比较offset number
        if (ap < bp) {
            return -1;
        } else if (ap > bp) {
            return 1;
        }
    } else if (ab < bb) {
        if (ap < bp) {
            return -1;
        } else if (ap > bp) {
            return 1;
        }
    } else if (ab > bb) {
        if (ap < bp) {
            return -1;
        } else if (ap > bp) {
            return 1;
        }
    }
    return 0;
}
static inline ItemPointerData MakeItemPointer(BlockNumber blk, OffsetNumber pos) {
    ItemPointerData ip{};
    ip.ip_blkid.bi_hi = static_cast<uint16_t>((blk >> 16) & 0xFFFFu);
    ip.ip_blkid.bi_lo = static_cast<uint16_t>(blk & 0xFFFFu);
    ip.ip_posid = pos;
    return ip;
}
void BSTinsert(BSTTree* T, ItemPointerData ctid, CtidNode* node) {
    // 1) BST search to find position or existing key
    BSTCtidNode *y = T->nil;
    BSTCtidNode *x = T->root;
    while (x != T->nil) {
        y = x;
        int cmp = compareItemPointer(ctid, x->key);
        if (cmp == 0) {
            x->key = ctid;
            x->value = node;
        }
        x = (cmp > 0) ? x->left : x->right;
    }

    // 2) Create new red node with single value
    auto *z = new BSTCtidNode;
    z->key = ctid;
    z->value = node;
    z->left = z->right = T->nil;
    z->parent = y;
//    z->color = RED;

    if (y == T->nil) {
        T->root = z; // empty tree
    } else if (compareItemPointer(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    // 3) Fix-up to restore RB properties
//    rbInsertFixup(T, z);
//    return z;
}

void BSTentry(ItemPointerData ctid, CtidNode* node, BSTTree* T) {
    BSTinsert(T, ctid, node);
};

void inorderWalk(BSTTree *T, BSTCtidNode *x) {
    if (x == T->nil) return;
    inorderWalk(T, x->left);
    printf(" (%d, %d)", ItemPointerGetBlockNumber(x->key), ItemPointerGetOffsetNumber(x->key));
    inorderWalk(T, x->right);
    printf(" (%d, %d)", ItemPointerGetBlockNumber(x->key), ItemPointerGetOffsetNumber(x->key));
}

