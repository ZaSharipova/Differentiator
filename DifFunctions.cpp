#include "DifFunctions.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"

static DifErrors SubstituteVariables(DifNode_t *node, VariableInfo *arr);
static double EvaluateExpression(DifNode_t *node, VariableInfo *vars);

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
    } else {
        memset(&(*node)->value, 0, sizeof(Value));
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

double SolveEquation(DifRoot *root, VariableInfo *vars) {
    assert(root);
    assert(vars);

    return EvaluateExpression(root->root, vars);
}

static double FindVariableValue(VariableInfo *arr, char var_name) {
    assert(arr);

    for (size_t i = 0; i < MAX_VARIABLES; i++) {
        if (arr[i].variable_name == var_name) {
            return arr[i].variable_value;
        }
        if (arr[i].variable_name == '\0') {
            break;
        }
    }
    
    return VARIABLE_NOT_FOUND;
}

static DifErrors SubstituteVariables(DifNode_t *node, VariableInfo *arr) {
    assert(arr);

    if (!node) {
        return kSuccess;
    }

    if (node->value.type == kVariablek) {
        char var_name = node->value.variable_name;
        double value = FindVariableValue(arr, var_name);
        
        if (value == VARIABLE_NOT_FOUND) {
            fprintf(stderr, "Error: Variable '%c' not found in array.\n", var_name);
            return kSyntaxError;
        }

        node->value.type = kNumberk;
        node->value.number = value;
    }


    DifErrors err = kSuccess;
    
    if (node->left) {
        err = SubstituteVariables(node->left, arr);
        if (err != kSuccess) return err;
    }
    
    if (node->right) {
        err = SubstituteVariables(node->right, arr);
        if (err != kSuccess) return err;
    }

    return kSuccess;
}

static double EvaluateExpression(DifNode_t *node, VariableInfo *vars) {
    assert(node);
    assert(vars);

    switch (node->value.type) {
        case kNumberk:
            return node->value.number;

        case kVariablek:
            return FindVariableValue(vars, node->value.variable_name);

        case kAdd:
            return EvaluateExpression(node->left, vars) +
                   EvaluateExpression(node->right, vars);

        case kSub:
            return EvaluateExpression(node->left, vars) -
                   EvaluateExpression(node->right, vars);

        case kMul:
            return EvaluateExpression(node->left, vars) *
                   EvaluateExpression(node->right, vars);

        case kDiv: {
            double right = EvaluateExpression(node->right, vars);
            if (right < 1e-12) {
                fprintf(stderr, "Division by zero.\n");
                return 0;
            }
            return EvaluateExpression(node->left, vars) / right;
        }

        case kPow:
            return pow(EvaluateExpression(node->left, vars),
                       EvaluateExpression(node->right, vars));

        case kSin:
            return sin(EvaluateExpression(node->right, vars));

        case kCos:
            return cos(EvaluateExpression(node->right, vars));

        case kTg:
            return tan(EvaluateExpression(node->right, vars));

        default:
            fprintf(stderr, "Unknown operation: %d.\n", node->value.type);
            return 0;
    }
}