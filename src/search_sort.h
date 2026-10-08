#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "internship.h"

int search_linear_text(Internship **items, int count, const char *query,
                       Internship **matches, int capacity);
Internship *search_binary_id(Internship **items, int count, int id);
void sort_by_stipend(Internship **items, int count);
void sort_by_deadline(Internship **items, int count);
void sort_by_id(Internship **items, int count);

#endif
