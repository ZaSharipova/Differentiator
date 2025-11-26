#ifndef DO_TEX_H_
#define DO_TEX_H_

#include "Enums.h"
#include "Structs.h"

void BeginTex(FILE *out, DifNode_t *node);
void EndTex(FILE *out);
void DoTexInner(DifNode_t *node, FILE *out);
void DoTexStep(DifNode_t *root, DifNode_t *current, const char *var, FILE *out);

void DoTex(DifNode_t *node, const char *value, FILE *out);
void PrintSolution(DifNode_t *node, double answer, FILE *out, VariableArr *VariableArr);
#endif //DO_TEX_H_