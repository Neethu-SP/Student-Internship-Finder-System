# InternHub – Student Internship Finder

A Windows desktop mini project for a Data Structures in C course. It uses the
Win32 API for its graphical interface and MinGW-w64 GCC for compilation. It
does not require Python, Java, Node.js, React, or a database.

## Features

- Resizable dashboard with a navy navigation sidebar and separate Dashboard,
  Find Internships, My Applications, Eligibility Checker, Manage Internships,
  and Internship Details views.
- Dashboard cards for the number of internships, applications, and available
  domains.
- Search and domain filtering across nine fictional internship records.
- Search by Internship ID (hash table) or text such as company, title, domain,
  and skills (linear search).
- A separate binary-search-by-ID action that sorts a temporary pointer list
  before searching.
- Sort by stipend or deadline.
- Find internships with a minimum stipend using the stipend binary search tree.
- Internship details and Apply Now actions, checking eligibility, applying, and
  viewing application statuses in a table.
- Add and delete records in the separate Manage Internships view.
- Plain-text persistence in `data/internships.txt` and
  `data/applications.txt`.

## Data structures

- `InternshipList` is the primary linked-list store.
- `InternshipHashTable` uses separate chaining to index Internship IDs.
- `StipendNode` is a binary search tree ordered by stipend, then ID.
- `ApplicationQueue` is a linked FIFO queue.
- `search_sort.c` contains linear search, binary search, and insertion sorts.

Adding or deleting an internship updates the linked list and both indexes.
Eligibility requires the student's CGPA to meet the minimum and every listed
required skill to appear as a comma-separated student skill.

## Build and run on Windows

Install MinGW-w64 GCC (for example, the MSYS2 UCRT64 toolchain), then open a
terminal in this project folder:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Isrc `
  src\main.c src\gui.c src\internship.c src\hash_table.c `
  src\stipend_bst.c src\application_queue.c src\search_sort.c `
  -o StudentInternshipFinder.exe -mwindows -lcomctl32
.\StudentInternshipFinder.exe
```

Alternatively, run `mingw32-make` if GNU Make is installed.
Run the data-structure tests with `mingw32-make test`.

Run the executable from the project folder so its relative `data` paths resolve.
The app creates `data/applications.txt` on first launch. Added and deleted
internships are saved back to `data/internships.txt`.

## Using InternHub

1. Use the sidebar to switch between the dashboard and feature views.
2. In **Find Internships**, search by text or ID, filter by domain, sort, or use
   the stipend-tree filter. Double-click a row or choose **View details** to open
   the details view.
3. In **Eligibility Checker**, enter a student ID, name, year (1-4), CGPA
   (0-10), comma-separated skills, and Internship ID. The result is shown in
   green or red; eligible students can submit from that view.
4. Choose **Apply now** from a listing or its details view to check the same
   eligibility form and submit the application.
5. **My Applications** displays application and student details in FIFO order.
   Manage records separately in **Manage Internships**.

The project keeps the data structures visible and modular; edit the implementation
files in `src/` to discuss each operation during a viva.
