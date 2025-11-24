#ifndef DIF_FUNCTIONS_H_
#define DIF_FUNCTIONS_H_

#include <stdio.h>
#include "Enums.h"
#include "Structs.h"

DifErrors NodeCtor(DifNode_t **node, Value *value);
DifErrors DifRootCtor(DifRoot *root);

double SolveEquation(DifRoot *root, VariableInfo *arr);
double EvaluateExpression(DifNode_t *node, VariableInfo *arr);

DifErrors TreeDtor(DifRoot *tree);
DifErrors DeleteNode(DifNode_t *node);

DifErrors InitArrOfVariable(VariableArr *arr, size_t capacity);
DifErrors ResizeArray(VariableArr *arr);
DifErrors DtorVariableArray(VariableArr *arr);

#endif //DIF_FUNCTIONS_H_