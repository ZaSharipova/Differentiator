#include <stdio.h>

#include "Structs.h"
#include "Enums.h"
#include "DifFunctions.h"
#include "ReadExpression.h"
#include "DoGraph.h"


int main(void) {
    FILE *file = fopen("expression.txt", "r");
    DifRoot root = {};
    DifRootCtor(&root);
    FileInfo Info = {};
    DumpInfo DumpInfo = {};

    size_t pos = 0;
    DifNode_t *new_node = NULL;
    DoBufRead(file, "expression.txt", &Info);
    FILE *logfile = fopen("logfile_for_expression.txt", "w");

    VariableInfo VARIABLES_ARRAY[30] = {};
    int i = 0;

    ReadNodeFromFile(&root, file, logfile, &pos, (root.root), Info.buf_ptr, &new_node, VARIABLES_ARRAY, &i);
    root.root = new_node;
    ReadVariableValues(i, VARIABLES_ARRAY);

    double res = SolveEquation(&root, VARIABLES_ARRAY);
    printf("%lf", res);

    fclose(file);
    fclose(logfile);

    DumpInfo.filename_to_write_graphviz = "output.txt";
    DoTreeInGraphviz(root.root, &DumpInfo, root.root);
}