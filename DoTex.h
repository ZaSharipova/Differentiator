#ifndef DO_TEX_H_
#define DO_TEX_H_

#include "Enums.h"
#include "Structs.h"

void BeginTex(FILE *out);
void EndTex(FILE *out);
void DoTex(DifNode_t *node, const char *value, FILE *out, bool is_last);

#endif //DO_TEX_H_