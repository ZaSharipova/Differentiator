#include "DoDump.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "Enums.h"
#include "Structs.h"
#include "DoGraph.h"

#define MAX_STRING_SIZE 60
#define NUMBER_OF_ERRORS (sizeof(TREE_ERROR_MESSAGES) / sizeof(TREE_ERROR_MESSAGES[0]))

static void PrintCurrentTime(FILE *file, const char *label);

void DoDump(DumpInfo *Info) {
    assert(Info);

    if (!Info->file) {
        fprintf(stderr, "Failed to open file %s.\n", Info->filename_to_write_dump);
    }
    FILE *file = Info->file;

    fprintf(file, "<h2> DUMP %s</h2>", Info->message);

    PrintCurrentTime(file, "made");
    fprintf(file, "<h3 style=\"margin: 2px 0;\"> tree root address: %p</h3>", Info->tree->root);
    //fprintf(file, "<h3 style=\"margin: 2px 0;\"> tree size: %zu</h3>", Info->tree->size);
    fprintf(file, "<img src = %s width=1500px>", Info->image_file);

    fprintf(file, "<hr style=\"border: none; height: 2px; background-color: black;\">");
}

static void PrintCurrentTime(FILE *file, const char *label) {
    assert(file);
    assert(label);

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    char buf_time[MAX_STRING_SIZE] = {};
    strftime(buf_time, sizeof(buf_time), "%Y-%m-%d %H:%M:%S\n", tm_now);

    fprintf(file, "<h4 style=\"margin: 2px 0;\">%s: %s</h4>\n", label, buf_time);
    fprintf(file, "<br>");
}