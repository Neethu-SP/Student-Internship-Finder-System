#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "internship.h"

#define HASH_TABLE_SIZE 31

typedef struct HashEntry {
    int id;
    Internship *internship;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry *buckets[HASH_TABLE_SIZE];
} InternshipHashTable;

void hash_table_init(InternshipHashTable *table);
void hash_table_clear(InternshipHashTable *table);
int hash_table_insert(InternshipHashTable *table, Internship *internship);
int hash_table_remove(InternshipHashTable *table, int id);
Internship *hash_table_find(const InternshipHashTable *table, int id);

#endif
