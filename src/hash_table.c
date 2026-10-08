#include "hash_table.h"

#include <stdlib.h>

static unsigned int hash_id(int id)
{
    return (unsigned int)id % HASH_TABLE_SIZE;
}

void hash_table_init(InternshipHashTable *table)
{
    unsigned int i;
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
}

void hash_table_clear(InternshipHashTable *table)
{
    unsigned int i;
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry != NULL) {
            HashEntry *next = entry->next;
            free(entry);
            entry = next;
        }
        table->buckets[i] = NULL;
    }
}

int hash_table_insert(InternshipHashTable *table, Internship *internship)
{
    unsigned int slot = hash_id(internship->id);
    HashEntry *entry = table->buckets[slot];
    while (entry != NULL) {
        if (entry->id == internship->id) {
            entry->internship = internship;
            return 1;
        }
        entry = entry->next;
    }

    /* Colliding IDs share a bucket through a linked chain. */
    entry = (HashEntry *)malloc(sizeof(*entry));
    if (entry == NULL) {
        return 0;
    }
    entry->id = internship->id;
    entry->internship = internship;
    entry->next = table->buckets[slot];
    table->buckets[slot] = entry;
    return 1;
}

int hash_table_remove(InternshipHashTable *table, int id)
{
    HashEntry **link = &table->buckets[hash_id(id)];
    while (*link != NULL) {
        if ((*link)->id == id) {
            HashEntry *removed = *link;
            *link = removed->next;
            free(removed);
            return 1;
        }
        link = &(*link)->next;
    }
    return 0;
}

Internship *hash_table_find(const InternshipHashTable *table, int id)
{
    const HashEntry *entry = table->buckets[hash_id(id)];
    while (entry != NULL) {
        if (entry->id == id) {
            return entry->internship;
        }
        entry = entry->next;
    }
    return NULL;
}
