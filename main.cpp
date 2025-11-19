#include <stdio.h>
#include <stdlib.h>

#include "Structs.h"
#include "Enums.h"
#include "DifFunctions.h"
#include "ReadExpression.h"
#include "DoSolve.h"
#include "DoGraph.h"
#include "DoTex.h"


int main(void) {
    FILE *file = fopen("expression.txt", "r");
    DifRoot root = {};
    DifRootCtor(&root);
    FileInfo Info = {};
    DumpInfo DumpInfo = {};
    DumpInfo.filename_to_write_graphviz = "output.txt";

    size_t pos = 0;
    DifNode_t *new_node = NULL;
    DoBufRead(file, "expression.txt", &Info);
    FILE *logfile = fopen("logfile_for_expression.txt", "w");

    VariableInfo *Variable_Array = (VariableInfo *) calloc (30, sizeof(VariableInfo));
    int i = 0;

    ReadNodeFromFile(&root, file, logfile, &pos, (root.root), Info.buf_ptr, &new_node, Variable_Array, &i);
    root.root = new_node;

    if (!root.root) {
        printf("AAAAAA");
    }
    DoTreeInGraphviz(root.root, &DumpInfo, root.root);
    DifNode_t *new_tree = Dif(root.root, "x");
    DifRoot root2 = {};
    root2.root = new_tree;
    DoTreeInGraphviz(root2.root, &DumpInfo, root2.root);
    DoTex(root2.root, "x");

    ReadVariableValue(i, Variable_Array);
    
    double res = SolveEquation(&root, Variable_Array);
    printf("%lf", res);
    free(Variable_Array);

    fclose(file);
    fclose(logfile);

    TreeDtor(&root);
    TreeDtor(&root2);
}