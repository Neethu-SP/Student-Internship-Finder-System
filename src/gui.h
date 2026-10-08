#ifndef GUI_H
#define GUI_H

#include <windows.h>

#include "application_queue.h"
#include "hash_table.h"
#include "stipend_bst.h"

int run_student_finder_gui(HINSTANCE instance, InternshipList *internships,
                           InternshipHashTable *hash, StipendNode **stipend_root,
                           ApplicationQueue *applications);

#endif
