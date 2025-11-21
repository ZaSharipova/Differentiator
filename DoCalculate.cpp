#include "DoCalculate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "DifFunctions.h"

static double FindVariableValue(VariableInfo *arr, char *var_name) {
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