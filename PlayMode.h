#ifndef PLAY_MODE_H
#define PLAY_MODE_H

#include "Enums.h"
#include "Structs.h"

void PrintExpressionResultToFile(FILE *out, DifRoot *root, const char *main_var);

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *DumpInfo);

#endif //PLAY_MODE_H