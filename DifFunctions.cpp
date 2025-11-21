#include "DifFunctions.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"

double EvaluateExpression(DifNode_t *node, VariableInfo *arr);

DifErrors NodeCtor(DifNode_t **node, Value *value) {
    assert(node);

    *node = (DifNode_t *) calloc(1, sizeof(DifNode_t));
    if (!*node) {
        fprintf(stderr, "No memory to calloc NODE.\n");
        return kNoMemory;
    }

    if (value) {
        (*node)->value = *value; 
    } 
    
    (*node)->left =  NULL;
    (*node)->right =  NULL;
    (*node)->parent = NULL;

    return kSuccess;
}

DifErrors DifRootCtor(DifRoot *root) {
    assert(root);

    root->root = NULL;
    root->size = 0;

    return kSuccess;
}

DifErrors DeleteNode(DifNode_t *node) {
    if (!node)
        return kSuccess;

    if (node->left) {
        DeleteNode(node->left);
        node->left = NULL;
    }

    if (node->right) {
        DeleteNode(node->right);
        node->right = NULL;
    }

    // if (&node->value)
    //     free(&node->value);
    free(node);

    return kSuccess;
}

DifErrors TreeDtor(DifRoot *tree) {
    assert(tree);

    DeleteNode(tree->root);

    tree->root =  NULL;
    tree->size = 0;

    return kSuccess;
}