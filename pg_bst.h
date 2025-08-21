//
// Created by 白杰 on 2025/8/13.
//

#ifndef PG_SNAPDUMP_PG_BST_H
#define PG_SNAPDUMP_PG_BST_H
#include "pg_common.h"

void BSTentry(ItemPointerData ctid, CtidNode* node, BSTTree*);
void BSTinsert(BSTTree* T, ItemPointerData ctid, CtidNode* node);
//void inorder(BSTTree *T, BSTCtidNode *n);
BSTTree* createBSTNode();
void inorderBST(BSTNode* root);
BSTNode* insertBST(BSTNode* root, size_t block_id, uint16_t offset, CtidNode* value);
void inorderWalk(BSTTree *T, BSTCtidNode *x);

#endif //PG_SNAPDUMP_PG_BST_H
