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

static DifNode_t *AddOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *SubOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *MulOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *DivOptimise(DifRoot *root, DifNode_t *node, bool *has_change);
static DifNode_t *PowOptimise(DifRoot *root, DifNode_t *node, bool *has_change);

static DifNode_t *CheckNodeAndConstOptimise(DifRoot *root, DifNode_t *node, bool *has_change);

static bool IsZero(DifNode_t *node);
static bool IsOne(DifNode_t *node);
static bool IsNumber(DifNode_t *node);
static bool IsOperation(DifNode_t *node);

DifNode_t *OptimiseTree(DifRoot *root, DifNode_t *node, FILE *out, const char *main_var) {
    assert(root);
    assert(node);
    assert(out);
    assert(main_var);

    bool has_change = true;

    while (has_change) {
        has_change = false;
        node = ConstOptimise(root, node, &has_change); 
        if (has_change) {
            node->parent = NULL;
            DoTex(node, main_var, out);
        }
        node = EraseNeutralElements(root, node, &has_change);
        if (has_change) {
            node->parent = NULL;
            DoTex(node, main_var, out);
        }
    }

    node->parent = NULL;
    return node;
}

DifNode_t *ConstOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(node);
    assert(has_change);

    node->right = CheckNodeAndConstOptimise(root, node->right, has_change);
    node->left = CheckNodeAndConstOptimise(root, node->left, has_change);

    if (IsNumber(node->left) && IsNumber(node->right)) {
        double ans = EvaluateExpression(node);

        DeleteNode(node->left);
        DeleteNode(node->right);
        node->left = node->right = NULL;
        node->type = kNumber;
        node->value.number = ans;

        root->size -= 2;
        *has_change = true;
        return node;
    }

    // if (!node->left && IsNumber(node->right)) {
    //     double ans = EvaluateExpression(node);
    //     size_t removed = CountSubTreeSize(node->right) - 1;

    //     node->type = kNumber;
    //     node->value.number = ans;

    //     DeleteNode(node->left);
    //     DeleteNode(node->right);
    //     node->left = node->right = NULL;

    //     root->size -= removed;
    //     *has_change = true;
    //     return node;
    // }

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

#define NEWN(num) NewNode(root, kNumber, (Value){ .number = (num)}, NULL, NULL, NULL)
#define MUL_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationMul}, left, right, NULL)

static DifNode_t *AddOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (IsZero(node->left)) {
        DifNode_t *res = node->right;

        size_t removed = 1 + CountSubTreeSize(node->left);
        if (res) res->parent = node->parent;

        if (node->left) {
            node->left->parent = NULL;
            DeleteNode(node->left);
            node->left = NULL;
        }

        node->right = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    if (IsZero(node->right)) {
        DifNode_t *res = node->left;

        size_t removed = 1 + CountSubTreeSize(node->right);
        if (res) res->parent = node->parent;

        if (node->right) {
            node->right->parent = NULL;
            DeleteNode(node->right);
            node->right = NULL;
        }

        node->left = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    return node;
}

static DifNode_t *SubOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (IsZero(node->right)) {
        DifNode_t *res = node->left;

        size_t removed = 1 + CountSubTreeSize(node->right);
        if (res) res->parent = node->parent;

        if (node->right) {
            node->right->parent = NULL;
            DeleteNode(node->right);
            node->right = NULL;
        }

        node->left = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    if (IsZero(node->left)) {
        DifNode_t *right = node->right;
        size_t removed = 1 + CountSubTreeSize(node->left);

        if (right) right->parent = NULL;
        if (node->left) {
            node->left->parent = NULL;
            DeleteNode(node->left);
            node->left = NULL;
        }

        node->right = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        DifNode_t *neg = NEWN(-1.0);
        DifNode_t *mul = MUL_(neg, right);
        if (neg) neg->parent = mul;
        if (right) right->parent = mul;

        *has_change = true;
        return mul;
    }

    return node;
}

static DifNode_t *MulOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (IsOne(node->left)) {
        DifNode_t *res = node->right;

        size_t removed = 1 + CountSubTreeSize(node->left);
        if (res) res->parent = node->parent;

        if (node->left) {
            node->left->parent = NULL;
            DeleteNode(node->left);
            node->left = NULL;
        }

        node->right = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;
        size_t removed = 1 + CountSubTreeSize(node->right);
        if (res) res->parent = node->parent;

        if (node->right) {
            node->right->parent = NULL;
            DeleteNode(node->right);
            node->right = NULL;
        }

        node->left = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    if (IsZero(node->left) || IsZero(node->right)) {
        size_t removed = CountSubTreeSize(node);
        DeleteNode(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;

        DifNode_t *zero = NEWN(0.0);
        return zero;
    }

    return node;
}

static DifNode_t *DivOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;

        size_t removed = 1 + CountSubTreeSize(node->right);
        if (res) res->parent = node->parent;

        if (node->right) {
            node->right->parent = NULL;
            DeleteNode(node->right);
            node->right = NULL;
        }

        node->left = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    if (IsZero(node->left)) {
        size_t removed = CountSubTreeSize(node);
        DeleteNode(node);
        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        DifNode_t *zero = NEWN(0.0);
        return zero;
    }

    return node;
}

static DifNode_t *PowOptimise(DifRoot *root, DifNode_t *node, bool *has_change) {
    assert(root);
    assert(node);
    assert(has_change);

    if (IsZero(node->left)) {
        size_t removed = CountSubTreeSize(node);
        DeleteNode(node);
        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        DifNode_t *zero = NEWN(0.0);
        return zero;
    }

    if (IsZero(node->right)) {
        size_t removed = CountSubTreeSize(node);
        DeleteNode(node);
        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        DifNode_t *one = NEWN(1);
        return one;
    }

    if (IsOne(node->right)) {
        DifNode_t *res = node->left;
        size_t removed = 1 + CountSubTreeSize(node->right);
        if (res) res->parent = node->parent;

        if (node->right) {
            node->right->parent = NULL;
            DeleteNode(node->right);
            node->right = NULL;
        }

        node->left = NULL;
        free(node);

        if (root->size >= removed) root->size -= removed;
        *has_change = true;
        return res;
    }

    return node;
}
#undef NEWN
#undef MUL_

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