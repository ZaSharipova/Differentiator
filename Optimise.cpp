#include "Optimise.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "DoSolve.h"

#include "DoGraph.h"
#include "DoDump.h"
#include "DoTex.h"


#define eps 1e-11

static DifNode_t *AddOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *SubOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *MulOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *DivOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *PowOptimise(DifNode_t *node, bool *has_change);

#define NEWN(number) NewNumber(number)

DifErrors OptimiseTree(DifNode_t *node, VariableInfo *arr, FILE *out) {
    assert(node);
    assert(arr);

    bool has_change = false;

    while (true) {
        has_change = false;
        node = ConstOptimise(node, arr, &has_change); 
        DoTex(node, "x", out, false);// NULL
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
        if (node->right->operation != kNumber) {
            node->right = ConstOptimise(node->right, arr, has_change);
            if (!node->right) return NULL;
        }
    }

    if (node->left) {
        if (node->left->operation != kNumber) {
            node->left = ConstOptimise(node->left, arr, has_change);
            if (!node->left) return NULL;
        }
    }

    if (node->left && node->right &&
        node->left->operation == kNumber && node->right->operation == kNumber) {
        double ans = EvaluateExpression(node, NULL);
        DeleteNode(node->left);
        DeleteNode(node->right);
        node->left = node->right = NULL;

        node->operation = kNumber;
        node->value.number = ans;

        *has_change = true;
        return node;
    }

    if (!node->left && node->right && node->right->operation == kNumber) {
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

    if (node->left->operation == kNumber && fabs((*(node->left)).value.number) < eps) {
        copied_node = CopyNode(node->right);
        *has_change = true;
        DeleteNode(node);
        return copied_node;
    }

    if (node->right->operation == kNumber && fabs((*(node->right)).value.number) < eps) {
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

    if (node->left->operation == kNumber && node->right->operation == kNumber && fabs((*(node->left)).value.number) < eps) {
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

    if (node->left->operation == kNumber && fabs((*(node->left)).value.number - 1) < eps) {
        new_node = CopyNode(node->right);

        DeleteNode(node);
        return new_node;
    }

    if (node->right->operation == kNumber && fabs((*(node->right)).value.number - 1) < eps) {
        new_node = CopyNode(node->left);

        DeleteNode(node);
        return new_node;
    }

    if ((node->left->operation == kNumber && fabs((*(node->left)).value.number) < eps) || 
    (node->left->operation == kNumber && fabs((*(node->left)).value.number) < eps)) {
        new_node = NEWN(0);

        DeleteNode(node);
        return new_node;
    }

    return node;
}

static DifNode_t *DivOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *new_node = NULL;

    if (node->right->operation == kNumber && fabs((*(node->right)).value.number - 1) < eps) {
        new_node = CopyNode(node->right);

        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    if (node->left->operation == kNumber && fabs((*(node->left)).value.number) < eps) {
        new_node = NEWN(0);

        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    return node;
}

static DifNode_t *PowOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (node->left->operation == kNumber && fabs((*(node->left)).value.number) < eps) {
        DeleteNode(node);
        *has_change = true;
        return NEWN(0);
    }

    if (node->right->operation == kNumber && fabs((*(node->right)).value.number) < eps) {
        DeleteNode(node);
        *has_change = true;
        return NEWN(1);
    }

    if (node->right->operation == kNumber && fabs((*(node->right)).value.number - 1) < eps) {
        DifNode_t *new_node = CopyNode(node->left);
        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    return node;
}

#undef NEWN