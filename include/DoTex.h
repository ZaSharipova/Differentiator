#ifndef DO_TEX_H_
#define DO_TEX_H_

#include "Enums.h"
#include "Structs.h"

void BeginTex(FILE *out);
void EndTex(FILE *out);
void DoTexInner(DifNode_t *node, FILE *out);

void DoTex(DifNode_t *node, const char *value, FILE *out);
void PrintSolution(DifNode_t *node, double answer, FILE *out, VariableArr *VariableArr);
void PrintShrich(DifNode_t *node, DifNode_t *result, FILE *out);
void UploadGraph(FILE *out);
void PrintTaylor(DifNode_t *node, const char *main_var, double number, size_t num_pos, FILE *out);
void PrintFirstExpression(FILE *out, DifNode_t *node);
void PrintSolutionForDerivative(DifNode_t *node, size_t num_of_der, double answer, FILE *out, VariableArr *Variable_Array);
void PrintColoredNaming(FILE *out, DifNode_t *node, const char *color, const char *color_rus);
void PrintAllResults(Forest *forest, FILE *out, double taylor_pos, const char *main_var);

#endif //DO_TEX_H_