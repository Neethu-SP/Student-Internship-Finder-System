#include "internship.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void internship_list_init(InternshipList *list)
{
    list->head = NULL;
    list->count = 0;
}

void internship_list_clear(InternshipList *list)
{
    InternshipNode *node = list->head;
    while (node != NULL) {
        InternshipNode *next = node->next;
        free(node);
        node = next;
    }
    list->head = NULL;
    list->count = 0;
}

int internship_list_add(InternshipList *list, const Internship *internship)
{
    InternshipNode *node;
    InternshipNode **tail;

    if (list == NULL || internship == NULL || internship->id <= 0 ||
        list->count >= MAX_INTERNSHIPS || internship_list_find(list, internship->id) != NULL) {
        return 0;
    }

    node = (InternshipNode *)malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }
    node->data = *internship;
    node->next = NULL;

    /* Append to preserve the linked list's insertion order. */
    tail = &list->head;
    while (*tail != NULL) {
        tail = &(*tail)->next;
    }
    *tail = node;
    list->count++;
    return 1;
}

int internship_list_delete(InternshipList *list, int id)
{
    InternshipNode **link;

    if (list == NULL) {
        return 0;
    }
    link = &list->head;
    while (*link != NULL) {
        if ((*link)->data.id == id) {
            InternshipNode *removed = *link;
            *link = removed->next;
            free(removed);
            list->count--;
            return 1;
        }
        link = &(*link)->next;
    }
    return 0;
}

Internship *internship_list_find(InternshipList *list, int id)
{
    InternshipNode *node;
    for (node = list != NULL ? list->head : NULL; node != NULL; node = node->next) {
        if (node->data.id == id) {
            return &node->data;
        }
    }
    return NULL;
}

int internship_list_collect(InternshipList *list, Internship **items, int capacity)
{
    InternshipNode *node;
    int count = 0;
    for (node = list != NULL ? list->head : NULL; node != NULL && count < capacity;
         node = node->next) {
        items[count++] = &node->data;
    }
    return count;
}

int internship_load_file(InternshipList *list, const char *path)
{
    FILE *file = fopen(path, "r");
    char line[640];
    int loaded = 0;

    if (file == NULL) {
        return -1;
    }

    internship_list_clear(list);
    while (fgets(line, sizeof(line), file) != NULL) {
        Internship item;
        int fields = sscanf(line, "%d|%63[^|]|%79[^|]|%47[^|]|%127[^|]|%d|%lf|%lf|%10[^\r\n]",
                            &item.id, item.company, item.title, item.domain, item.skills,
                            &item.duration_months, &item.stipend, &item.min_cgpa, item.deadline);
        if (fields == 9 && internship_list_add(list, &item)) {
            loaded++;
        }
    }
    fclose(file);
    return loaded;
}

int internship_save_file(const InternshipList *list, const char *path)
{
    FILE *file = fopen(path, "w");
    const InternshipNode *node;

    if (file == NULL) {
        return 0;
    }
    for (node = list->head; node != NULL; node = node->next) {
        const Internship *item = &node->data;
        if (fprintf(file, "%d|%s|%s|%s|%s|%d|%.2f|%.2f|%s\n",
                    item->id, item->company, item->title, item->domain, item->skills,
                    item->duration_months, item->stipend, item->min_cgpa, item->deadline) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

int internship_seed_defaults(InternshipList *list)
{
    static const Internship samples[] = {
        {1001, "Northstar Analytics", "Data Analyst Intern", "Data Science", "SQL, Excel, Python", 3, 12000, 7.0, "2026-11-15"},
        {1002, "Bluejay Robotics", "Computer Vision Intern", "AI and Robotics", "C, OpenCV, Linear Algebra", 4, 18000, 7.5, "2026-11-22"},
        {1003, "Maple Metrics", "Business Intelligence Intern", "Analytics", "SQL, Power BI, Excel", 3, 10000, 6.5, "2026-12-01"},
        {1004, "Copper Cloud Labs", "Cloud Systems Intern", "Cloud Computing", "C, Linux, Networking", 6, 22000, 7.0, "2026-12-10"},
        {1005, "Orbit Data Works", "Machine Learning Intern", "Machine Learning", "Python, Statistics, SQL", 4, 20000, 8.0, "2026-11-30"},
        {1006, "Pixel Harbor", "Frontend Design Intern", "Web Technology", "HTML, CSS, JavaScript", 3, 9000, 6.0, "2026-12-05"},
        {1007, "Cedar Cyber Labs", "Security Research Intern", "Cybersecurity", "C, Linux, Networking", 5, 16000, 7.2, "2026-12-14"},
        {1008, "Silverline Mobility", "Data Engineering Intern", "Data Engineering", "C, SQL, Linux", 4, 15000, 6.8, "2026-11-28"},
        {1009, "Amber BioCompute", "AI Research Intern", "AI and Healthcare", "Python, Statistics, Linear Algebra", 6, 25000, 8.2, "2026-12-18"}
    };
    size_t i;
    for (i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        if (!internship_list_add(list, &samples[i])) {
            return 0;
        }
    }
    return 1;
}

static int has_skill(const char *student_skills, const char *required_skill)
{
    char copy[SKILLS_LEN];
    char required[SKILLS_LEN];
    char *cursor;
    char *token;
    size_t required_length = strlen(required_skill);

    if (required_length == 0 || required_length >= sizeof(copy)) {
        return 0;
    }
    (void)snprintf(copy, sizeof(copy), "%s", student_skills);
    (void)snprintf(required, sizeof(required), "%s", required_skill);
    required_length = strlen(required);
    while (required_length > 0 &&
           (required[required_length - 1] == ' ' || required[required_length - 1] == '\t')) {
        required[--required_length] = '\0';
    }
    cursor = copy;
    while (*cursor != '\0') {
        char *comma;
        token = cursor;
        comma = strchr(cursor, ',');
        if (comma != NULL) {
            *comma = '\0';
            cursor = comma + 1;
        } else {
            cursor += strlen(cursor);
        }
        while (*token == ' ' || *token == '\t') {
            token++;
        }
        {
            size_t token_length = strlen(token);
            while (token_length > 0 &&
                   (token[token_length - 1] == ' ' || token[token_length - 1] == '\t')) {
                token[--token_length] = '\0';
            }
        }
        if (_stricmp(token, required) == 0) {
            return 1;
        }
    }
    return 0;
}

int student_is_eligible(const Student *student, const Internship *internship)
{
    char requirements[SKILLS_LEN];
    char *skill;

    if (student == NULL || internship == NULL || student->cgpa < internship->min_cgpa) {
        return 0;
    }
    (void)snprintf(requirements, sizeof(requirements), "%s", internship->skills);
    {
        char *cursor = requirements;
        while (*cursor != '\0') {
            char *comma = strchr(cursor, ',');
            skill = cursor;
            if (comma != NULL) {
                *comma = '\0';
                cursor = comma + 1;
            } else {
                cursor += strlen(cursor);
            }
            while (*skill == ' ' || *skill == '\t') {
                skill++;
            }
            if (!has_skill(student->skills, skill)) {
                return 0;
            }
        }
    }
    return 1;
}
