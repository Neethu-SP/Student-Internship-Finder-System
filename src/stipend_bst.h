#ifndef STIPEND_BST_H
#define STIPEND_BST_H

#include "internship.h"

typedef struct StipendNode {
    Internship *internship;
    struct StipendNode *left;
    struct StipendNode *right;
} StipendNode;

int stipend_bst_insert(StipendNode **root, Internship *internship);
void stipend_bst_clear(StipendNode *root);
int stipend_bst_remove(StipendNode **root, int id, double stipend);
Internship *stipend_bst_find_at_least(StipendNode *root, double minimum_stipend);
int stipend_bst_collect_at_least(StipendNode *root, double minimum_stipend,
                                 Internship **items, int capacity);

#endif
