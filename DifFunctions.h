#ifndef DIF_FUNCTIONS_H_
#define DIF_FUNCTIONS_H_

#include <stdio.h>
#include "Enums.h"
#include "Structs.h"

DifErrors NodeCtor(DifNode_t **node, Value *value);
DifErrors DifRootCtor(DifRoot *root);

double SolveEquation(DifRoot *root, VariableInfo *arr);

#endif //DIF_FUNCTIONS_H_