#ifndef OPTIMISE_H_
#define OPTIMISE_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *OptimiseTree(DifNode_t *node, FILE *out);
DifNode_t *ConstOptimise(DifNode_t *node,  bool *has_change);
DifNode_t *EraseNeutralElements(DifNode_t *node, bool *has_change);

#endif //OPTIMISE_H_