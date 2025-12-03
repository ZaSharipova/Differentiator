
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Structs.h"
#include "Enums.h"

#include "DifFunctions.h"
#include "ReadExpression.h"
#include "DoTex.h"
#include "PlayMode.h"

#define MAX_VARIABLE_SIZE 30

int main(void) {
    DifRoot root = {};
    DifRootCtor(&root);

    VariableArr Variable_Array = {};
    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(InitArrOfVariable(&Variable_Array, 4));

    FILE_OPEN_AND_CHECK(out, "diftex.tex", "w");
    BeginTex(out);

    INIT_DUMP_INFO(dump_info);
    dump_info.tree = &root;

    char *string = NULL;
    CHECK_ERROR_RETURN(ReadInfix(&root, &dump_info, &Variable_Array, "./data/input.txt", out, &string));

    CHECK_ERROR_RETURN(DiffPlay(&Variable_Array, &root, out, &dump_info, string));
    EndTex(out);
    fclose(out);

    DtorVariableArray(&Variable_Array);
    return kSuccess;
    //TreeDtor(&root);
}
