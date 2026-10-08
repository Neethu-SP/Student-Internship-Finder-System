#include "application_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void application_queue_init(ApplicationQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

void application_queue_clear(ApplicationQueue *queue)
{
    ApplicationNode *node = queue->front;
    while (node != NULL) {
        ApplicationNode *next = node->next;
        free(node);
        node = next;
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

int application_queue_enqueue(ApplicationQueue *queue, const Application *application)
{
    ApplicationNode *node = (ApplicationNode *)malloc(sizeof(*node));
    if (node == NULL) return 0;
    node->data = *application;
    node->next = NULL;
    /* Link at the rear; dequeue always removes from the front (FIFO). */
    if (queue->rear == NULL) {
        queue->front = node;
    } else {
        queue->rear->next = node;
    }
    queue->rear = node;
    queue->count++;
    return 1;
}

int application_queue_dequeue(ApplicationQueue *queue, Application *application)
{
    ApplicationNode *node;
    if (queue->front == NULL) return 0;
    node = queue->front;
    if (application != NULL) *application = node->data;
    queue->front = node->next;
    if (queue->front == NULL) queue->rear = NULL;
    free(node);
    queue->count--;
    return 1;
}

int application_queue_has_student_application(const ApplicationQueue *queue,
                                              int student_id, int internship_id)
{
    const ApplicationNode *node;
    for (node = queue->front; node != NULL; node = node->next) {
        if (node->data.student.id == student_id && node->data.internship_id == internship_id) {
            return 1;
        }
    }
    return 0;
}

int application_queue_load_file(ApplicationQueue *queue, const char *path)
{
    FILE *file = fopen(path, "r");
    char line[640];
    int loaded = 0;
    if (file == NULL) return -1;
    application_queue_clear(queue);
    while (fgets(line, sizeof(line), file) != NULL) {
        Application application;
        int fields = sscanf(line, "%d|%63[^|]|%d|%lf|%127[^|]|%d|%23[^|]|%10[^\r\n]",
                            &application.student.id, application.student.name,
                            &application.student.year, &application.student.cgpa,
                            application.student.skills, &application.internship_id,
                            application.status, application.applied_on);
        if (fields == 8 && application_queue_enqueue(queue, &application)) loaded++;
    }
    fclose(file);
    return loaded;
}

int application_queue_save_file(const ApplicationQueue *queue, const char *path)
{
    FILE *file = fopen(path, "w");
    const ApplicationNode *node;
    if (file == NULL) return 0;
    for (node = queue->front; node != NULL; node = node->next) {
        const Application *app = &node->data;
        if (fprintf(file, "%d|%s|%d|%.2f|%s|%d|%s|%s\n",
                    app->student.id, app->student.name, app->student.year, app->student.cgpa,
                    app->student.skills, app->internship_id, app->status, app->applied_on) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}
