#include "Calculate.h"

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

double SolveEquation(DifRoot *root) {
    assert(root);

    return EvaluateExpression(root->root);
}

double EvaluateExpression(DifNode_t *node) {
    assert(node);

    if (node->type == kNumber) {
        return node->value.number;
    }
    if (node->type == kVariable) {
        return node->value.variable->variable_value;
        //return FindVariableValue(arr, node->value.variable->variable_name);
    }

    switch (node->value.operation) {
    case (kOperationAdd):
        return EvaluateExpression(node->left) +
            EvaluateExpression(node->right);

    case (kOperationSub):
        return EvaluateExpression(node->left) -
            EvaluateExpression(node->right);

    case (kOperationMul):
        return EvaluateExpression(node->left) *
            EvaluateExpression(node->right);

    case (kOperationDiv): {
        double right = EvaluateExpression(node->right);
        if (right < 1e-12) {
            fprintf(stderr, "Division by zero.\n");
            return 0;
        }
        return EvaluateExpression(node->left) / right;
    }

    case (kOperationPow):
        return pow(EvaluateExpression(node->left),
                EvaluateExpression(node->right));

    case (kOperationSin):
        return sin(EvaluateExpression(node->right));

    case (kOperationCos):
        return cos(EvaluateExpression(node->right));

    case (kOperationTg):
        return tan(EvaluateExpression(node->right));
    case (kOperationLn):
        return log(EvaluateExpression(node->right));
    case (kOperationArctg):
        return atan(EvaluateExpression(node->right));
    case (kOperationSinh):
        return sinh(EvaluateExpression(node->right));    
    case (kOperationCosh):
        return cosh(EvaluateExpression(node->right));
    case (kOperationTgh):
        return tanh(EvaluateExpression(node->right));

    case (kOperationNone):
    default:
        fprintf(stderr, "Unknown operation: %d.\n", node->value.operation);
        return 0;
    }
}