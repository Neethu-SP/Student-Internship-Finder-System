#include "application_queue.h"
#include "hash_table.h"
#include "internship.h"
#include "search_sort.h"
#include "stipend_bst.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    InternshipList list;
    InternshipHashTable hash;
    StipendNode *root = NULL;
    ApplicationQueue queue;
    Internship *items[MAX_INTERNSHIPS];
    Internship *matches[MAX_INTERNSHIPS];
    Internship collision;
    Student student = {42, "Asha Student", 2, 7.5, "SQL, Excel, Python"};
    Application first = {0};
    Application second = {0};
    int count, i;

    internship_list_init(&list);
    hash_table_init(&hash);
    application_queue_init(&queue);
    assert(internship_load_file(&list, "data\\internships.txt") == 9);
    count = internship_list_collect(&list, items, MAX_INTERNSHIPS);
    assert(count == 9);

    for (i = 0; i < count; i++) {
        assert(hash_table_insert(&hash, items[i]));
        assert(stipend_bst_insert(&root, items[i]));
    }
    assert(hash_table_find(&hash, 1005) != NULL);
    assert(strcmp(hash_table_find(&hash, 1005)->company, "Orbit Data Works") == 0);
    memset(&collision, 0, sizeof(collision));
    collision.id = 1032;
    assert(collision.id % HASH_TABLE_SIZE == 1001 % HASH_TABLE_SIZE);
    assert(hash_table_insert(&hash, &collision));
    assert(hash_table_find(&hash, 1032) == &collision);
    assert(hash_table_find(&hash, 1001) != NULL);
    assert(hash_table_remove(&hash, 1032));
    assert(student_is_eligible(&student, hash_table_find(&hash, 1001)));
    student.cgpa = 6.9;
    assert(!student_is_eligible(&student, hash_table_find(&hash, 1001)));
    student.cgpa = 7.5;
    (void)snprintf(student.skills, sizeof(student.skills), "%s", "SQL, Excel");
    assert(!student_is_eligible(&student, hash_table_find(&hash, 1001)));

    assert(stipend_bst_collect_at_least(root, 18000, matches, MAX_INTERNSHIPS) == 4);
    assert(stipend_bst_find_at_least(root, 23000)->stipend == 25000);
    assert(search_linear_text(items, count, "robotics", matches, MAX_INTERNSHIPS) == 1);
    assert(strcmp(matches[0]->company, "Bluejay Robotics") == 0);
    sort_by_id(items, count);
    assert(search_binary_id(items, count, 1008) != NULL);
    assert(search_binary_id(items, count, 1099) == NULL);
    sort_by_stipend(items, count);
    assert(items[0]->stipend == 9000);
    sort_by_deadline(items, count);
    assert(strcmp(items[0]->deadline, "2026-11-15") == 0);

    first.student.id = 42;
    first.internship_id = 1001;
    second.student.id = 43;
    second.internship_id = 1002;
    assert(application_queue_enqueue(&queue, &first));
    assert(application_queue_enqueue(&queue, &second));
    assert(application_queue_has_student_application(&queue, 43, 1002));
    assert(application_queue_dequeue(&queue, &first));
    assert(first.internship_id == 1001);
    assert(application_queue_dequeue(&queue, &second));
    assert(second.internship_id == 1002);
    assert(!application_queue_dequeue(&queue, NULL));

    assert(stipend_bst_remove(&root, 1001, hash_table_find(&hash, 1001)->stipend));
    assert(hash_table_remove(&hash, 1001));
    assert(internship_list_delete(&list, 1001));
    assert(hash_table_find(&hash, 1001) == NULL);
    assert(list.count == 8);

    stipend_bst_clear(root);
    hash_table_clear(&hash);
    application_queue_clear(&queue);
    internship_list_clear(&list);
    puts("Core data-structure tests passed.");
    return 0;
}
