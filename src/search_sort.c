#include "search_sort.h"

#include <string.h>

static int contains_text(const char *text, const char *query)
{
    size_t query_length = strlen(query);
    size_t i;
    if (query_length == 0) return 1;
    for (i = 0; text[i] != '\0'; i++) {
        if (_strnicmp(text + i, query, query_length) == 0) return 1;
    }
    return 0;
}

int search_linear_text(Internship **items, int count, const char *query,
                       Internship **matches, int capacity)
{
    int found = 0;
    int i;
    for (i = 0; i < count; i++) {
        const Internship *item = items[i];
        if (contains_text(item->company, query) || contains_text(item->title, query) ||
            contains_text(item->domain, query) || contains_text(item->skills, query)) {
            if (found < capacity) matches[found++] = items[i];
        }
    }
    return found;
}

Internship *search_binary_id(Internship **items, int count, int id)
{
    /* The caller sorts by ID first; each comparison discards half the range. */
    int low = 0;
    int high = count - 1;
    while (low <= high) {
        int middle = low + (high - low) / 2;
        if (items[middle]->id == id) return items[middle];
        if (items[middle]->id < id) low = middle + 1;
        else high = middle - 1;
    }
    return NULL;
}

void sort_by_stipend(Internship **items, int count)
{
    int i;
    for (i = 1; i < count; i++) {
        Internship *key = items[i];
        int j = i - 1;
        while (j >= 0 && items[j]->stipend > key->stipend) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }
}

void sort_by_deadline(Internship **items, int count)
{
    /* ISO YYYY-MM-DD strings sort chronologically with a lexical comparison. */
    int i;
    for (i = 1; i < count; i++) {
        Internship *key = items[i];
        int j = i - 1;
        while (j >= 0 && strcmp(items[j]->deadline, key->deadline) > 0) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }
}

void sort_by_id(Internship **items, int count)
{
    int i;
    for (i = 1; i < count; i++) {
        Internship *key = items[i];
        int j = i - 1;
        while (j >= 0 && items[j]->id > key->id) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = key;
    }
}
