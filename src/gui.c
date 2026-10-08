#include "gui.h"
#include "search_sort.h"

#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_CLASS "InternHubMainWindow"
#define DIALOG_CLASS "InternHubAddDialog"
#define DATA_FILE "data\\internships.txt"
#define APPLICATION_FILE "data\\applications.txt"
#define SIDEBAR_WIDTH 220
#define MAX_TRACKED_CONTROLS 128

enum {
    VIEW_DASHBOARD,
    VIEW_FIND,
    VIEW_APPLICATIONS,
    VIEW_ELIGIBILITY,
    VIEW_MANAGE,
    PANEL_DETAILS,
    PANEL_TOTAL
};

enum {
    ID_NAV_DASHBOARD = 100,
    ID_NAV_FIND,
    ID_NAV_APPLICATIONS,
    ID_NAV_ELIGIBILITY,
    ID_NAV_MANAGE,
    ID_NAV_EXIT,
    ID_FIND_SEARCH,
    ID_FIND_DOMAIN,
    ID_FIND_GO,
    ID_FIND_BINARY,
    ID_FIND_RESET,
    ID_FIND_STIPEND_SORT,
    ID_FIND_DEADLINE_SORT,
    ID_FIND_MIN_STIPEND,
    ID_FIND_STIPEND_GO,
    ID_FIND_DETAILS,
    ID_FIND_APPLY,
    ID_MANAGE_ADD,
    ID_MANAGE_DELETE,
    ID_ELIG_STUDENT_ID,
    ID_ELIG_NAME,
    ID_ELIG_YEAR,
    ID_ELIG_CGPA,
    ID_ELIG_SKILLS,
    ID_ELIG_INTERNSHIP_ID,
    ID_ELIG_CHECK,
    ID_ELIG_APPLY,
    ID_ELIG_RESULT,
    ID_DETAILS_BACK,
    ID_DETAILS_APPLY,
    ID_DASH_FIND,
    ID_DASH_APPLICATIONS,
    ID_DASH_ELIGIBILITY,
    ID_DASH_MANAGE,
    ID_FIND_LIST,
    ID_APPLICATION_LIST,
    ID_MANAGE_LIST,
    ID_METRIC_INTERNSHIPS,
    ID_METRIC_APPLICATIONS,
    ID_METRIC_DOMAINS,
    ID_DIALOG_OK = 500,
    ID_DIALOG_CANCEL,
    ID_DIALOG_FIRST_FIELD = 520
};

typedef struct {
    HWND window;
    HWND find_list;
    HWND application_list;
    HWND manage_list;
    HWND search;
    HWND domain;
    HWND domain_label;
    HWND stipend;
    HWND stipend_label;
    HWND student_id;
    HWND student_name;
    HWND student_year;
    HWND student_cgpa;
    HWND student_skills;
    HWND internship_id;
    HWND eligibility_result;
    HWND brand_label;
    HWND brand_caption;
    HWND dashboard_info;
    HWND metric_values[3];
    HWND metric_labels[3];
    HWND detail_values[8];
    HWND detail_labels[8];
    HWND eligibility_labels[6];
    HWND page_titles[PANEL_TOTAL];
    HWND page_subtitles[PANEL_TOTAL];
    HWND nav_buttons[5];
    HFONT font;
    HFONT heading_font;
    HBRUSH background_brush;
    HBRUSH white_brush;
    HBRUSH sidebar_brush;
    InternshipList *internships;
    InternshipHashTable *hash;
    StipendNode **stipend_root;
    ApplicationQueue *applications;
    Internship *find_items[MAX_INTERNSHIPS];
    Internship *manage_items[MAX_INTERNSHIPS];
    int find_count;
    int manage_count;
    int selected_internship_id;
    int active_view;
    int result_state;
    int control_count;
    struct {
        HWND handle;
        int view;
    } controls[MAX_TRACKED_CONTROLS];
    char domains[MAX_INTERNSHIPS][DOMAIN_LEN];
    int domain_count;
} AppContext;

typedef struct {
    Internship result;
    int accepted;
} AddDialogState;

static AppContext app;

static int run_add_dialog(Internship *result);

static const COLORREF COLOR_NAVY = RGB(22, 37, 62);
static const COLORREF COLOR_NAVY_LIGHT = RGB(34, 53, 82);
static const COLORREF COLOR_BLUE = RGB(47, 111, 237);
static const COLORREF COLOR_BLUE_DARK = RGB(34, 88, 194);
static const COLORREF UI_BACKGROUND = RGB(244, 247, 251);
static const COLORREF COLOR_WHITE = RGB(255, 255, 255);
static const COLORREF COLOR_TEXT = RGB(38, 52, 72);
static const COLORREF COLOR_MUTED = RGB(112, 126, 145);
static const COLORREF COLOR_GREEN = RGB(29, 137, 85);
static const COLORREF COLOR_ORANGE = RGB(199, 119, 30);
static const COLORREF COLOR_RED = RGB(188, 58, 64);

static HWND make_control(int view, const char *class_name, const char *text, DWORD style,
                         int x, int y, int width, int height, int id)
{
    HWND control = CreateWindowExA(0, class_name, text, WS_CHILD | style,
                                   x, y, width, height, app.window,
                                   (HMENU)(INT_PTR)id, GetModuleHandleA(NULL), NULL);
    if (control != NULL) {
        SendMessageA(control, WM_SETFONT, (WPARAM)app.font, TRUE);
        if (view >= 0 && app.control_count < MAX_TRACKED_CONTROLS) {
            app.controls[app.control_count].handle = control;
            app.controls[app.control_count].view = view;
            app.control_count++;
        }
    }
    return control;
}

static HWND make_button(int view, const char *text, int x, int y, int width, int height, int id)
{
    return make_control(view, "BUTTON", text, BS_OWNERDRAW | WS_TABSTOP,
                        x, y, width, height, id);
}

static HWND make_label(int view, const char *text, int x, int y, int width, int height)
{
    return make_control(view, "STATIC", text, SS_LEFT, x, y, width, height, 0);
}

static void set_text(HWND control, const char *text)
{
    if (control != NULL) SetWindowTextA(control, text);
}

static void read_text(HWND control, char *buffer, size_t capacity)
{
    if (capacity == 0) return;
    GetWindowTextA(control, buffer, (int)capacity);
    buffer[capacity - 1] = '\0';
}

static int parse_int(const char *text, int *value)
{
    char *end;
    long parsed;
    if (text[0] == '\0') return 0;
    parsed = strtol(text, &end, 10);
    if (*end != '\0' || parsed < 1 || parsed > 2147483647L) return 0;
    *value = (int)parsed;
    return 1;
}

static int parse_double(const char *text, double *value)
{
    char *end;
    double parsed;
    if (text[0] == '\0') return 0;
    parsed = strtod(text, &end);
    if (*end != '\0' || !isfinite(parsed)) return 0;
    *value = parsed;
    return 1;
}

static int valid_deadline(const char *date)
{
    int year, month, day, i;
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') return 0;
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return 0;
    }
    year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
           (date[2] - '0') * 10 + date[3] - '0';
    month = (date[5] - '0') * 10 + date[6] - '0';
    day = (date[8] - '0') * 10 + date[9] - '0';
    return year >= 2020 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

static int text_has_pipe(const char *text)
{
    return strchr(text, '|') != NULL || strchr(text, '\n') != NULL || strchr(text, '\r') != NULL;
}

static int collect_all(Internship **items)
{
    return internship_list_collect(app.internships, items, MAX_INTERNSHIPS);
}

static int internship_id_from_list(HWND list)
{
    LVITEMA item;
    int row = ListView_GetNextItem(list, -1, LVNI_SELECTED);
    if (row < 0) return 0;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = row;
    return ListView_GetItem(list, &item) ? (int)item.lParam : 0;
}

static void show_view(int view)
{
    int i;
    if (view < 0 || view >= PANEL_TOTAL) return;
    app.active_view = view;
    for (i = 0; i < app.control_count; i++) {
        ShowWindow(app.controls[i].handle,
                   app.controls[i].view == view ? SW_SHOW : SW_HIDE);
    }
    for (i = 0; i < 5; i++) {
        InvalidateRect(app.nav_buttons[i], NULL, TRUE);
    }
    InvalidateRect(app.window, NULL, TRUE);
    SetFocus(view == VIEW_FIND ? app.search :
             view == VIEW_ELIGIBILITY ? app.student_id :
             view == VIEW_APPLICATIONS ? app.application_list :
             view == VIEW_MANAGE ? app.manage_list : app.window);
}

static void update_dashboard_metrics(void)
{
    Internship *items[MAX_INTERNSHIPS];
    int count = collect_all(items);
    int domains = 0;
    int i, j;
    char text[48];
    for (i = 0; i < count; i++) {
        for (j = 0; j < i; j++) {
            if (_stricmp(items[i]->domain, items[j]->domain) == 0) break;
        }
        if (j == i) domains++;
    }
    (void)snprintf(text, sizeof(text), "%d", count);
    set_text(app.metric_values[0], text);
    (void)snprintf(text, sizeof(text), "%d", app.applications->count);
    set_text(app.metric_values[1], text);
    (void)snprintf(text, sizeof(text), "%d", domains);
    set_text(app.metric_values[2], text);
}

static void insert_internship_row(HWND list, int row, const Internship *internship)
{
    char id[24], duration[24], stipend[32], cgpa[24];
    LVITEMA entry;
    (void)snprintf(id, sizeof(id), "%d", internship->id);
    (void)snprintf(duration, sizeof(duration), "%d months", internship->duration_months);
    (void)snprintf(stipend, sizeof(stipend), "%.0f", internship->stipend);
    (void)snprintf(cgpa, sizeof(cgpa), "%.1f", internship->min_cgpa);
    ZeroMemory(&entry, sizeof(entry));
    entry.mask = LVIF_TEXT | LVIF_PARAM;
    entry.iItem = row;
    entry.pszText = id;
    entry.lParam = (LPARAM)internship->id;
    ListView_InsertItem(list, &entry);
    ListView_SetItemText(list, row, 1, (LPSTR)internship->company);
    ListView_SetItemText(list, row, 2, (LPSTR)internship->title);
    ListView_SetItemText(list, row, 3, (LPSTR)internship->domain);
    ListView_SetItemText(list, row, 4, (LPSTR)internship->skills);
    ListView_SetItemText(list, row, 5, duration);
    ListView_SetItemText(list, row, 6, stipend);
    ListView_SetItemText(list, row, 7, cgpa);
    ListView_SetItemText(list, row, 8, (LPSTR)internship->deadline);
}

static void show_internships(HWND list, Internship **items, int count)
{
    int i;
    if (list == NULL) return;
    ListView_DeleteAllItems(list);
    for (i = 0; i < count && i < MAX_INTERNSHIPS; i++) {
        insert_internship_row(list, i, items[i]);
    }
}

static void refresh_manage_list(void)
{
    app.manage_count = collect_all(app.manage_items);
    show_internships(app.manage_list, app.manage_items, app.manage_count);
    update_dashboard_metrics();
}

static void refresh_applications(void)
{
    const ApplicationNode *node;
    int row = 0;
    ListView_DeleteAllItems(app.application_list);
    for (node = app.applications->front; node != NULL; node = node->next) {
        Internship *internship = hash_table_find(app.hash, node->data.internship_id);
        LVITEMA item;
        char student[NAME_LEN + 24], id[24];
        (void)snprintf(student, sizeof(student), "%s (#%d)",
                       node->data.student.name, node->data.student.id);
        (void)snprintf(id, sizeof(id), "%d", node->data.internship_id);
        ZeroMemory(&item, sizeof(item));
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = row;
        item.pszText = internship != NULL ? internship->title : "Internship removed";
        item.lParam = (LPARAM)node->data.internship_id;
        ListView_InsertItem(app.application_list, &item);
        ListView_SetItemText(app.application_list, row, 1,
                             internship != NULL ? (LPSTR)internship->company : "");
        ListView_SetItemText(app.application_list, row, 2, student);
        ListView_SetItemText(app.application_list, row, 3, (LPSTR)node->data.status);
        ListView_SetItemText(app.application_list, row, 4, id);
        ListView_SetItemText(app.application_list, row, 5, (LPSTR)node->data.applied_on);
        row++;
    }
    update_dashboard_metrics();
}

static void collect_domains(void)
{
    Internship *items[MAX_INTERNSHIPS];
    int count = collect_all(items);
    int i, j;
    app.domain_count = 0;
    for (i = 0; i < count; i++) {
        for (j = 0; j < app.domain_count; j++) {
            if (_stricmp(items[i]->domain, app.domains[j]) == 0) break;
        }
        if (j == app.domain_count && app.domain_count < MAX_INTERNSHIPS) {
            (void)snprintf(app.domains[app.domain_count], DOMAIN_LEN, "%s", items[i]->domain);
            app.domain_count++;
        }
    }
    if (app.domain != NULL) {
        int selection = (int)SendMessageA(app.domain, CB_GETCURSEL, 0, 0);
        SendMessageA(app.domain, CB_RESETCONTENT, 0, 0);
        SendMessageA(app.domain, CB_ADDSTRING, 0, (LPARAM)"All domains");
        for (i = 0; i < app.domain_count; i++) {
            SendMessageA(app.domain, CB_ADDSTRING, 0, (LPARAM)app.domains[i]);
        }
        if (selection < 0 || selection > app.domain_count) selection = 0;
        SendMessageA(app.domain, CB_SETCURSEL, selection, 0);
    }
}

static void run_search(void)
{
    Internship *all[MAX_INTERNSHIPS];
    Internship *domain_items[MAX_INTERNSHIPS];
    Internship *matches[MAX_INTERNSHIPS];
    char query[SKILLS_LEN];
    char *end;
    long id;
    int count = collect_all(all);
    int domain_index = (int)SendMessageA(app.domain, CB_GETCURSEL, 0, 0);
    int domain_count = 0;
    int i;

    read_text(app.search, query, sizeof(query));
    if (domain_index > 0 && domain_index <= app.domain_count) {
        for (i = 0; i < count; i++) {
            if (_stricmp(all[i]->domain, app.domains[domain_index - 1]) == 0) {
                domain_items[domain_count++] = all[i];
            }
        }
    } else {
        memcpy(domain_items, all, (size_t)count * sizeof(all[0]));
        domain_count = count;
    }

    if (query[0] == '\0') {
        app.find_count = domain_count;
        memcpy(app.find_items, domain_items, (size_t)domain_count * sizeof(domain_items[0]));
    } else {
        id = strtol(query, &end, 10);
        if (end != query && *end == '\0' && id > 0 && id <= 2147483647L) {
            Internship *match = hash_table_find(app.hash, (int)id);
            app.find_count = match != NULL &&
                (domain_index == 0 || _stricmp(match->domain, app.domains[domain_index - 1]) == 0)
                ? 1 : 0;
            if (app.find_count != 0) app.find_items[0] = match;
        } else {
            app.find_count = search_linear_text(domain_items, domain_count, query,
                                                matches, MAX_INTERNSHIPS);
            memcpy(app.find_items, matches, (size_t)app.find_count * sizeof(matches[0]));
        }
    }
    show_internships(app.find_list, app.find_items, app.find_count);
}

static void run_binary_search(void)
{
    Internship *items[MAX_INTERNSHIPS];
    Internship *match;
    char query[32];
    int id, count;
    read_text(app.search, query, sizeof(query));
    if (!parse_int(query, &id)) {
        MessageBoxA(app.window, "Type a positive Internship ID in the search box first.",
                    "Binary search", MB_ICONINFORMATION | MB_OK);
        return;
    }
    count = collect_all(items);
    sort_by_id(items, count);
    match = search_binary_id(items, count, id);
    app.find_count = match == NULL ? 0 : 1;
    if (match != NULL) app.find_items[0] = match;
    show_internships(app.find_list, app.find_items, app.find_count);
}

static int read_student(Student *student)
{
    char id[32], year[32], cgpa[32];
    read_text(app.student_id, id, sizeof(id));
    read_text(app.student_name, student->name, sizeof(student->name));
    read_text(app.student_year, year, sizeof(year));
    read_text(app.student_cgpa, cgpa, sizeof(cgpa));
    read_text(app.student_skills, student->skills, sizeof(student->skills));
    if (!parse_int(id, &student->id) || student->name[0] == '\0' ||
        text_has_pipe(student->name) || !parse_int(year, &student->year) ||
        student->year < 1 || student->year > 4 ||
        !parse_double(cgpa, &student->cgpa) || student->cgpa < 0.0 ||
        student->cgpa > 10.0 || student->skills[0] == '\0' ||
        text_has_pipe(student->skills)) {
        MessageBoxA(app.window,
                    "Enter a positive student ID, name, year (1-4), CGPA (0-10), "
                    "and comma-separated skills.",
                    "Check student details", MB_ICONINFORMATION | MB_OK);
        return 0;
    }
    return 1;
}

static void set_eligibility_result(int eligible, const Internship *internship)
{
    char text[512];
    app.result_state = eligible ? 1 : -1;
    if (eligible) {
        (void)snprintf(text, sizeof(text), "ELIGIBLE  |  You meet the requirements for %s.",
                       internship->title);
    } else {
        (void)snprintf(text, sizeof(text),
                       "NOT ELIGIBLE  |  Required CGPA: %.1f  |  Required skills: %s",
                       internship->min_cgpa, internship->skills);
    }
    set_text(app.eligibility_result, text);
    ShowWindow(app.eligibility_result, SW_SHOW);
    InvalidateRect(app.eligibility_result, NULL, TRUE);
}

static void submit_application(const Student *student, Internship *internship)
{
    Application application;
    SYSTEMTIME today;
    if (application_queue_has_student_application(app.applications, student->id, internship->id)) {
        MessageBoxA(app.window, "This student already applied for this internship.",
                    "Duplicate application", MB_ICONINFORMATION | MB_OK);
        return;
    }
    ZeroMemory(&application, sizeof(application));
    application.student = *student;
    application.internship_id = internship->id;
    (void)snprintf(application.status, sizeof(application.status), "%s", "Submitted");
    GetLocalTime(&today);
    application.applied_on[0] = (char)('0' + (today.wYear / 1000) % 10);
    application.applied_on[1] = (char)('0' + (today.wYear / 100) % 10);
    application.applied_on[2] = (char)('0' + (today.wYear / 10) % 10);
    application.applied_on[3] = (char)('0' + today.wYear % 10);
    application.applied_on[4] = '-';
    application.applied_on[5] = (char)('0' + today.wMonth / 10);
    application.applied_on[6] = (char)('0' + today.wMonth % 10);
    application.applied_on[7] = '-';
    application.applied_on[8] = (char)('0' + today.wDay / 10);
    application.applied_on[9] = (char)('0' + today.wDay % 10);
    application.applied_on[10] = '\0';
    if (!application_queue_enqueue(app.applications, &application)) {
        MessageBoxA(app.window, "Not enough memory to record the application.",
                    "Application failed", MB_ICONERROR | MB_OK);
        return;
    }
    refresh_applications();
    if (!application_queue_save_file(app.applications, APPLICATION_FILE)) {
        MessageBoxA(app.window, "The application was added for this session but could not be "
                                "saved to data\\applications.txt.",
                    "Could not save application", MB_ICONWARNING | MB_OK);
    } else {
        MessageBoxA(app.window, "Application submitted and added to the queue.",
                    "Application submitted", MB_ICONINFORMATION | MB_OK);
    }
}

static int check_eligibility(int apply)
{
    char internship_id[32];
    int id;
    Student student;
    Internship *internship;
    read_text(app.internship_id, internship_id, sizeof(internship_id));
    if (!parse_int(internship_id, &id)) {
        MessageBoxA(app.window, "Enter a positive Internship ID first.",
                    "Eligibility checker", MB_ICONINFORMATION | MB_OK);
        return 0;
    }
    internship = hash_table_find(app.hash, id);
    if (internship == NULL) {
        MessageBoxA(app.window, "No available internship has that ID.",
                    "Eligibility checker", MB_ICONINFORMATION | MB_OK);
        return 0;
    }
    if (!read_student(&student)) return 0;
    if (!student_is_eligible(&student, internship)) {
        set_eligibility_result(0, internship);
        return 0;
    }
    set_eligibility_result(1, internship);
    if (apply) submit_application(&student, internship);
    return 1;
}

static int selected_find_id(void)
{
    int id = internship_id_from_list(app.find_list);
    return id != 0 ? id : app.selected_internship_id;
}

static void show_details(int id)
{
    Internship *item = hash_table_find(app.hash, id);
    char values[8][192];
    int i;
    if (item == NULL) {
        MessageBoxA(app.window, "Select an available internship first.",
                    "Internship details", MB_ICONINFORMATION | MB_OK);
        return;
    }
    app.selected_internship_id = id;
    (void)snprintf(values[0], sizeof(values[0]), "%s", item->company);
    (void)snprintf(values[1], sizeof(values[1]), "%s", item->title);
    (void)snprintf(values[2], sizeof(values[2]), "%s", item->domain);
    (void)snprintf(values[3], sizeof(values[3]), "%s", item->skills);
    (void)snprintf(values[4], sizeof(values[4]), "%d months", item->duration_months);
    (void)snprintf(values[5], sizeof(values[5]), "%.0f per month", item->stipend);
    (void)snprintf(values[6], sizeof(values[6]), "%.1f", item->min_cgpa);
    (void)snprintf(values[7], sizeof(values[7]), "%s", item->deadline);
    for (i = 0; i < 8; i++) set_text(app.detail_values[i], values[i]);
    show_view(PANEL_DETAILS);
}

static void handle_add_internship(void)
{
    Internship item;
    Internship *stored;
    if (!run_add_dialog(&item)) return;
    if (internship_list_find(app.internships, item.id) != NULL) {
        MessageBoxA(app.window, "That Internship ID is already in use.",
                    "Duplicate ID", MB_ICONWARNING | MB_OK);
        return;
    }
    if (!internship_list_add(app.internships, &item)) {
        MessageBoxA(app.window, "Could not add the internship. The linked list may be full "
                                "or memory may be unavailable.",
                    "Add failed", MB_ICONERROR | MB_OK);
        return;
    }
    stored = internship_list_find(app.internships, item.id);
    if (!hash_table_insert(app.hash, stored)) {
        (void)internship_list_delete(app.internships, item.id);
        MessageBoxA(app.window, "Could not update the ID hash table; no record was added.",
                    "Add failed", MB_ICONERROR | MB_OK);
        return;
    }
    if (!stipend_bst_insert(app.stipend_root, stored)) {
        (void)hash_table_remove(app.hash, item.id);
        (void)internship_list_delete(app.internships, item.id);
        MessageBoxA(app.window, "Could not update the stipend tree; no record was added.",
                    "Add failed", MB_ICONERROR | MB_OK);
        return;
    }
    if (!internship_save_file(app.internships, DATA_FILE)) {
        MessageBoxA(app.window, "The internship is available for this session but could not "
                                "be saved to data\\internships.txt.",
                    "Could not save record", MB_ICONWARNING | MB_OK);
    }
    collect_domains();
    refresh_manage_list();
    run_search();
    InvalidateRect(app.window, NULL, TRUE);
}

static void handle_delete_internship(void)
{
    int id = internship_id_from_list(app.manage_list);
    Internship *item;
    char prompt[192];
    if (id == 0) {
        MessageBoxA(app.window, "Select an internship from the table first.",
                    "Manage internships", MB_ICONINFORMATION | MB_OK);
        return;
    }
    item = hash_table_find(app.hash, id);
    if (item == NULL) return;
    (void)snprintf(prompt, sizeof(prompt), "Delete %s (ID %d)?", item->title, id);
    if (MessageBoxA(app.window, prompt, "Confirm delete",
                    MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    (void)stipend_bst_remove(app.stipend_root, id, item->stipend);
    (void)hash_table_remove(app.hash, id);
    (void)internship_list_delete(app.internships, id);
    if (!internship_save_file(app.internships, DATA_FILE)) {
        MessageBoxA(app.window, "The record was deleted for this session but the file could "
                                "not be updated.",
                    "Could not save records", MB_ICONWARNING | MB_OK);
    }
    refresh_manage_list();
    collect_domains();
    run_search();
}

static void create_list_view(HWND *result, int view, int id, int x, int y, int width, int height,
                             const char **headers, const int *widths, int columns)
{
    LVCOLUMNA column;
    int i;
    *result = make_control(view, WC_LISTVIEWA, "",
                           LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                           x, y, width, height, id);
    if (*result == NULL) return;
    ListView_SetExtendedListViewStyle(*result,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    ListView_SetBkColor(*result, COLOR_WHITE);
    ListView_SetTextBkColor(*result, COLOR_WHITE);
    ListView_SetTextColor(*result, COLOR_TEXT);
    for (i = 0; i < columns; i++) {
        ZeroMemory(&column, sizeof(column));
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = (LPSTR)headers[i];
        column.cx = widths[i];
        column.iSubItem = i;
        ListView_InsertColumn(*result, i, &column);
    }
}

static void create_navigation(void)
{
    int ids[5] = {ID_NAV_DASHBOARD, ID_NAV_FIND, ID_NAV_APPLICATIONS,
                  ID_NAV_ELIGIBILITY, ID_NAV_MANAGE};
    const char *labels[5] = {"Dashboard", "Find Internships", "My Applications",
                             "Eligibility Checker", "Manage Internships"};
    int i;
    app.brand_label = CreateWindowExA(0, "STATIC", "InternHub",
                                      WS_CHILD | WS_VISIBLE | SS_LEFT,
                                      28, 26, 170, 38, app.window, NULL,
                                      GetModuleHandleA(NULL), NULL);
    app.brand_caption = CreateWindowExA(0, "STATIC", "STUDENT INTERNSHIP FINDER",
                                        WS_CHILD | WS_VISIBLE | SS_LEFT,
                                        29, 64, 170, 32, app.window, NULL,
                                        GetModuleHandleA(NULL), NULL);
    SendMessageA(app.brand_label, WM_SETFONT, (WPARAM)app.heading_font, TRUE);
    SendMessageA(app.brand_caption, WM_SETFONT, (WPARAM)app.font, TRUE);
    for (i = 0; i < 5; i++) {
        app.nav_buttons[i] = CreateWindowExA(0, "BUTTON", labels[i],
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
            16, 130 + i * 56, SIDEBAR_WIDTH - 32, 44,
            app.window, (HMENU)(INT_PTR)ids[i], GetModuleHandleA(NULL), NULL);
        if (app.nav_buttons[i] != NULL) {
            SendMessageA(app.nav_buttons[i], WM_SETFONT, (WPARAM)app.font, TRUE);
        }
    }
    (void)CreateWindowExA(0, "BUTTON", "Exit", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                          16, 0, SIDEBAR_WIDTH - 32, 44, app.window,
                          (HMENU)(INT_PTR)ID_NAV_EXIT, GetModuleHandleA(NULL), NULL);
}

static void create_main_controls(void)
{
    static const char *internship_headers[] = {
        "ID", "Company", "Internship title", "Domain", "Required skills",
        "Duration", "Stipend", "Min CGPA", "Deadline"
    };
    static const int internship_widths[] = {66, 145, 180, 125, 190, 82, 90, 82, 105};
    static const char *application_headers[] = {
        "Internship", "Company", "Student", "Status", "Internship ID", "Applied on"
    };
    static const int application_widths[] = {205, 175, 195, 112, 110, 115};
    int i;

    create_navigation();
    for (i = 0; i < PANEL_TOTAL; i++) {
        app.page_titles[i] = make_label(i, "", SIDEBAR_WIDTH + 30, 24, 700, 38);
        app.page_subtitles[i] = make_label(i, "", SIDEBAR_WIDTH + 32, 62, 800, 24);
        SendMessageA(app.page_titles[i], WM_SETFONT, (WPARAM)app.heading_font, TRUE);
    }
    set_text(app.page_titles[VIEW_DASHBOARD], "Good day, welcome to InternHub");
    set_text(app.page_subtitles[VIEW_DASHBOARD],
             "Your campus-ready space to discover internships and manage applications.");
    set_text(app.page_titles[VIEW_FIND], "Find internships");
    set_text(app.page_subtitles[VIEW_FIND],
             "Search opportunities, compare requirements, and find the right fit.");
    set_text(app.page_titles[VIEW_APPLICATIONS], "My applications");
    set_text(app.page_subtitles[VIEW_APPLICATIONS],
             "Your submissions, shown in first-in first-out application order.");
    set_text(app.page_titles[VIEW_ELIGIBILITY], "Eligibility checker");
    set_text(app.page_subtitles[VIEW_ELIGIBILITY],
             "Match a student's CGPA and skills against an internship's requirements.");
    set_text(app.page_titles[VIEW_MANAGE], "Manage internships");
    set_text(app.page_subtitles[VIEW_MANAGE],
             "Add or remove listings. Changes update the linked list and search indexes.");
    set_text(app.page_titles[PANEL_DETAILS], "Internship details");
    set_text(app.page_subtitles[PANEL_DETAILS],
             "Review the complete listing, then continue to apply or return to search.");

    make_label(VIEW_DASHBOARD, "YOUR OVERVIEW", 250, 118, 400, 24);
    make_label(VIEW_DASHBOARD, "A quick snapshot of opportunities and activity", 250, 143, 600, 22);
    {
        const char *labels[] = {"TOTAL INTERNSHIPS", "APPLICATIONS", "AVAILABLE DOMAINS"};
        int ids[] = {ID_METRIC_INTERNSHIPS, ID_METRIC_APPLICATIONS, ID_METRIC_DOMAINS};
        for (i = 0; i < 3; i++) {
            int x = 250 + i * 286;
            HWND label = make_label(VIEW_DASHBOARD, labels[i], x + 20, 184, 240, 22);
            app.metric_labels[i] = label;
            app.metric_values[i] = make_label(VIEW_DASHBOARD, "0", x + 20, 214, 245, 52);
            SendMessageA(label, WM_SETFONT, (WPARAM)app.font, TRUE);
            SendMessageA(app.metric_values[i], WM_SETFONT, (WPARAM)app.heading_font, TRUE);
            (void)ids[i];
        }
        make_label(VIEW_DASHBOARD, "GET STARTED", 250, 330, 400, 24);
        make_label(VIEW_DASHBOARD, "Choose a section to continue", 250, 356, 450, 22);
        make_button(VIEW_DASHBOARD, "Browse internships", 250, 399, 210, 48, ID_DASH_FIND);
        make_button(VIEW_DASHBOARD, "View applications", 476, 399, 210, 48, ID_DASH_APPLICATIONS);
        make_button(VIEW_DASHBOARD, "Check eligibility", 702, 399, 210, 48, ID_DASH_ELIGIBILITY);
        make_button(VIEW_DASHBOARD, "Manage listings", 928, 399, 210, 48, ID_DASH_MANAGE);
        app.dashboard_info = make_label(VIEW_DASHBOARD,
                   "InternHub keeps the course data structures at the heart of the app: "
                   "linked-list records, hash lookup, stipend tree, and a FIFO application queue.",
                   250, 497, 850, 56);
    }

    make_label(VIEW_FIND, "Search by ID, company, title, domain, or skill", 250, 112, 460, 22);
    app.search = make_control(VIEW_FIND, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                              250, 138, 294, 32, ID_FIND_SEARCH);
    SendMessageA(app.search, EM_LIMITTEXT, SKILLS_LEN - 1, 0);
    app.domain_label = make_label(VIEW_FIND, "Domain", 559, 112, 100, 22);
    app.domain = make_control(VIEW_FIND, "COMBOBOX", "",
                              CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                              559, 138, 190, 220, ID_FIND_DOMAIN);
    make_button(VIEW_FIND, "Search", 763, 137, 94, 34, ID_FIND_GO);
    make_button(VIEW_FIND, "Binary ID", 864, 137, 100, 34, ID_FIND_BINARY);
    make_button(VIEW_FIND, "Reset", 971, 137, 82, 34, ID_FIND_RESET);
    make_button(VIEW_FIND, "Sort stipend", 250, 187, 126, 34, ID_FIND_STIPEND_SORT);
    make_button(VIEW_FIND, "Sort deadline", 385, 187, 132, 34, ID_FIND_DEADLINE_SORT);
    app.stipend_label = make_label(VIEW_FIND, "Stipend at least", 540, 193, 122, 22);
    app.stipend = make_control(VIEW_FIND, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                               666, 187, 120, 32, ID_FIND_MIN_STIPEND);
    make_button(VIEW_FIND, "Find in BST", 795, 187, 112, 34, ID_FIND_STIPEND_GO);
    make_button(VIEW_FIND, "View details", 917, 187, 118, 34, ID_FIND_DETAILS);
    make_button(VIEW_FIND, "Apply now", 1045, 187, 110, 34, ID_FIND_APPLY);
    create_list_view(&app.find_list, VIEW_FIND, ID_FIND_LIST, 250, 239, 1000, 410,
                     internship_headers, internship_widths, 9);

    create_list_view(&app.application_list, VIEW_APPLICATIONS, ID_APPLICATION_LIST,
                     250, 154, 1000, 500, application_headers, application_widths, 6);

    make_label(VIEW_ELIGIBILITY, "STUDENT PROFILE", 250, 120, 350, 24);
    app.eligibility_labels[0] = make_label(VIEW_ELIGIBILITY, "Student ID", 250, 160, 145, 22);
    app.eligibility_labels[1] = make_label(VIEW_ELIGIBILITY, "Full name", 435, 160, 240, 22);
    app.eligibility_labels[2] = make_label(VIEW_ELIGIBILITY, "Year (1-4)", 720, 160, 125, 22);
    app.eligibility_labels[3] = make_label(VIEW_ELIGIBILITY, "CGPA (0-10)", 885, 160, 145, 22);
    app.student_id = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                  WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                  250, 187, 160, 34, ID_ELIG_STUDENT_ID);
    app.student_name = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                    WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                    435, 187, 255, 34, ID_ELIG_NAME);
    app.student_year = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                    WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                    720, 187, 125, 34, ID_ELIG_YEAR);
    app.student_cgpa = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                    WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                    885, 187, 145, 34, ID_ELIG_CGPA);
    app.eligibility_labels[4] = make_label(VIEW_ELIGIBILITY, "Skills (comma-separated)",
                                           250, 241, 600, 22);
    app.student_skills = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                      WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                      250, 268, 780, 34, ID_ELIG_SKILLS);
    app.eligibility_labels[5] = make_label(VIEW_ELIGIBILITY, "Internship ID", 250, 322, 180, 22);
    app.internship_id = make_control(VIEW_ELIGIBILITY, "EDIT", "",
                                     WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                                     250, 349, 180, 34, ID_ELIG_INTERNSHIP_ID);
    make_button(VIEW_ELIGIBILITY, "Check eligibility", 446, 348, 168, 38, ID_ELIG_CHECK);
    make_button(VIEW_ELIGIBILITY, "Apply now", 626, 348, 132, 38, ID_ELIG_APPLY);
    app.eligibility_result = make_control(VIEW_ELIGIBILITY, "STATIC",
                                          "Enter student details and an Internship ID to check.",
                                          SS_LEFT, 250, 418, 850, 66, ID_ELIG_RESULT);

    app.detail_labels[0] = make_label(PANEL_DETAILS, "COMPANY", 270, 135, 210, 22);
    app.detail_values[0] = make_label(PANEL_DETAILS, "", 270, 160, 450, 28);
    app.detail_labels[1] = make_label(PANEL_DETAILS, "INTERNSHIP TITLE", 270, 209, 240, 22);
    app.detail_values[1] = make_label(PANEL_DETAILS, "", 270, 234, 660, 28);
    app.detail_labels[2] = make_label(PANEL_DETAILS, "DOMAIN", 270, 283, 220, 22);
    app.detail_values[2] = make_label(PANEL_DETAILS, "", 270, 308, 500, 28);
    app.detail_labels[3] = make_label(PANEL_DETAILS, "REQUIRED SKILLS", 270, 357, 240, 22);
    app.detail_values[3] = make_label(PANEL_DETAILS, "", 270, 382, 760, 42);
    app.detail_labels[4] = make_label(PANEL_DETAILS, "DURATION", 270, 447, 150, 22);
    app.detail_values[4] = make_label(PANEL_DETAILS, "", 270, 472, 180, 28);
    app.detail_labels[5] = make_label(PANEL_DETAILS, "STIPEND", 495, 447, 150, 22);
    app.detail_values[5] = make_label(PANEL_DETAILS, "", 495, 472, 225, 28);
    app.detail_labels[6] = make_label(PANEL_DETAILS, "MINIMUM CGPA", 760, 447, 165, 22);
    app.detail_values[6] = make_label(PANEL_DETAILS, "", 760, 472, 140, 28);
    app.detail_labels[7] = make_label(PANEL_DETAILS, "APPLICATION DEADLINE", 270, 530, 260, 22);
    app.detail_values[7] = make_label(PANEL_DETAILS, "", 270, 555, 220, 28);
    make_button(PANEL_DETAILS, "Back to internships", 270, 620, 180, 40, ID_DETAILS_BACK);
    make_button(PANEL_DETAILS, "Apply now", 470, 620, 135, 40, ID_DETAILS_APPLY);

    make_button(VIEW_MANAGE, "Add internship", 250, 119, 150, 38, ID_MANAGE_ADD);
    make_button(VIEW_MANAGE, "Delete selected", 414, 119, 155, 38, ID_MANAGE_DELETE);
    make_label(VIEW_MANAGE, "Select a row before deleting. The indexes update with each change.",
               590, 127, 610, 24);
    create_list_view(&app.manage_list, VIEW_MANAGE, ID_MANAGE_LIST, 250, 177, 1000, 470,
                     internship_headers, internship_widths, 9);
}

static void move_panel_controls(HWND window)
{
    RECT client;
    int width, height, content_width, list_height, left;
    GetClientRect(window, &client);
    width = client.right;
    height = client.bottom;
    content_width = width - SIDEBAR_WIDTH - 54;
    if (content_width < 300) content_width = 300;
    left = SIDEBAR_WIDTH + 30;
    list_height = height - 340;
    if (list_height < 130) list_height = 130;

    MoveWindow(app.find_list, left, 276,
               content_width, list_height, TRUE);
    MoveWindow(app.manage_list, SIDEBAR_WIDTH + 30, 177,
               content_width, height - 290, TRUE);
    MoveWindow(app.application_list, SIDEBAR_WIDTH + 30, 154,
               content_width, height - 235, TRUE);

    MoveWindow(app.page_titles[VIEW_DASHBOARD], SIDEBAR_WIDTH + 30, 24,
               content_width, 38, TRUE);
    MoveWindow(app.page_subtitles[VIEW_DASHBOARD], SIDEBAR_WIDTH + 32, 62,
               content_width, 24, TRUE);
    for (int i = 0; i < PANEL_TOTAL; i++) {
        if (i == VIEW_DASHBOARD) continue;
        MoveWindow(app.page_titles[i], SIDEBAR_WIDTH + 30, 24, content_width, 38, TRUE);
        MoveWindow(app.page_subtitles[i], SIDEBAR_WIDTH + 32, 62, content_width, 24, TRUE);
    }

    if (app.active_view == VIEW_DASHBOARD) {
        int card_width = (content_width - 36) / 3;
        for (int i = 0; i < 3; i++) {
            int x = left + i * (card_width + 18);
            MoveWindow(app.metric_labels[i], x + 20, 184, card_width - 38, 22, TRUE);
            MoveWindow(app.metric_values[i], x + 20, 214, card_width - 38, 52, TRUE);
        }
        for (int i = 0; i < 4; i++) {
            int card_gap = 16;
            int button_width = (content_width - 3 * card_gap) / 4;
            MoveWindow(GetDlgItem(window, ID_DASH_FIND + i),
                       left + i * (button_width + card_gap),
                       399, button_width, 48, TRUE);
        }
        if (height < 650) {
            MoveWindow(app.dashboard_info, left, height - 65, content_width, 48, TRUE);
        } else {
            MoveWindow(app.dashboard_info, left, 497, content_width, 56, TRUE);
        }
    }
    {
        int gap = 6;
        int usable = content_width - 4 * gap;
        int search_width = usable * 34 / 100;
        int domain_width = usable * 23 / 100;
        int search_button_width = usable * 15 / 100;
        int binary_width = usable * 15 / 100;
        int reset_width = usable - search_width - domain_width -
                          search_button_width - binary_width - 4 * gap;
        int x = left;
        MoveWindow(app.search, x, 138, search_width, 32, TRUE);
        x += search_width + gap;
        MoveWindow(app.domain, x, 138, domain_width, 220, TRUE);
        MoveWindow(app.domain_label, x, 112, domain_width, 22, TRUE);
        x += domain_width + gap;
        MoveWindow(GetDlgItem(window, ID_FIND_GO), x, 137, search_button_width, 34, TRUE);
        x += search_button_width + gap;
        MoveWindow(GetDlgItem(window, ID_FIND_BINARY), x, 137, binary_width, 34, TRUE);
        x += binary_width + gap;
        MoveWindow(GetDlgItem(window, ID_FIND_RESET), x, 137, reset_width, 34, TRUE);

        x = left;
        {
            int sort_width = content_width * 18 / 100;
            int deadline_width = content_width * 18 / 100;
            int label_width = content_width * 17 / 100;
            int edit_width = content_width * 17 / 100;
            int tree_width = content_width - sort_width - deadline_width -
                             label_width - edit_width - 4 * gap;
            MoveWindow(GetDlgItem(window, ID_FIND_STIPEND_SORT), x, 187, sort_width, 34, TRUE);
            x += sort_width + gap;
            MoveWindow(GetDlgItem(window, ID_FIND_DEADLINE_SORT), x, 187, deadline_width, 34, TRUE);
            x += deadline_width + gap;
            MoveWindow(app.stipend_label, x, 193, label_width, 22, TRUE);
            x += label_width + gap;
            MoveWindow(GetDlgItem(window, ID_FIND_MIN_STIPEND), x, 187, edit_width, 32, TRUE);
            x += edit_width + gap;
            MoveWindow(GetDlgItem(window, ID_FIND_STIPEND_GO), x, 187, tree_width, 34, TRUE);
        }
        MoveWindow(GetDlgItem(window, ID_FIND_DETAILS), width - 255, 231, 112, 34, TRUE);
        MoveWindow(GetDlgItem(window, ID_FIND_APPLY), width - 135, 231, 110, 34, TRUE);
    }
    if (content_width < 760) {
        int half = (content_width - 14) / 2;
        int right_x = left + half + 14;
        MoveWindow(app.eligibility_labels[0], left, 154, half, 20, TRUE);
        MoveWindow(app.eligibility_labels[1], right_x, 154, half, 20, TRUE);
        MoveWindow(app.student_id, left, 178, half, 32, TRUE);
        MoveWindow(app.student_name, right_x, 178, half, 32, TRUE);
        MoveWindow(app.eligibility_labels[2], left, 224, half, 20, TRUE);
        MoveWindow(app.eligibility_labels[3], right_x, 224, half, 20, TRUE);
        MoveWindow(app.student_year, left, 248, half, 32, TRUE);
        MoveWindow(app.student_cgpa, right_x, 248, half, 32, TRUE);
        MoveWindow(app.eligibility_labels[4], left, 294, content_width, 20, TRUE);
        MoveWindow(app.student_skills, left, 318, content_width, 32, TRUE);
        MoveWindow(app.eligibility_labels[5], left, 364, 120, 20, TRUE);
        MoveWindow(app.internship_id, left, 388, 130, 32, TRUE);
        MoveWindow(GetDlgItem(window, ID_ELIG_CHECK), left + 142, 387, 150, 36, TRUE);
        MoveWindow(GetDlgItem(window, ID_ELIG_APPLY), left + 302, 387, 110, 36, TRUE);
        MoveWindow(app.eligibility_result, left, 444, content_width, 60, TRUE);
    } else {
        int wide = (content_width - 36) / 4;
        MoveWindow(app.eligibility_labels[0], left, 160, wide, 20, TRUE);
        MoveWindow(app.eligibility_labels[1], left + wide + 12, 160, wide, 20, TRUE);
        MoveWindow(app.eligibility_labels[2], left + 2 * (wide + 12), 160, wide, 20, TRUE);
        MoveWindow(app.eligibility_labels[3], left + 3 * (wide + 12), 160, wide, 20, TRUE);
        MoveWindow(app.student_id, left, 187, wide, 34, TRUE);
        MoveWindow(app.student_name, left + wide + 12, 187, wide, 34, TRUE);
        MoveWindow(app.student_year, left + 2 * (wide + 12), 187, wide, 34, TRUE);
        MoveWindow(app.student_cgpa, left + 3 * (wide + 12), 187, wide, 34, TRUE);
        MoveWindow(app.eligibility_labels[4], left, 241, content_width, 22, TRUE);
        MoveWindow(app.student_skills, left, 268, content_width, 34, TRUE);
        MoveWindow(app.eligibility_labels[5], left, 322, 180, 22, TRUE);
        MoveWindow(app.internship_id, left, 349, 180, 34, TRUE);
        MoveWindow(GetDlgItem(window, ID_ELIG_CHECK), left + 196, 348, 168, 38, TRUE);
        MoveWindow(GetDlgItem(window, ID_ELIG_APPLY), left + 376, 348, 132, 38, TRUE);
        MoveWindow(app.eligibility_result, left, 418, content_width, 66, TRUE);
    }
    {
        int detail_left = SIDEBAR_WIDTH + 50;
        int detail_width = width - detail_left - 36;
        int third = (detail_width - 24) / 3;
        int compact = height < 650;
        int company_label_y = compact ? 118 : 135;
        int title_label_y = compact ? 166 : 209;
        int domain_label_y = compact ? 214 : 283;
        int skills_label_y = compact ? 262 : 357;
        int metric_label_y = compact ? 340 : 447;
        int deadline_y = compact ? height - 122 : 530;
        int button_y = compact ? height - 58 : 620;
        if (detail_width < 280) detail_width = 280;
        MoveWindow(app.detail_labels[0], detail_left, company_label_y, 210, 22, TRUE);
        MoveWindow(app.detail_values[0], detail_left, company_label_y + 25, detail_width, 28, TRUE);
        MoveWindow(app.detail_labels[1], detail_left, title_label_y, 240, 22, TRUE);
        MoveWindow(app.detail_values[1], detail_left, title_label_y + 25, detail_width, 28, TRUE);
        MoveWindow(app.detail_labels[2], detail_left, domain_label_y, 220, 22, TRUE);
        MoveWindow(app.detail_values[2], detail_left, domain_label_y + 25, detail_width, 28, TRUE);
        MoveWindow(app.detail_labels[3], detail_left, skills_label_y, 240, 22, TRUE);
        MoveWindow(app.detail_values[3], detail_left, skills_label_y + 25, detail_width, 42, TRUE);
        MoveWindow(app.detail_labels[4], detail_left, metric_label_y, third, 22, TRUE);
        MoveWindow(app.detail_values[4], detail_left, metric_label_y + 25, third, 28, TRUE);
        MoveWindow(app.detail_labels[5], detail_left + third + 12, metric_label_y, third, 22, TRUE);
        MoveWindow(app.detail_values[5], detail_left + third + 12, metric_label_y + 25, third, 28, TRUE);
        MoveWindow(app.detail_labels[6], detail_left + 2 * (third + 12),
                   metric_label_y, third, 22, TRUE);
        MoveWindow(app.detail_values[6], detail_left + 2 * (third + 12),
                   metric_label_y + 25, third, 28, TRUE);
        MoveWindow(app.detail_labels[7], detail_left, deadline_y, 260, 22, TRUE);
        MoveWindow(app.detail_values[7], detail_left, deadline_y + 25, 220, 28, TRUE);
        MoveWindow(GetDlgItem(window, ID_DETAILS_BACK), detail_left, button_y, 180, 40, TRUE);
        MoveWindow(GetDlgItem(window, ID_DETAILS_APPLY), detail_left + 200, button_y, 135, 40, TRUE);
    }
}

static void draw_button(const DRAWITEMSTRUCT *draw)
{
    HDC dc = draw->hDC;
    RECT rect = draw->rcItem;
    int id = GetDlgCtrlID(draw->hwndItem);
    COLORREF fill = COLOR_BLUE;
    COLORREF text = COLOR_WHITE;
    HBRUSH brush;
    int nav = id >= ID_NAV_DASHBOARD && id <= ID_NAV_MANAGE;
    int active_nav = id == ID_NAV_DASHBOARD + app.active_view ||
                     (app.active_view == PANEL_DETAILS && id == ID_NAV_FIND);
    if (nav) {
        fill = active_nav ? COLOR_BLUE : COLOR_NAVY;
    } else if (id == ID_NAV_EXIT || id == ID_MANAGE_DELETE) {
        fill = id == ID_NAV_EXIT ? COLOR_NAVY : COLOR_RED;
    } else if (id == ID_FIND_RESET || id == ID_DETAILS_BACK) {
        fill = RGB(231, 237, 246);
        text = COLOR_TEXT;
    }
    if (draw->itemState & ODS_SELECTED) {
        if (fill == COLOR_BLUE) fill = COLOR_BLUE_DARK;
        else fill = COLOR_NAVY_LIGHT;
    }
    brush = CreateSolidBrush(fill);
    SelectObject(dc, brush);
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
    DeleteObject(brush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, text);
    {
        char caption[128];
        GetWindowTextA(draw->hwndItem, caption, sizeof(caption));
        DrawTextA(dc, caption, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (draw->itemState & ODS_FOCUS) {
        InflateRect(&rect, -5, -5);
        DrawFocusRect(dc, &rect);
    }
}

static void paint_dashboard_cards(HWND window)
{
    RECT client;
    int content_width, card_width, info_top, info_bottom, i;
    HDC dc;
    HBRUSH card_brush;
    GetClientRect(window, &client);
    content_width = client.right - SIDEBAR_WIDTH - 54;
    if (content_width < 300) content_width = 300;
    card_width = (content_width - 36) / 3;
    info_top = client.bottom < 650 ? client.bottom - 72 : 480;
    info_bottom = client.bottom < 650 ? client.bottom - 17 : 568;
    dc = GetDC(window);
    card_brush = CreateSolidBrush(COLOR_WHITE);
    for (i = 0; i < 3; i++) {
        RECT card = {SIDEBAR_WIDTH + 30 + i * (card_width + 18), 174,
                     SIDEBAR_WIDTH + 30 + i * (card_width + 18) + card_width, 286};
        FillRect(dc, &card, card_brush);
        FrameRect(dc, &card, app.white_brush);
    }
    {
        RECT info = {SIDEBAR_WIDTH + 30, info_top, client.right - 24, info_bottom};
        FillRect(dc, &info, card_brush);
        FrameRect(dc, &info, app.white_brush);
    }
    DeleteObject(card_brush);
    ReleaseDC(window, dc);
}

static void paint_sidebar(HWND window)
{
    RECT client;
    RECT side;
    HDC dc;
    HBRUSH brush;
    GetClientRect(window, &client);
    side.left = 0;
    side.top = 0;
    side.right = SIDEBAR_WIDTH;
    side.bottom = client.bottom;
    dc = GetDC(window);
    brush = CreateSolidBrush(COLOR_NAVY);
    FillRect(dc, &side, brush);
    DeleteObject(brush);
    ReleaseDC(window, dc);
    MoveWindow(GetDlgItem(window, ID_NAV_EXIT), 16, client.bottom - 66,
               SIDEBAR_WIDTH - 32, 44, TRUE);
}

static void create_dialog_control(HWND window, const char *class_name, const char *text,
                                 DWORD style, int x, int y, int width, int height,
                                 int id, HFONT font)
{
    HWND control = CreateWindowExA(0, class_name, text, WS_CHILD | WS_VISIBLE | style,
                                   x, y, width, height, window, (HMENU)(INT_PTR)id,
                                   GetModuleHandleA(NULL), NULL);
    if (control != NULL) SendMessageA(control, WM_SETFONT, (WPARAM)font, TRUE);
}

static void dialog_get_field(HWND window, int index, char *buffer, size_t capacity)
{
    GetWindowTextA(GetDlgItem(window, ID_DIALOG_FIRST_FIELD + index), buffer, (int)capacity);
    buffer[capacity - 1] = '\0';
}

static LRESULT CALLBACK add_dialog_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    AddDialogState *state = (AddDialogState *)GetWindowLongPtrA(window, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        CREATESTRUCTA *creation = (CREATESTRUCTA *)lparam;
        SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)creation->lpCreateParams);
        return DefWindowProcA(window, message, wparam, lparam);
    }
    if (message == WM_CREATE) {
        static const char *labels[] = {
            "Internship ID", "Company", "Title", "Domain", "Required skills",
            "Duration (months)", "Stipend", "Minimum CGPA", "Deadline (YYYY-MM-DD)"
        };
        int i;
        for (i = 0; i < 9; i++) {
            int column = i < 5 ? 0 : 1;
            int row = i < 5 ? i : i - 5;
            int x = column == 0 ? 24 : 330;
            int y = 26 + row * 61;
            create_dialog_control(window, "STATIC", labels[i], 0, x, y, 265, 20, 0, app.font);
            create_dialog_control(window, "EDIT", "", WS_BORDER | ES_AUTOHSCROLL,
                                  x, y + 22, 270, 25, ID_DIALOG_FIRST_FIELD + i, app.font);
        }
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD, EM_LIMITTEXT, 10, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 1, EM_LIMITTEXT, COMPANY_LEN - 1, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 2, EM_LIMITTEXT, TITLE_LEN - 1, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 3, EM_LIMITTEXT, DOMAIN_LEN - 1, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 4, EM_LIMITTEXT, SKILLS_LEN - 1, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 5, EM_LIMITTEXT, 3, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 6, EM_LIMITTEXT, 12, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 7, EM_LIMITTEXT, 5, 0);
        SendDlgItemMessageA(window, ID_DIALOG_FIRST_FIELD + 8, EM_LIMITTEXT, 10, 0);
        create_dialog_control(window, "BUTTON", "Add internship", BS_DEFPUSHBUTTON,
                              380, 354, 128, 36, ID_DIALOG_OK, app.font);
        create_dialog_control(window, "BUTTON", "Cancel", 0,
                              520, 354, 88, 36, ID_DIALOG_CANCEL, app.font);
        return 0;
    }
    if (message == WM_COMMAND && state != NULL) {
        if (LOWORD(wparam) == ID_DIALOG_CANCEL) {
            DestroyWindow(window);
            return 0;
        }
        if (LOWORD(wparam) == ID_DIALOG_OK) {
            char id[32], duration[32], stipend[32], cgpa[32];
            Internship *item = &state->result;
            dialog_get_field(window, 0, id, sizeof(id));
            dialog_get_field(window, 1, item->company, sizeof(item->company));
            dialog_get_field(window, 2, item->title, sizeof(item->title));
            dialog_get_field(window, 3, item->domain, sizeof(item->domain));
            dialog_get_field(window, 4, item->skills, sizeof(item->skills));
            dialog_get_field(window, 5, duration, sizeof(duration));
            dialog_get_field(window, 6, stipend, sizeof(stipend));
            dialog_get_field(window, 7, cgpa, sizeof(cgpa));
            dialog_get_field(window, 8, item->deadline, sizeof(item->deadline));
            if (!parse_int(id, &item->id) || item->company[0] == '\0' ||
                item->title[0] == '\0' || item->domain[0] == '\0' ||
                item->skills[0] == '\0' || !parse_int(duration, &item->duration_months) ||
                !parse_double(stipend, &item->stipend) || item->stipend < 0.0 ||
                !parse_double(cgpa, &item->min_cgpa) || item->min_cgpa < 0.0 ||
                item->min_cgpa > 10.0 || !valid_deadline(item->deadline) ||
                text_has_pipe(item->company) || text_has_pipe(item->title) ||
                text_has_pipe(item->domain) || text_has_pipe(item->skills)) {
                MessageBoxA(window, "Check each field. Use numeric values where required, "
                                   "CGPA from 0 to 10, and a YYYY-MM-DD deadline.",
                            "Invalid internship", MB_ICONWARNING | MB_OK);
                return 0;
            }
            state->accepted = 1;
            DestroyWindow(window);
            return 0;
        }
    }
    if (message == WM_CLOSE) {
        DestroyWindow(window);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

static int run_add_dialog(Internship *result)
{
    AddDialogState state;
    RECT owner_rect;
    HWND dialog;
    MSG message;
    ZeroMemory(&state, sizeof(state));
    GetWindowRect(app.window, &owner_rect);
    EnableWindow(app.window, FALSE);
    dialog = CreateWindowExA(WS_EX_DLGMODALFRAME, DIALOG_CLASS, "Add an internship",
                             WS_CAPTION | WS_SYSMENU | WS_POPUP,
                             owner_rect.left + 180, owner_rect.top + 100, 640, 440,
                             app.window, NULL, GetModuleHandleA(NULL), &state);
    if (dialog == NULL) {
        EnableWindow(app.window, TRUE);
        MessageBoxA(app.window, "Could not open the internship form.",
                    "InternHub", MB_ICONERROR | MB_OK);
        return 0;
    }
    ShowWindow(dialog, SW_SHOW);
    UpdateWindow(dialog);
    while (IsWindow(dialog) && GetMessageA(&message, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(dialog, &message)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    EnableWindow(app.window, TRUE);
    SetActiveWindow(app.window);
    if (state.accepted) *result = state.result;
    return state.accepted;
}

static LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE) {
        app.window = window;
        return DefWindowProcA(window, message, wparam, lparam);
    }
    if (message == WM_CREATE) {
        create_main_controls();
        collect_domains();
        refresh_manage_list();
        refresh_applications();
        update_dashboard_metrics();
        show_view(VIEW_DASHBOARD);
        move_panel_controls(window);
        return 0;
    }
    if (message == WM_SIZE) {
        move_panel_controls(window);
        paint_sidebar(window);
        InvalidateRect(window, NULL, TRUE);
        return 0;
    }
    if (message == WM_GETMINMAXINFO) {
        MINMAXINFO *limits = (MINMAXINFO *)lparam;
        limits->ptMinTrackSize.x = 760;
        limits->ptMinTrackSize.y = 500;
        return 0;
    }
    if (message == WM_DRAWITEM) {
        draw_button((const DRAWITEMSTRUCT *)lparam);
        return TRUE;
    }
    if (message == WM_COMMAND) {
        switch (LOWORD(wparam)) {
        case ID_NAV_DASHBOARD: show_view(VIEW_DASHBOARD); return 0;
        case ID_NAV_FIND: show_view(VIEW_FIND); run_search(); return 0;
        case ID_NAV_APPLICATIONS: refresh_applications(); show_view(VIEW_APPLICATIONS); return 0;
        case ID_NAV_ELIGIBILITY:
            app.result_state = 0;
            set_text(app.eligibility_result, "Enter student details and an Internship ID to check.");
            show_view(VIEW_ELIGIBILITY);
            return 0;
        case ID_NAV_MANAGE: refresh_manage_list(); show_view(VIEW_MANAGE); return 0;
        case ID_NAV_EXIT: DestroyWindow(window); return 0;
        case ID_DASH_FIND: show_view(VIEW_FIND); run_search(); return 0;
        case ID_DASH_APPLICATIONS: refresh_applications(); show_view(VIEW_APPLICATIONS); return 0;
        case ID_DASH_ELIGIBILITY: show_view(VIEW_ELIGIBILITY); return 0;
        case ID_DASH_MANAGE: show_view(VIEW_MANAGE); return 0;
        case ID_FIND_GO: run_search(); return 0;
        case ID_FIND_BINARY: run_binary_search(); return 0;
        case ID_FIND_RESET:
            set_text(app.search, "");
            SendMessageA(app.domain, CB_SETCURSEL, 0, 0);
            run_search();
            return 0;
        case ID_FIND_STIPEND_SORT: {
            Internship *all[MAX_INTERNSHIPS];
            app.find_count = collect_all(all);
            sort_by_stipend(all, app.find_count);
            memcpy(app.find_items, all, (size_t)app.find_count * sizeof(all[0]));
            show_internships(app.find_list, app.find_items, app.find_count);
            return 0;
        }
        case ID_FIND_DEADLINE_SORT: {
            Internship *all[MAX_INTERNSHIPS];
            app.find_count = collect_all(all);
            sort_by_deadline(all, app.find_count);
            memcpy(app.find_items, all, (size_t)app.find_count * sizeof(all[0]));
            show_internships(app.find_list, app.find_items, app.find_count);
            return 0;
        }
        case ID_FIND_STIPEND_GO: {
            char text[32];
            double minimum;
            read_text(app.stipend, text, sizeof(text));
            if (!parse_double(text, &minimum) || minimum < 0.0) {
                MessageBoxA(window, "Enter a non-negative minimum stipend.",
                            "Stipend tree search", MB_ICONINFORMATION | MB_OK);
                return 0;
            }
            app.find_count = stipend_bst_collect_at_least(*app.stipend_root, minimum,
                                                           app.find_items, MAX_INTERNSHIPS);
            show_internships(app.find_list, app.find_items, app.find_count);
            return 0;
        }
        case ID_FIND_DETAILS:
            show_details(selected_find_id());
            return 0;
        case ID_FIND_APPLY: {
            char id_text[32];
            int id = selected_find_id();
            if (id == 0) {
                MessageBoxA(window, "Select an internship from the table first.",
                            "Apply now", MB_ICONINFORMATION | MB_OK);
                return 0;
            }
            (void)snprintf(id_text, sizeof(id_text), "%d", id);
            set_text(app.internship_id, id_text);
            show_view(VIEW_ELIGIBILITY);
            (void)check_eligibility(1);
            return 0;
        }
        case ID_MANAGE_ADD: handle_add_internship(); return 0;
        case ID_MANAGE_DELETE: handle_delete_internship(); return 0;
        case ID_ELIG_CHECK: (void)check_eligibility(0); return 0;
        case ID_ELIG_APPLY: (void)check_eligibility(1); return 0;
        case ID_DETAILS_BACK: show_view(VIEW_FIND); return 0;
        case ID_DETAILS_APPLY: {
            char id_text[32];
            (void)snprintf(id_text, sizeof(id_text), "%d", app.selected_internship_id);
            set_text(app.internship_id, id_text);
            show_view(VIEW_ELIGIBILITY);
            (void)check_eligibility(1);
            return 0;
        }
        default:
            if (HIWORD(wparam) == CBN_SELCHANGE && LOWORD(wparam) == ID_FIND_DOMAIN) {
                run_search();
                return 0;
            }
            break;
        }
    }
    if (message == WM_NOTIFY) {
        NMHDR *header = (NMHDR *)lparam;
        if (header->code == NM_CUSTOMDRAW &&
            (header->hwndFrom == app.find_list || header->hwndFrom == app.manage_list ||
             header->hwndFrom == app.application_list)) {
            NMLVCUSTOMDRAW *draw = (NMLVCUSTOMDRAW *)lparam;
            if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
            if (draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                draw->clrTextBk = ((draw->nmcd.dwItemSpec % 2) == 0)
                    ? COLOR_WHITE : RGB(248, 250, 253);
                return CDRF_NOTIFYSUBITEMDRAW;
            }
            if (draw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
                if (header->hwndFrom == app.application_list && draw->iSubItem == 3) {
                    char status[STATUS_LEN];
                    ListView_GetItemText(app.application_list, (int)draw->nmcd.dwItemSpec,
                                         3, status, sizeof(status));
                    draw->clrText = _stricmp(status, "Rejected") == 0 ? COLOR_RED : COLOR_ORANGE;
                }
                return CDRF_NEWFONT;
            }
        }
        if ((header->hwndFrom == app.find_list || header->hwndFrom == app.manage_list) &&
            (header->code == NM_DBLCLK || header->code == LVN_ITEMACTIVATE)) {
            int id = internship_id_from_list(header->hwndFrom);
            if (id != 0) show_details(id);
            return 0;
        }
        if (header->hwndFrom == app.find_list && header->code == LVN_ITEMCHANGED) {
            int id = internship_id_from_list(app.find_list);
            if (id != 0) app.selected_internship_id = id;
        }
        return 0;
    }
    if (message == WM_CTLCOLORSTATIC) {
        HDC dc = (HDC)wparam;
        HWND control = (HWND)lparam;
        int id = GetDlgCtrlID(control);
        SetBkMode(dc, TRANSPARENT);
        if (control == app.brand_label) {
            SetTextColor(dc, COLOR_WHITE);
            return (LRESULT)app.sidebar_brush;
        } else if (control == app.brand_caption) {
            SetTextColor(dc, RGB(181, 194, 214));
            return (LRESULT)app.sidebar_brush;
        } else if (control == app.metric_labels[0] || control == app.metric_labels[1] ||
                   control == app.metric_labels[2] || control == app.metric_values[0] ||
                   control == app.metric_values[1] || control == app.metric_values[2]) {
            SetTextColor(dc, control == app.metric_values[0] ||
                             control == app.metric_values[1] ||
                             control == app.metric_values[2] ? COLOR_BLUE : COLOR_MUTED);
            return (LRESULT)app.white_brush;
        } else if (id == ID_ELIG_RESULT) {
            SetTextColor(dc, app.result_state > 0 ? COLOR_GREEN :
                             app.result_state < 0 ? COLOR_RED : COLOR_MUTED);
        } else if (id == 0 && GetParent(control) == window &&
                   (GetWindowLongPtrA(control, GWL_STYLE) & SS_OWNERDRAW) != 0) {
            SetTextColor(dc, COLOR_WHITE);
        } else {
            SetTextColor(dc, COLOR_TEXT);
        }
        return (LRESULT)app.background_brush;
    }
    if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX) {
        HDC dc = (HDC)wparam;
        SetTextColor(dc, COLOR_TEXT);
        SetBkColor(dc, COLOR_WHITE);
        return (LRESULT)app.white_brush;
    }
    if (message == WM_ERASEBKGND) {
        RECT rect;
        GetClientRect(window, &rect);
        FillRect((HDC)wparam, &rect, app.background_brush);
        return 1;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        paint_sidebar(window);
        if (app.active_view == VIEW_DASHBOARD) paint_dashboard_cards(window);
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

int run_student_finder_gui(HINSTANCE instance, InternshipList *internships,
                           InternshipHashTable *hash, StipendNode **stipend_root,
                           ApplicationQueue *applications)
{
    WNDCLASSEXA window_class;
    WNDCLASSEXA dialog_class;
    HWND window;
    MSG message;
    int result;

    ZeroMemory(&app, sizeof(app));
    app.internships = internships;
    app.hash = hash;
    app.stipend_root = stipend_root;
    app.applications = applications;
    app.font = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    app.heading_font = CreateFontA(25, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    app.background_brush = CreateSolidBrush(UI_BACKGROUND);
    app.white_brush = CreateSolidBrush(COLOR_WHITE);
    app.sidebar_brush = CreateSolidBrush(COLOR_NAVY);

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = main_window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = app.background_brush;
    window_class.lpszClassName = WINDOW_CLASS;
    if (!RegisterClassExA(&window_class)) {
        MessageBoxA(NULL, "Could not register the InternHub main window.",
                    "InternHub", MB_ICONERROR | MB_OK);
        result = 1;
        goto cleanup;
    }

    ZeroMemory(&dialog_class, sizeof(dialog_class));
    dialog_class.cbSize = sizeof(dialog_class);
    dialog_class.lpfnWndProc = add_dialog_proc;
    dialog_class.hInstance = instance;
    dialog_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    dialog_class.hbrBackground = app.background_brush;
    dialog_class.lpszClassName = DIALOG_CLASS;
    if (!RegisterClassExA(&dialog_class)) {
        MessageBoxA(NULL, "Could not register the internship form.",
                    "InternHub", MB_ICONERROR | MB_OK);
        UnregisterClassA(WINDOW_CLASS, instance);
        result = 1;
        goto cleanup;
    }

    window = CreateWindowExA(0, WINDOW_CLASS, "InternHub - Student Internship Finder",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1120, 720,
                             NULL, NULL, instance, NULL);
    if (window == NULL) {
        MessageBoxA(NULL, "Could not create the InternHub application window.",
                    "InternHub", MB_ICONERROR | MB_OK);
        UnregisterClassA(DIALOG_CLASS, instance);
        UnregisterClassA(WINDOW_CLASS, instance);
        result = 1;
        goto cleanup;
    }
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    UnregisterClassA(DIALOG_CLASS, instance);
    UnregisterClassA(WINDOW_CLASS, instance);
    result = (int)message.wParam;

cleanup:
    if (app.font != NULL) DeleteObject(app.font);
    if (app.heading_font != NULL) DeleteObject(app.heading_font);
    if (app.background_brush != NULL) DeleteObject(app.background_brush);
    if (app.white_brush != NULL) DeleteObject(app.white_brush);
    if (app.sidebar_brush != NULL) DeleteObject(app.sidebar_brush);
    return result;
}
