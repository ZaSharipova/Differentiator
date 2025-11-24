#ifndef DO_SOLVE_H_
#define DO_SOLVE_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *Dif(DifNode_t *root, DifNode_t *node, const char *main_var, FILE *texfile);
DifNode_t *NewNumber(double value);
DifNode_t *CopyNode(DifNode_t *node);

#endif //DO_SOLVE_H_