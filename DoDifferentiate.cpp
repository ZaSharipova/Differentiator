#include "DoDifferentiate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"

static DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var);
static DifNode_t *DoCountPowDerivative(DifNode_t *node, const char *main_var);

DifNode_t *NewNumber(double value);
DifNode_t *NewOperationNode(OperationTypes op_type, DifNode_t *left, DifNode_t *right);

#define PR node->parent
#define CL CopyNode(node->left)
#define CR CopyNode(node->right)
#define DL Dif(node->left, main_var)
#define DR Dif(node->right, main_var)
#define ADD_(left, right) NewOperationNode(kAdd, left, right)
#define SUB_(left, right) NewOperationNode(kSub, left, right)
#define MUL_(left, right) NewOperationNode(kMul, left, right) 
#define DIV_(left, right) NewOperationNode(kDiv, left, right) 
#define POW_(left, right) NewOperationNode(kPow, left, right)
#define SIN_(right) NewOperationNode(kSin, NULL, right)
#define COS_(right) NewOperationNode(kCos, NULL, right)
#define LN_(right) NewOperationNode(kLn, NULL, right)
#define NEWN(number) NewNumber(number)

DifNode_t *NewNumber(double value) {

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->operation = kNumber;
    new_node->value.number = value;

    return new_node;
}

DifNode_t *NewOperationNode(OperationTypes op_type, DifNode_t *left, DifNode_t *right) {
    assert(right);
    assert(right);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->operation = kOperation;
    new_node->value.type = op_type;

    new_node->left = left;
    new_node->right = right;

    if (left) {
        left->parent = new_node;
    }

    if (right) {
        right->parent = new_node;
    }

    return new_node;
}

DifNode_t *CopyNode(DifNode_t *node) {
    if (node == NULL) {
        return NULL;
    }

    DifNode_t *new_node = NULL;
    if (NodeCtor(&new_node, &node->value) != kSuccess) {
        fprintf(stderr, "Failed to allocate node in CopyNode.\n");
        return NULL;
    }

    new_node->operation = node->operation;
    new_node->parent = NULL;

    new_node->left = CopyNode(node->left);
    if (new_node->left) {
        new_node->left->parent = new_node;
    }

    new_node->right = CopyNode(node->right);
    if (new_node->right) {
        new_node->right->parent = new_node;
    }

    return new_node;
}


DifNode_t *Dif(DifNode_t *node, const char *main_var) {
    assert(node); // количества вершин ради передавать указатель полностью на дерево
    assert(main_var);

    if (node->operation == kNumber) {
        return NewNumber(0);
    } else if (node->operation == kVariable) {
        if (strcmp(node->value.variable_name, main_var) == 0) {
            return NewNumber(1);
        } else {
            return NewNumber(0);
        }
    } else {
        switch (node->value.type) {
        case (kAdd):
            return ADD_(DL, DR);
        case (kSub):
            return SUB_(DL, DR);
        case (kMul):
            return ADD_(MUL_(DL, CR), MUL_(CL, DR));
        case (kDiv):
            return DIV_(SUB_(MUL_(DL, CR), MUL_(CL, DR)), POW_(CR, NEWN(2)));
        case (kSin):
            return MUL_(COS_(CR), DR);
        case (kCos):
            return MUL_(MUL_(NEWN(-1), SIN_(CR)), DR);
        case (kTg):
            return MUL_(DIV_(NEWN(1), POW_(COS_(CR), NEWN(2))), DR);
        case (kLn):
            return MUL_(DIV_(NEWN(1), CR), DR);
        case (kArctg):
            return MUL_(DIV_(NEWN(1), ADD_(NEWN(1), POW_((CR), NEWN(2)))), DR);
        case (kPow):
            return (DoCountPowDerivative(node, main_var));
        case (kNone):
        default: 
            fprintf(stderr, "No such operation.\n");
            return NULL;
        }
    }
}

static DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var) {
    if (!node) {
        return kSuccess;
    }
    assert(main_var);
    assert(node_with_main_var);

    if (!(*node_with_main_var)) {
        if (node->operation == kVariable && strcmp(node->value.variable_name, main_var) == 0) {
            *node_with_main_var = node;
            return kSuccess;
        } else {
            FindMainVar(node->left, main_var, node_with_main_var);
            FindMainVar(node->right, main_var, node_with_main_var);
        }
    }
    return kSuccess;
}

static DifNode_t *DoCountPowDerivative(DifNode_t *node, const char *main_var) {
    assert(node);
    assert(main_var);

    DifNode_t *node_left_main = NULL;
    FindMainVar(node->left, main_var, &node_left_main);
    
    DifNode_t *node_right_main = NULL;
    FindMainVar(node->right, main_var, &node_right_main);

    if (!node_left_main) {
        if (!node_right_main) {
            return NEWN(0);
        } else {
            return MUL_(MUL_(POW_(CL, CR), LN_(CR)), DR);
        }
    }  else {
        if (!node_right_main) {
            return MUL_(MUL_(CR, POW_(CL, SUB_(CR, NEWN(1)))), DL);
        } else {
            return MUL_(POW_(CL, CR), Dif(MUL_(LN_(CL), CR), main_var));
        }
    }
}

#undef CL
#undef CR
#undef DL
#undef DR
#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef POW_
#undef SIN_
#undef COS_
#undef LN_
#undef NEWN