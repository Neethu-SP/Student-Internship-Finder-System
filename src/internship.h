#ifndef INTERNSHIP_H
#define INTERNSHIP_H

#define COMPANY_LEN 64
#define TITLE_LEN 80
#define DOMAIN_LEN 48
#define SKILLS_LEN 128
#define NAME_LEN 64
#define DATE_LEN 11
#define MAX_INTERNSHIPS 256

typedef struct {
    int id;
    char company[COMPANY_LEN];
    char title[TITLE_LEN];
    char domain[DOMAIN_LEN];
    char skills[SKILLS_LEN];
    int duration_months;
    double stipend;
    double min_cgpa;
    char deadline[DATE_LEN];
} Internship;

typedef struct {
    int id;
    char name[NAME_LEN];
    int year;
    double cgpa;
    char skills[SKILLS_LEN];
} Student;

typedef struct InternshipNode {
    Internship data;
    struct InternshipNode *next;
} InternshipNode;

typedef struct {
    InternshipNode *head;
    int count;
} InternshipList;

void internship_list_init(InternshipList *list);
void internship_list_clear(InternshipList *list);
int internship_list_add(InternshipList *list, const Internship *internship);
int internship_list_delete(InternshipList *list, int id);
Internship *internship_list_find(InternshipList *list, int id);
int internship_list_collect(InternshipList *list, Internship **items, int capacity);
int internship_load_file(InternshipList *list, const char *path);
int internship_save_file(const InternshipList *list, const char *path);
int internship_seed_defaults(InternshipList *list);
int student_is_eligible(const Student *student, const Internship *internship);

#endif
