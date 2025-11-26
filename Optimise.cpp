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

static DifNode_t *AddOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *SubOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *MulOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *DivOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *PowOptimise(DifRoot *root, DifNode_t *node, bool *has_change);

size_t FreeSubTree(DifNode_t *node);

static DifNode_t *CheckNodeAndConstOptimise(DifRoot *root, DifNode_t *node, 
    bool *has_change);

static bool IsZero(DifNode_t *node);
static bool IsOne(DifNode_t *node);
static bool IsNumber(DifNode_t *node);
static bool IsOperation(DifNode_t *node);

#define NEWN(number) NewNumber(root, number)

DifNode_t *OptimiseTree(DifRoot *root, DifNode_t *node, FILE *out) {
    assert(root);
    assert(node);
    assert(out);

    bool has_change = true;

    while (has_change) {
        has_change = false;
        node = ConstOptimise(root, node, &has_change); 
        if (has_change) DoTex(node, "x", out); // NULL
        node = EraseNeutralElements(root, node, &has_change); //
        if (has_change) DoTex(node, "x", out); // NULL
    }
    return node;
}

DifNode_t *ConstOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    CheckNodeAndConstOptimise(root, node->right, has_change);
    CheckNodeAndConstOptimise(root, node->left, has_change);

    if (IsNumber(node->left) && IsNumber(node->right)) {
        double ans = EvaluateExpression(node);
        size_t removed = CountSubTreeSize(node) - 1;

        DeleteNode(node->left);
        DeleteNode(node->right);
        node->left = node->right = NULL;
        node->type = kNumber;
        node->value.number = ans;

        root->size -= removed;
        *has_change = true;
        return node;
    }

    if (!node->left && IsNumber(node->right)) {
        double ans = EvaluateExpression(node);
        size_t removed = CountSubTreeSize(node->right);

        node->right = NULL;
        node->type = kNumber;
        node->value.number = ans;

        DeleteNode(node);
        root->size -= removed;
        *has_change = true;
        return node;
    }

    return node;
}

DifNode_t *EraseNeutralElements(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (node->left) {
        node->left = EraseNeutralElements(root, node->left, has_change);
    }

    if (node->right) {
        node->right = EraseNeutralElements(root, node->right, has_change);
    }

    if ((!node->left || !node->right) || !IsOperation(node)) {
        return node;
    }

    OperationTypes operation = node->value.operation;

    if (operation == kOperationAdd) {
        return AddOptimise(root, node, has_change);
    }
    if (operation == kOperationSub) {
        return SubOptimise(root, node, has_change);
    } 
    if (operation == kOperationMul) {
        return MulOptimise(root, node, has_change);
    }
    if (operation == kOperationDiv) {
        return DivOptimise(root, node, has_change);
    }
    if (operation == kOperationPow) {
        return PowOptimise(root, node, has_change);
    }

    return node;
}


static DifNode_t *AddOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    DifNode_t *copied_node = NULL;

   if (IsZero(node->left)) {
        DifNode_t *res = node->right;

        node->right = node->left = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }

    if (IsZero(node->right)) {
        DifNode_t *res = node->left;

        node->right = node->left = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }

    return node;
}

#define MUL_(left, right) NewOperationNode(root, kOperationMul, left, right) 

static DifNode_t *SubOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (IsZero(node->right)) {
        DifNode_t *res = node->left;

        node->left = node->right = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }


    if (IsZero(node->left)) {
        DifNode_t *x = node->right;

        node->right = node->left = NULL; // удалить надо ли ноду
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return NewOperationNode(root, kOperationMul,
            NEWN(-1), x);
    }

    return node;
}
#undef MUL_

static DifNode_t *MulOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *new_node = NULL;

    if (IsOne(node->left)) {
        DifNode_t *res = node->right;

        node->left = node->right = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;

        node->left = node->right = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }

    if (IsZero(node->left) || IsZero(node->right)) {
        root->size -= CountSubTreeSize(node);
        DeleteNode(node); //

        *has_change = true;
        return NewNumber(root, 0);
    }
    return node;
}

static DifNode_t *DivOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    DifNode_t *new_node = NULL;

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;

        node->left = node->right = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
    }

    if (IsZero(node->left)) {
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return NewNumber(root, 0);
    }
    return node;
}

static DifNode_t *PowOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    if (IsZero(node->left)) {
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return NewNumber(root, 0);
    }

    if (IsZero(node->right)) {
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return NewNumber(root, 1);
    }

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;

        node->left = node->right = NULL;
        root->size -= CountSubTreeSize(node);
        DeleteNode(node);

        *has_change = true;
        return res;
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

static DifNode_t *CheckNodeAndConstOptimise(DifRoot *root, DifNode_t *node, 
    bool *has_change) {
    assert(has_change);

    if (node && !IsNumber(node)) {
        node = ConstOptimise(root, node, has_change);
        if (!node) return NULL;
    }
    return node;
}

size_t FreeSubTree(DifNode_t *node) {
    if (!node) return 0;

    size_t count = 1;
    count += FreeSubTree(node->left);
    count += FreeSubTree(node->right);

    free(node);
    return count;
}
