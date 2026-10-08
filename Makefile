CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS = -mwindows -lcomctl32
TARGET = StudentInternshipFinder.exe
SOURCES = src/main.c src/gui.c src/internship.c src/hash_table.c \
          src/stipend_bst.c src/application_queue.c src/search_sort.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -Isrc $(SOURCES) -o $(TARGET) $(LDFLAGS)

tests/test_core.exe: tests/test_core.c src/internship.c src/hash_table.c \
                     src/stipend_bst.c src/application_queue.c src/search_sort.c
	$(CC) $(CFLAGS) -Isrc tests/test_core.c src/internship.c src/hash_table.c \
		src/stipend_bst.c src/application_queue.c src/search_sort.c -o $@

test: tests/test_core.exe
	./tests/test_core.exe

clean:
	rm -f $(TARGET) tests/test_core.exe
