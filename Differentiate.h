#ifndef DO_SOLVE_H_
#define DO_SOLVE_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *Dif(DifNode_t *root, DifNode_t *node, const char *main_var, FILE *texfile);
DifNode_t *NewNumber(double value);
DifNode_t *CopyNode(DifNode_t *node);
DifNode_t *NewOperationNode(OperationTypes op_type, DifNode_t *left, DifNode_t *right);
DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var);

#endif //DO_SOLVE_H_