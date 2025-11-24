#include "Optimise.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "Differentiate.h"
#include "Calculate.h"

#include "DoGraph.h"
#include "DoDump.h"
#include "DoTex.h"

double eps = 1e-11;

static DifNode_t *AddOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *SubOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *MulOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *DivOptimise(DifNode_t *node, bool *has_change);
static DifNode_t *PowOptimise(DifNode_t *node, bool *has_change);

static DifNode_t *CheckNodeAndConstOptimise(DifNode_t *node, 
    bool *has_change);

static bool IsZero(DifNode_t *node);
static bool IsOne(DifNode_t *node);
static bool IsNumber(DifNode_t *node);
static bool IsOperation(DifNode_t *node);

#define NEWN(number) NewNumber(number)

DifNode_t *OptimiseTree(DifNode_t *node, FILE *out) {
    assert(node);
    assert(out);

    bool has_change = true;

    while (has_change) {
        has_change = false;
        node = ConstOptimise(node, &has_change); 
        if (has_change) DoTex(node, "x", out, false); // NULL
        node = EraseNeutralElements(node, &has_change); //
        if (has_change) DoTex(node, "x", out, false); // NULL
        // if (!has_change) {
        //     break;
        // }
    }
    return node;
}

DifNode_t *ConstOptimise(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    CheckNodeAndConstOptimise(node->right, has_change);
    CheckNodeAndConstOptimise(node->left, has_change);

    if (node->left && node->right && IsNumber(node->left) && IsNumber(node->right)) {
        double ans = EvaluateExpression(node);
        DeleteNode(node->left);
        DeleteNode(node->right);
        node->left = node->right = NULL;

        node->type = kNumber;
        node->value.number = ans;

        *has_change = true;
        return node;
    }

    if (!node->left && IsNumber(node->right)) {
        double ans = EvaluateExpression(node);
        DeleteNode(node->right);
        node->right = NULL;

        node->type = kNumber;
        node->value.number = ans;

        *has_change = true;
        return node;
    }

    return node;
}

DifNode_t *EraseNeutralElements(DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (node->left) {
        node->left = EraseNeutralElements(node->left, has_change);
    }

    if (node->right) {
        node->right = EraseNeutralElements(node->right, has_change);
    }

    if ((!node->left || !node->right) || !IsOperation(node)) {
        return node;
    }

    OperationTypes operation = node->value.operation;

    if (operation == kOperationAdd) {
        return AddOptimise(node, has_change);
    }
    if (operation == kOperationSub) {
        return SubOptimise(node, has_change);
    } 
    if (operation == kOperationMul) {
        return MulOptimise(node, has_change);
    }
    if (operation == kOperationDiv) {
        return DivOptimise(node, has_change);
    }
    if (operation == kOperationPow) {
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
        node->left = node->right = NULL;
        *has_change = true;
        DeleteNode(node);
        return copied_node;
    }

    if (IsZero(node->right)) {
        copied_node = CopyNode(node->left);
        node->left = node->right = NULL;
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
        node->left = node->right = NULL;
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
        node->left = node->right = NULL;
        DeleteNode(node);
        return new_node;
    }

    if (IsOne(node->right)) {
        new_node = CopyNode(node->left);
        node->left = node->right = NULL;
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
        node->left = node->right = NULL;
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
        node->left = node->right = NULL;
        DeleteNode(node);
        *has_change = true;
        return NEWN(0);
    }

    if (IsZero(node->right)) {
        node->left = node->right = NULL;
        DeleteNode(node);
        *has_change = true;
        return NEWN(1);
    }

    if (IsOne(node->right)) {
        DifNode_t *new_node = CopyNode(node->left);
        node->left = node->right = NULL;
        DeleteNode(node);
        *has_change = true;
        return new_node;
    }

    return node;
}

#undef NEWN

static bool IsZero(DifNode_t *node) {
    if (!node) {
        return false;
    }

    return (node->type == kNumber && fabs(node->value.number) < eps);
}

static bool IsOne(DifNode_t *node) {
    if (!node) {
        return false;
    }

    return (node->type == kNumber && fabs(node->value.number - 1) < eps);
}

static bool IsNumber(DifNode_t *node) {
    if (!node) {
        return false;
    }

    return (node->type == kNumber);
}

static bool IsOperation(DifNode_t *node) {
    if (!node) {
        return false;
    }

    return (node->type == kOperation);
}

static DifNode_t *CheckNodeAndConstOptimise(DifNode_t *node, 
    bool *has_change) {
    assert(has_change);

    if (node && !IsNumber(node)) {
        node = ConstOptimise(node, has_change);
        if (!node) return NULL;
    }
    return node;
}