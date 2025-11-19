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
static const char *PrintExpressionType(const DifNode_t *node) {
    assert(node);

    switch (node->value.type) {
        case (kAdd):
            return "ADD";
        case (kSub):
            return "SUB";
        case (kMul):
            return "MUL";
        case (kDiv):
            return "DIV";
        case (kPow):
            return "POW";
        case (kSin):
            return "Sin";
        case (kCos):
            return "COS";
        case (kTg):
            return "TAN";
        default: return NULL;
    }
}

void PrintDotNode(FILE *file, const DifNode_t *node, const DifNode_t *node_colored, bool flag) {
    assert(file);
    assert(node);
    assert(node_colored);

    if (node->operation == kNumber) {
        fprintf(file, "    \"%p\" [label=\"Parent: %p \n  Addr: %p \n  Operation: %s\n", 
            (void *)node, (void *)node->parent, (void *)node, PrintOperationType(node));
        fprintf(file, "  Value: %lf  \nLeft: %p  Right: %p\" shape=egg color=black width=4 height=1.5 fixedsize=true];\n", 
                node->value.number, (void *)node->left, (void *)node->right);
    } else if (node->operation == kVariable) {
        fprintf(file, "    \"%p\" [label=\"Parent: %p \n  Addr: %p \n  Operation: %s\n", 
            (void *)node, (void *)node->parent, (void *)node, PrintOperationType(node));
        fprintf(file, "  Value: %s  \nLeft: %p  Right: %p\" shape=octagon color=black width=6 height=1.5 fixedsize=true];\n", 
                node->value.variable_name, (void *)node->left, (void *)node->right);
    } else if (node->operation == kOperation) {
        fprintf(file, "    \"%p\" [label=\"{Parent: %p \n | Addr: %p \n | Operation: %s\n", 
            (void *)node, (void *)node->parent, (void *)node, PrintOperationType(node));
        fprintf(file, " | Value: %s | {Left: %p | Right: %p}}\" shape=Mrecord color=black];\n", 
                PrintExpressionType(node), (void *)node->left, (void *)node->right);
    }

    if (node->left) {
        fprintf(file, "    \"%p\" -> \"%p\" [label=\"да\", fontsize=15, fontcolor=darkgreen, labeldistance=2.0, labelangle=45, color=darkolivegreen2];\n", 
                (void *)node, (void *)node->left);
        PrintDotNode(file, node->left, node_colored, flag);
    }

    if (node->right) {
        fprintf(file, "    \"%p\" -> \"%p\" [label=\"нет\", fontsize=15, fontcolor=darkred, labeldistance=2.0, labelangle=45, color=coral1];\n\n", 
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
