#ifndef READ_EXPRESSION_H_
#define READ_EXPRESSION_H_

#include "Enums.h"
#include "Structs.h"

#define CHECK_ERROR_RETURN(cond) \
    err = cond;                  \
    if (err != kSuccess) {       \
        return err;              \
    }

void DoBufRead(FILE *file, const char *filename, FileInfo *Info);
DifErrors ReadNodeFromFile(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *node, Dif_t buffer, DifNode_t **node_to_add, VariableArr *arr, int *i);
void ReadVariableValue(VariableArr *arr);

#endif //READ_EXPRESSION_H_