#include "gui.h"
#include "internship.h"

#include <commctrl.h>
#include <stdio.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show_command)
{
    InternshipList internships;
    InternshipHashTable hash;
    StipendNode *stipend_root = NULL;
    ApplicationQueue applications;
    Internship *items[MAX_INTERNSHIPS];
    int count;
    int i;
    int loaded;
    int result;

    (void)previous;
    (void)command_line;
    (void)show_command;
    SetProcessDPIAware();
    InitCommonControls();
    internship_list_init(&internships);
    hash_table_init(&hash);
    application_queue_init(&applications);

    loaded = internship_load_file(&internships, "data\\internships.txt");
    if (loaded <= 0) {
        internship_list_clear(&internships);
        if (!internship_seed_defaults(&internships)) {
            MessageBoxA(NULL, "Could not load or initialize internship records.",
                        "Student Internship Finder", MB_ICONERROR | MB_OK);
            internship_list_clear(&internships);
            return 1;
        }
        if (!internship_save_file(&internships, "data\\internships.txt")) {
            MessageBoxA(NULL, "Sample internships are loaded in memory, but data\\internships.txt "
                              "could not be written. Run the program from its project folder.",
                        "Student Internship Finder", MB_ICONWARNING | MB_OK);
        }
    }

    count = internship_list_collect(&internships, items, MAX_INTERNSHIPS);
    for (i = 0; i < count; i++) {
        if (!hash_table_insert(&hash, items[i]) ||
            !stipend_bst_insert(&stipend_root, items[i])) {
            MessageBoxA(NULL, "Not enough memory to build the internship indexes.",
                        "Student Internship Finder", MB_ICONERROR | MB_OK);
            stipend_bst_clear(stipend_root);
            hash_table_clear(&hash);
            internship_list_clear(&internships);
            return 1;
        }
    }

    loaded = application_queue_load_file(&applications, "data\\applications.txt");
    if (loaded < 0 && !application_queue_save_file(&applications, "data\\applications.txt")) {
        MessageBoxA(NULL, "Applications will work for this session, but the application file "
                          "could not be created.",
                    "Student Internship Finder", MB_ICONWARNING | MB_OK);
    }

    result = run_student_finder_gui(instance, &internships, &hash, &stipend_root, &applications);
    stipend_bst_clear(stipend_root);
    hash_table_clear(&hash);
    application_queue_clear(&applications);
    internship_list_clear(&internships);
    return result;
}
