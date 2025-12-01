#ifndef DO_SOLVE_H_
#define DO_SOLVE_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *Dif(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile, VariableArr *Variable_Array);

DifNode_t *CopyNode(DifRoot *root, DifNode_t *node);
DifNode_t *NewNode(DifRoot *root, DifTypes type, Value value, DifNode_t *left, DifNode_t *right, VariableArr *Variable_Array);
DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var);
DifNode_t *NewVariable(DifRoot *root, const char *variable, VariableArr *VariableArr);
#endif //DO_SOLVE_H_