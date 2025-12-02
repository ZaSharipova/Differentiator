#ifndef READ_EXPRESSION_H_
#define READ_EXPRESSION_H_

#include "Enums.h"
#include "Structs.h"

#define CHECK_ERROR_RETURN(cond) \
    err = cond;                  \
    if (err != kSuccess) {       \
        return err;              \
    }

#define FILE_OPEN_AND_CHECK(file, filename, mode) \
    FILE *file = fopen(filename, mode);           \
    if (!file) {                                  \
        perror("Error opening file");             \
        return kErrorOpening;                     \
    }

#define INIT_DUMP_INFO(name)                                       \
    DumpInfo name = {};                                            \
    dump_info.filename_to_write_dump = "./data/alldump.html";      \
    dump_info.file = fopen(dump_info.filename_to_write_dump, "w"); \
    dump_info.filename_to_write_graphviz = "./data/output.txt";    \
    strcpy(dump_info.message, "Expression tree");

DifErrors ReadNodeFromFile(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *node, Dif_t buffer, DifNode_t **node_to_add, VariableArr *arr, int *i);
void ReadVariableValue(VariableArr *arr);
DifErrors ReadInfix(DifRoot *root, DumpInfo *dump_info, VariableArr *Variable_Array, const char *filename, FILE *texfile, char **new_string);

#endif //READ_EXPRESSION_H_