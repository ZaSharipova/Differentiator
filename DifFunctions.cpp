#include "DifFunctions.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"

#include "ReadExpression.h"
#include "Calculate.h"
#include "DoTex.h"
#include "DoGraph.h"
#include "DoDump.h"
#include "Differentiate.h"
#include "Optimise.h"
#include "ReadInfixExpression.h"

size_t DEFAULT_SIZE = 1;

DifErrors NodeCtor(DifNode_t **node, Value *value) {
    assert(node);

    *node = (DifNode_t *) calloc (DEFAULT_SIZE, sizeof(DifNode_t));
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
        //node->left = NULL;
    }

    if (node->right) {
        DeleteNode(node->right);
        //node->right = NULL;
    }

    node->parent = NULL;

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

size_t CountSubTreeSize(DifNode_t *node) {
    if (node == NULL) {
        return 0;
    }

    return 1 + CountSubTreeSize(node->left) + CountSubTreeSize(node->right);
}

DifErrors InitArrOfVariable(VariableArr *arr, size_t capacity) {
    assert(arr);

    arr->capacity = capacity;
    arr->size = 0;

    arr->var_array = (VariableInfo *) calloc (capacity, sizeof(VariableInfo));
    if (!arr->var_array) {
        fprintf(stderr, "Memory error.\n");
        return kNoMemory;
    }

    return kSuccess;
}

DifErrors ResizeArray(VariableArr *arr)  {
    assert(arr);

    if (arr->size + 2 > arr->capacity) {
        VariableInfo *ptr = (VariableInfo *) calloc (arr->capacity += 2, sizeof(VariableInfo));
        if (!ptr) {
            fprintf(stderr, "Memory error.\n");
            return kNoMemory;
        }
        arr->var_array = ptr;
    }

    return kSuccess;
}

DifErrors DtorVariableArray(VariableArr *arr) {
    assert(arr);

    arr->capacity = 0;
    arr->size = 0;

    free(arr->var_array);

    return kSuccess;
}

DifErrors ForestCtor(Forest *forest, size_t size) {
    assert(forest);

    forest->size = size;

    forest->trees = (DifRoot *) calloc (size, sizeof(DifRoot));
    if (!forest->trees) {
        fprintf(stderr, "Memory error.\n");
        return kNoMemory;
    }

    return kSuccess;
}

void ForestDtor(Forest *forest) {
    assert(forest);

    forest->size = 0;

    free(forest->trees);
}