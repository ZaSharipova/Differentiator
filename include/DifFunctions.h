#ifndef DIF_FUNCTIONS_H_
#define DIF_FUNCTIONS_H_

#include <stdio.h>
#include "Enums.h"
#include "Structs.h"

DifErrors NodeCtor(DifNode_t **node, Value *value);
DifErrors DifRootCtor(DifRoot *root);

DifErrors TreeDtor(DifRoot *tree);
DifErrors DeleteNode(DifRoot *root, DifNode_t *node);

DifErrors InitArrOfVariable(VariableArr *arr, size_t capacity);
DifErrors ResizeArray(VariableArr *arr);
DifErrors DtorVariableArray(VariableArr *arr);

DifErrors ForestCtor(Forest *forest, size_t size);
DifErrors ResizeForest(Forest *forest, size_t new_size);
void ForestDtor(Forest *forest);

size_t CountSubTreeSize(DifNode_t *node);
DifErrors VerifyTree(DifRoot *root);

#endif //DIF_FUNCTIONS_H_