#ifndef READ_EXPRESSION_H_
#define READ_EXPRESSION_H_

#include "Enums.h"
#include "Structs.h"

void DoBufRead(FILE *file, const char *filename, FileInfo *Info);
DifErrors ReadNodeFromFile(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *node, Dif_t buffer, DifNode_t **node_to_add, VariableInfo *arr, int *i);
void ReadVariableValues(int size, VariableInfo *arr);

#endif //READ_EXPRESSION_H_