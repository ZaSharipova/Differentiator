#ifndef PLAY_MODE_H
#define PLAY_MODE_H

#include "Enums.h"
#include "Structs.h"

void PrintExpressionResultToFile(FILE *out, DifRoot *root, const char *main_var, double border1, double border2);

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *DumpInfo, char *string);

#endif //PLAY_MODE_H