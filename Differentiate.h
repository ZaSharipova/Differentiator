#ifndef DO_SOLVE_H_
#define DO_SOLVE_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *Dif(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile);
DifNode_t *NewNumber(DifRoot *root, double value);

DifNode_t *CopyNode(DifRoot *root, DifNode_t *node);
DifNode_t *NewOperationNode(DifRoot *root, OperationTypes op_type, DifNode_t *left, DifNode_t *right);
DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var);

#endif //DO_SOLVE_H_