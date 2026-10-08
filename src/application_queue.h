#ifndef APPLICATION_QUEUE_H
#define APPLICATION_QUEUE_H

#include "internship.h"

#define STATUS_LEN 24

typedef struct {
    Student student;
    int internship_id;
    char status[STATUS_LEN];
    char applied_on[DATE_LEN];
} Application;

typedef struct ApplicationNode {
    Application data;
    struct ApplicationNode *next;
} ApplicationNode;

typedef struct {
    ApplicationNode *front;
    ApplicationNode *rear;
    int count;
} ApplicationQueue;

void application_queue_init(ApplicationQueue *queue);
void application_queue_clear(ApplicationQueue *queue);
int application_queue_enqueue(ApplicationQueue *queue, const Application *application);
int application_queue_dequeue(ApplicationQueue *queue, Application *application);
int application_queue_has_student_application(const ApplicationQueue *queue,
                                              int student_id, int internship_id);
int application_queue_load_file(ApplicationQueue *queue, const char *path);
int application_queue_save_file(const ApplicationQueue *queue, const char *path);

#endif
