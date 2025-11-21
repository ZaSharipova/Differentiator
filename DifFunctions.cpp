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
    //assert(value);

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

double SolveEquation(DifRoot *root, VariableInfo *arr) {
    assert(root);
    assert(arr);

    return EvaluateExpression(root->root, arr);
}

static double FindVariableValue(VariableInfo *arr, char *var_name) {
    assert(arr);

    for (size_t i = 0; i < MAX_VARIABLES; i++) {
        if (strcmp(arr[i].variable_name, var_name) == 0) {
            return arr[i].variable_value;
        }
        if (strcmp(arr[i].variable_name, "\0")) {
            break;
        }
    }
    
    return 0;
}

double EvaluateExpression(DifNode_t *node, VariableInfo *arr) {
    assert(node);

    if (node->operation == kNumber) {
        return node->value.number;
    } else if (node->operation == kVariable) {
         return FindVariableValue(arr, node->value.variable_name);
    } else {
        switch (node->value.type) {
        case (kAdd):
            return EvaluateExpression(node->left, arr) +
                EvaluateExpression(node->right, arr);

        case (kSub):
            return EvaluateExpression(node->left, arr) -
                EvaluateExpression(node->right, arr);

        case (kMul):
            return EvaluateExpression(node->left, arr) *
                EvaluateExpression(node->right, arr);

        case (kDiv): {
            double right = EvaluateExpression(node->right, arr);
            if (right < 1e-12) {
                fprintf(stderr, "Division by zero.\n");
                return 0;
            }
            return EvaluateExpression(node->left, arr) / right;
        }

        case (kPow):
            return pow(EvaluateExpression(node->left, arr),
                    EvaluateExpression(node->right, arr));

        case (kSin):
            return sin(EvaluateExpression(node->right, arr));

        case (kCos):
            return cos(EvaluateExpression(node->right, arr));

        case (kTg):
            return tan(EvaluateExpression(node->right, arr));
        case (kLn):
            return log(EvaluateExpression(node->right, arr));
        case (kArctg):
            return atan(EvaluateExpression(node->right, arr));

        case (kNone):
        default:
            fprintf(stderr, "Unknown operation: %d.\n", node->value.type);
            return 0;
        }
    }
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