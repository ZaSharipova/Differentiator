
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Structs.h"
#include "Enums.h"
#include "DifFunctions.h"
#include "ReadExpression.h"
#include "DoDifferentiate.h"
#include "DoGraph.h"
#include "DoTex.h"
#include "DoDump.h"
#include "DoCalculate.h"

#include "Optimise.h"

#define MAX_VARIABLE_SIZE 30
#define FILE_OPEN_AND_CHECK(file, filename, mode) \
    FILE *file = fopen(filename, mode);           \
    if (!file) {                                  \
        perror("Error opening file");             \
        return kErrorOpening;                     \
    }

int main(void) {
    FILE_OPEN_AND_CHECK(file, "expression.txt", "r");
    DifRoot root = {};
    DifRootCtor(&root);
    FileInfo Info = {};

    DumpInfo DumpInfo = {};
    DumpInfo.filename_to_write_dump = "alldump.html";
    DumpInfo.file = fopen(DumpInfo.filename_to_write_dump, "w");
    DumpInfo.filename_to_write_graphviz = "output.txt";
    strcpy(DumpInfo.message, "Expression tree");

    size_t pos = 0;
    DifNode_t *new_node = NULL;
    DoBufRead(file, "expression.txt", &Info);

    FILE_OPEN_AND_CHECK(logfile, "logfile_for_expression.txt", "w");

    VariableInfo *Variable_Array = (VariableInfo *) calloc (MAX_VARIABLE_SIZE, sizeof(VariableInfo));
    if (!Variable_Array) {
        perror("ERROR calloc.\n");
        return kNoMemory;
    }

    int i = 0;
    ReadNodeFromFile(&root, file, logfile, &pos, root.root, Info.buf_ptr, &new_node, Variable_Array, &i);
    root.root = new_node;

    DumpInfo.tree = &root;
    DoTreeInGraphviz(root.root, &DumpInfo, root.root);
    DoDump(&DumpInfo);

    DifNode_t *new_tree = Dif(root.root, "x");
    DifRoot root2 = {};
    root2.root = new_tree;
    DumpInfo.tree = &root2;

    strcpy(DumpInfo.message, " Do derivative");
    DoTreeInGraphviz(root2.root, &DumpInfo, root2.root);
    DoDump(&DumpInfo);
    FILE_OPEN_AND_CHECK(out, "diftex.tex", "w");
    BeginTex(out);
    DoTex(root2.root, "x", out, false);

    OptimiseTree(root2.root, Variable_Array, out);
    DoTreeInGraphviz(root2.root, &DumpInfo, root2.root);
    DoDump(&DumpInfo);
    DoTex(root2.root, "x", out, true);

    ReadVariableValue(i, Variable_Array);
    double res = SolveEquation(&root, Variable_Array);
    printf("Результат вычисления выражения: %lf", res);
    free(Variable_Array);

    EndTex(out);
    fclose(out);
    fclose(file);
    fclose(logfile);

    TreeDtor(&root);
    TreeDtor(&root2);
}
