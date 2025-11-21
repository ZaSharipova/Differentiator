#include "Optimise.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "DoDifferentiate.h"

#include "DoGraph.h"
#include "DoDump.h"
#include "DoTex.h"

#define eps 1e-11

static DifNode_t *AddOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *SubOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *MulOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *DivOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *PowOptimise(DifNode_t *node, bool *has_change);

static bool IsZero(DifNode_t *node);
static bool IsOne(DifNode_t *node);
static bool IsNumber(DifNode_t *node);

#define NEWN(number) NewNumber(number)

DifErrors OptimiseTree(DifNode_t *node, VariableInfo *arr, FILE *out) {
    assert(node);
    assert(arr);
    assert(out);

    bool has_change = false;

    while (true) {
        has_change = false;
        node = ConstOptimise(node, arr, &has_change); 
        DoTex(node, "x", out, false); // NULL
        node = EraseNeutralElements(node, arr, &has_change);
        if (has_change == false) {
            break;
        }
    }
    return kSuccess;
}

DifNode_t *ConstOptimise(DifNode_t *node, VariableInfo *arr, bool *has_change) {
    assert(node);
    assert(arr);
    assert(has_change);

    if (node->right) {
        if (!IsNumber(node->right)) {
            node->right = ConstOptimise(node->right, arr, has_change);
            if (!node->right) return NULL;
        }
    }

    if (node->left) {
        if (!IsNumber(node->left)) {
            node->left = ConstOptimise(node->left, arr, has_change);
            if (!node->left) return NULL;
        }
    }

    if (node->left && node->right && IsNumber(node->left) && IsNumber(node->right)) {
        double ans = EvaluateExpression(node, NULL);
        DeleteNode(node->left);
        DeleteNode(node->right);
        node->left = node->right = NULL;

        node->operation = kNumber;
        node->value.number = ans;

        *has_change = true;
        return node;
    }

    if (!node->left && node->right && IsNumber(node->right)) {
        double ans = EvaluateExpression(node, arr);
        DeleteNode(node->right);
        node->right = NULL;

        node->operation = kNumber;
        node->value.number = ans;

        *has_change = true;
        return node;
    }

    return node;
}

DifNode_t *EraseNeutralElements(DifNode_t *node, VariableInfo *arr, bool *has_change) {
    assert(node);
    assert(arr);
    assert(has_change);

    if (node->left) {
        node->left = EraseNeutralElements(node->left, arr, has_change);
    }

    if (node->right) {
        node->right = EraseNeutralElements(node->right, arr, has_change);
    }

    if ((!node->left || !node->right) || !(node->operation == kOperation)) {
        return node;
    }

    OperationTypes operation = node->value.type;

    if (operation == kAdd) {
        return AddOptimise(node, has_change);
    } else if (operation == kSub) {
        return SubOptimise(node, has_change);
    } else if (operation == kMul) {
        return MulOptimise(node, has_change);
    } else if (operation == kDiv) {
        return DivOptimise(node, has_change);
    } else if (operation == kPow) {
        return PowOptimise(node, has_change);
    }

    return node;
}


static DifNode_t *AddOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *copied_node = NULL;

    if (IsZero(node->left)) {
        copied_node = CopyNode(node->right);
        *has_change = true;
        DeleteNode(node);
        return copied_node;
    }

    if (IsZero(node->right)) {
        copied_node = CopyNode(node->left);
        *has_change = true;
        DeleteNode(node);
        return copied_node;
    }

    return node;
}

static DifNode_t *SubOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (IsNumber(node->right) && IsZero(node->left)) {
        DifNode_t *copied_node = CopyNode(node->left);
        DeleteNode(node);
        *has_change = true;
        return copied_node;
    }

    return node;
}

static DifNode_t *MulOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *new_node = NULL;

    if (IsOne(node->left)) {
        new_node = CopyNode(node->right);
        DeleteNode(node);
        return new_node;
    }

    if (IsOne(node->right)) {
        new_node = CopyNode(node->left);
        DeleteNode(node);
        return new_node;
    }

    if ((IsZero(node->left)) || IsZero(node->right)) {
        DeleteNode(node);
        return NEWN(0);
    }

    return node;
}

static DifNode_t *DivOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *new_node = NULL;

    if (IsOne(node->right)) {
        new_node = CopyNode(node->right);
        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    if (IsZero(node->left)) {
        DeleteNode(node);
        *has_change = true;
        return NEWN(0);
    }

    return node;
}

static DifNode_t *PowOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (IsZero(node->left)) {
        DeleteNode(node);
        *has_change = true;
        return NEWN(0);
    }

    if (IsZero(node->right)) {
        DeleteNode(node);
        *has_change = true;
        return NEWN(1);
    }

    if (IsOne(node->right)) {
        DifNode_t *new_node = CopyNode(node->left);
        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    return node;
}

#undef NEWN

static bool IsZero(DifNode_t *node) {
    assert(node);

    return (node->operation == kNumber && fabs(node->value.number) < eps);
}

static bool IsOne(DifNode_t *node) {
    assert(node);
    
    return (node->operation == kNumber && fabs(node->value.number - 1) < eps);
}

static bool IsNumber(DifNode_t *node) {
    assert(node);

    return (node->operation == kOperation);
}