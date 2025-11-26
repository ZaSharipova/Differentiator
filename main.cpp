
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Structs.h"
#include "Enums.h"
#include "DifFunctions.h"
#include "ReadExpression.h"
#include "Differentiate.h"
#include "DoGraph.h"
#include "DoTex.h"
#include "DoDump.h"
#include "Calculate.h"

#include "Optimise.h"

#define MAX_VARIABLE_SIZE 30

#define INIT_DUMP_INFO(name)                                     \
    DumpInfo DumpInfo = {};                                      \
    DumpInfo.filename_to_write_dump = "alldump.html";            \
    DumpInfo.file = fopen(DumpInfo.filename_to_write_dump, "w"); \
    DumpInfo.filename_to_write_graphviz = "output.txt";          \
    strcpy(DumpInfo.message, "Expression tree");

int main(void) {
    FILE_OPEN_AND_CHECK(file, "expression.txt", "r");
    DifRoot root = {};
    DifRootCtor(&root);

    FileInfo Info = {};
    DoBufRead(file, "expression.txt", &Info);

    VariableArr Variable_Array = {};
    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(InitArrOfVariable(&Variable_Array, 4));

    size_t pos = 0;
    DifNode_t *new_node = NULL;
    int i = 0;
    FILE_OPEN_AND_CHECK(logfile, "logfile_for_expression.txt", "w");
    ReadNodeFromFile(&root, file, logfile, &pos, root.root, Info.buf_ptr, &new_node, &Variable_Array, &i);
    root.root = new_node;

    INIT_DUMP_INFO(DumpInfo);
    DumpInfo.tree = &root;
    DoTreeInGraphviz(root.root, &DumpInfo, root.root);
    DoDump(&DumpInfo);

    FILE_OPEN_AND_CHECK(out, "diftex.tex", "w");
    BeginTex(out, root.root);

    CHECK_ERROR_RETURN(DiffPlay(&Variable_Array, &root, out, &DumpInfo));
    EndTex(out);
    fclose(out);
    fclose(file);
    fclose(logfile);

    DtorVariableArray(&Variable_Array);
    TreeDtor(&root);
}
