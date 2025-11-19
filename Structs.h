#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <stdio.h>

#include "Enums.h"
#define MAX_IMAGE_SIZE 60

typedef char* Dif_t;

struct Value {
    OperationTypes type;
    double number;
    char variable_name;
    int pos_of_variable;
};

struct FileInfo {
    char *buf_ptr;
    size_t filesize;
};

struct DifNode_t {
    DifTypes operation;
    struct Value value;
    DifNode_t *parent;
    DifNode_t *left;
    DifNode_t *right;
};

struct DifRoot {
    DifNode_t *root;
    size_t size;
};

typedef struct DumpInfo {
    DifRoot *tree;
    const char *filename_to_write_dump;
    FILE *file;
    const char *filename_to_write_graphviz;
    const char *filename_dump_made;
    char message[MAX_IMAGE_SIZE];
    char *name;
    char *question;
    char image_file[MAX_IMAGE_SIZE];
    size_t graph_counter;
    bool flag_new;

    enum DifErrors error;
} DumpInfo;

struct VariableInfo {
    char variable_name;
    double variable_value;
};

#endif //STRUCTS_H_