#ifndef DO_CALCULATE_H_
#define DO_CALCULATE_H_

#include "Enums.h"
#include "Structs.h"

double SolveEquation(DifRoot *root, VariableInfo *arr);
double EvaluateExpression(DifNode_t *node, VariableInfo *arr);

#endif //DO_CALCULATE_H_