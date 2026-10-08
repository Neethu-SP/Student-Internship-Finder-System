#include "stipend_bst.h"

#include <stdlib.h>

static int compare_key(double stipend_a, int id_a, double stipend_b, int id_b)
{
    /* ID is the tie-breaker so equal stipends still have unique tree positions. */
    if (stipend_a < stipend_b) return -1;
    if (stipend_a > stipend_b) return 1;
    if (id_a < id_b) return -1;
    if (id_a > id_b) return 1;
    return 0;
}

int stipend_bst_insert(StipendNode **root, Internship *internship)
{
    StipendNode **link = root;
    StipendNode *node;
    while (*link != NULL) {
        int order = compare_key(internship->stipend, internship->id,
                                (*link)->internship->stipend, (*link)->internship->id);
        if (order == 0) return 1;
        link = order < 0 ? &(*link)->left : &(*link)->right;
    }
    node = (StipendNode *)malloc(sizeof(*node));
    if (node == NULL) return 0;
    node->internship = internship;
    node->left = NULL;
    node->right = NULL;
    *link = node;
    return 1;
}

void stipend_bst_clear(StipendNode *root)
{
    if (root == NULL) return;
    stipend_bst_clear(root->left);
    stipend_bst_clear(root->right);
    free(root);
}

static StipendNode *detach_minimum(StipendNode **root)
{
    StipendNode **link = root;
    while ((*link)->left != NULL) {
        link = &(*link)->left;
    }
    {
        StipendNode *minimum = *link;
        *link = minimum->right;
        minimum->right = NULL;
        return minimum;
    }
}

int stipend_bst_remove(StipendNode **root, int id, double stipend)
{
    StipendNode **link = root;
    while (*link != NULL) {
        int order = compare_key(stipend, id, (*link)->internship->stipend,
                                (*link)->internship->id);
        if (order == 0) {
            StipendNode *removed = *link;
            if (removed->left == NULL) {
                *link = removed->right;
            } else if (removed->right == NULL) {
                *link = removed->left;
            } else {
                StipendNode *successor = detach_minimum(&removed->right);
                successor->left = removed->left;
                successor->right = removed->right;
                *link = successor;
            }
            free(removed);
            return 1;
        }
        link = order < 0 ? &(*link)->left : &(*link)->right;
    }
    return 0;
}

Internship *stipend_bst_find_at_least(StipendNode *root, double minimum_stipend)
{
    Internship *candidate = NULL;
    while (root != NULL) {
        if (root->internship->stipend >= minimum_stipend) {
            candidate = root->internship;
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return candidate;
}

int stipend_bst_collect_at_least(StipendNode *root, double minimum_stipend,
                                 Internship **items, int capacity)
{
    int count = 0;
    if (root == NULL || capacity <= 0) return 0;
    /* In-order traversal returns qualifying internships in stipend order. */
    count += stipend_bst_collect_at_least(root->left, minimum_stipend, items, capacity);
    if (count < capacity && root->internship->stipend >= minimum_stipend) {
        items[count++] = root->internship;
    }
    if (count < capacity) {
        count += stipend_bst_collect_at_least(root->right, minimum_stipend,
                                              items + count, capacity - count);
    }
    return count;
}
