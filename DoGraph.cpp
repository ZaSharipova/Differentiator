#include "DoGraph.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "Enums.h"
#include "Structs.h"
#define FILE_OUT "output.txt"

static const char *PrintOperationType(const DifNode_t *node) {
    assert(node);

    switch(node->operation) {
    case (kNumber):
        return "NUM";
    case (kVariable):
        return "VAR";
    case (kOperation):
        return "OP";
    }
}

static GraphOperation PrintExpressionType(const DifNode_t *node) {
    assert(node);

    switch (node->value.type) {
        case (kAdd):
            return {"ADD", "plum"};
        case (kSub):
            return {"SUB", "orchid3"};
        case (kMul):
            return {"MUL", "salmon1"};
        case (kDiv):
            return {"DIV", "skyblue"};
        case (kPow):
            return {"POW", "darkseagreen3"};
        case (kSin):
            return {"Sin", "khaki3"};
        case (kCos):
            return {"COS", "cornsilk3"};
        case (kTg):
            return {"TAN", "tan"};
        case (kLn):
            return {"LOG", "cadetblue1"};
        case (kArctg):
            return {"ARCTG", "lightgoldenrod"};
        case (kNone):
        default: return {NULL, NULL};
    }
}

void PrintDotNode(FILE *file, const DifNode_t *node, const DifNode_t *node_colored, bool flag) {
    assert(file);
    assert(node);
    assert(node_colored);

    const char *color_number    = "dodgerblue";
    const char *color_variable  = "gold";

    if (node->operation == kNumber || node->operation == kVariable) {
        fprintf(file, "    \"%p\" [label=\"Parent: %p \n  Addr: %p \n  Operation: %s\n", 
            (void *)node, (void *)node->parent, (void *)node, PrintOperationType(node));
        if (node->operation == kNumber) {
            fprintf(file, "  Value: %lf  \nLeft: %p | Right: %p\" shape=egg color=black fillcolor=%s style=filled width=4 height=1.5 fixedsize=true];\n", 
                node->value.number, (void *)node->left, (void *)node->right, color_number);
        } else {
            fprintf(file, "  Value: %s  \nLeft: %p | Right: %p\" shape=octagon color=black fillcolor=%s style=filled width=4 height=1.5 fixedsize=true];\n", 
                (node->value).variable_name, (void *)node->left, (void *)node->right, color_variable);
        }
    } else if (node->operation == kOperation) {
        fprintf(file, "    \"%p\" [label=\"{Parent: %p \n | Addr: %p \n | Operation: %s\n", 
            (void *)node, (void *)node->parent, (void *)node, PrintOperationType(node));
        fprintf(file, " | Value: %s | {Left: %p | Right: %p}}\" shape=Mrecord color=black fillcolor=%s, style=filled];\n", 
                PrintExpressionType(node).operation_name, (void *)node->left, (void *)node->right, PrintExpressionType(node).color);
    }

    if (node->left) {
        fprintf(file, "    \"%p\" -> \"%p\";\n", 
                (void *)node, (void *)node->left);
        PrintDotNode(file, node->left, node_colored, flag);
    }

    if (node->right) {
        fprintf(file, "    \"%p\" -> \"%p\";\n\n", 
                (void *)node, (void *)node->right);
        PrintDotNode(file, node->right, node_colored, flag);
    }
}



static void DoSnprintf(DumpInfo *Info) {
    assert(Info);

    snprintf(Info->image_file, sizeof(Info->image_file), "Images/graph_%zu.svg", Info->graph_counter);
    (Info->graph_counter)++;
    char cmd[50] = {};
    snprintf(cmd, sizeof(cmd), "dot " FILE_OUT " -T svg -o %s", Info->image_file);
    
    system(cmd);
}

void DoTreeInGraphviz(const DifNode_t *node, DumpInfo *Info, const DifNode_t *node_colored) {
    assert(node);
    assert(Info);
    assert(node_colored);

    FILE *file = fopen(Info->filename_to_write_graphviz, "w");
    if (!file) {
        perror("Cannot open file for writing.");
        return;
    }

    fprintf(file, "digraph BinaryTree {\n");
    fprintf(file, "    rankdir=TB;\n");
    fprintf(file, "    node [shape=record, style=filled, fillcolor=lightblue];\n");
    fprintf(file, "    edge [fontsize=10];\n\n");
    fprintf(file, "    graph [fontname=\"Arial\"];\n");
    fprintf(file, "    node [fontname=\"Arial\"];\n");
    fprintf(file, "    edge [fontname=\"Arial\"];\n");

    if (node->parent == NULL) {
        fprintf(file, "    \"1\" [label=\"ROOT\", shape=rect, fillcolor=pink];\n");
        fprintf(file, "    \"1\" -> \"%p\";\n", (void *)node);
    } else {
        fprintf(file, "    // Empty tree\n");
    }

    PrintDotNode(file, node, node_colored, Info->flag_new);

    fprintf(file, "}\n");
    fclose(file);

    system("mkdir -p Images");
    DoSnprintf(Info);
}
