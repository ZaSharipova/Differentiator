#include "DoCalculate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "DifFunctions.h"

static double FindVariableValue(VariableInfo *arr, const char *var_name) {
    assert(arr);
    assert(var_name);

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

double SolveEquation(DifRoot *root, VariableInfo *arr) {
    assert(root);
    assert(arr);

    return EvaluateExpression(root->root, arr);
}

double EvaluateExpression(DifNode_t *node, VariableInfo *arr) {
    assert(node);
    assert(arr);

    if (node->type == kNumber) {
        return node->value.number;
    }
    if (node->type == kVariable) {
        return node->value.variable->variable_value;
        //return FindVariableValue(arr, node->value.variable->variable_name);
    }

    switch (node->value.operation) {
    case (kOperationAdd):
        return EvaluateExpression(node->left, arr) +
            EvaluateExpression(node->right, arr);

    case (kOperationSub):
        return EvaluateExpression(node->left, arr) -
            EvaluateExpression(node->right, arr);

    case (kOperationMul):
        return EvaluateExpression(node->left, arr) *
            EvaluateExpression(node->right, arr);

    case (kOperationDiv): {
        double right = EvaluateExpression(node->right, arr);
        if (right < 1e-12) {
            fprintf(stderr, "Division by zero.\n");
            return 0;
        }
        return EvaluateExpression(node->left, arr) / right;
    }

    case (kOperationPow):
        return pow(EvaluateExpression(node->left, arr),
                EvaluateExpression(node->right, arr));

    case (kOperationSin):
        return sin(EvaluateExpression(node->right, arr));

    case (kOperationCos):
        return cos(EvaluateExpression(node->right, arr));

    case (kOperationTg):
        return tan(EvaluateExpression(node->right, arr));
    case (kOperationLn):
        return log(EvaluateExpression(node->right, arr));
    case (kOperationArctg):
        return atan(EvaluateExpression(node->right, arr));

    case (kOperationNone):
    default:
        fprintf(stderr, "Unknown operation: %d.\n", node->value.operation);
        return 0;
    }
}