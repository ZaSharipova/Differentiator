#ifndef READ_INFIX_EXPRESSION_H_
#define READ_INFIX_EXPRESSION_H_

#include "Enums.h"
#include "Structs.h"

DifNode_t *GetGoal(DifRoot *root, const char **string, VariableArr *Variable_Array, size_t *pos);

#endif //READ_INFIX_EXPRESSION_H_