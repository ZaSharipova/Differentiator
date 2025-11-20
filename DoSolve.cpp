#include "DoSolve.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"

#define DEFAULT_NUMBER NULL
#define CL CopyNode(node->left)
#define CR CopyNode(node->right)
#define DL Dif(node->left, main_var)
#define DR Dif(node->right, main_var)
#define ADD_(left, right) NewNode(kOperation, kAdd, left, right)
#define SUB_(left, right) NewNode(kOperation, kSub, left, right)
#define MUL_(left, right) NewNode(kOperation, kMul, left, right) 
#define DIV_(left, right) NewNode(kOperation, kDiv, left, right) 
#define POW_(left, right) NewNode(kOperation, kPow, left, right)
#define SIN_(right) NewNode(kOperation, kCos, NULL, right)
#define COS_(right) NewNode(kOperation, kCos, NULL, right)
#define NEWN(number) NewNumber(number)

DifNode_t *NewNumber(double value) {

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->operation = kNumber;

    new_node->value.number = value;

    return new_node;
}

DifNode_t *NewNode(DifTypes dif_type, OperationTypes op_type, DifNode_t *left, DifNode_t *right) {
    // assert(left);
    // assert(right);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->operation = dif_type;
    new_node->value.type = op_type;

    new_node->left = left;
    new_node->right = right;

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
    assert(node);

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
            return MUL_(NEWN(-1), SIN_(DR));
        case (kTg):
            return MUL_(DIV_(NEWN(1), POW_(COS_(CR), NEWN(2))), DR);
        case (kLn):
            return MUL_(DIV_(NEWN(1), CR), DR);
        case (kArctg):
            return MUL_(DIV_(NEWN(1), ADD_(NEWN(1), POW_((CR), NEWN(2)))), DR);
        default: 
            fprintf(stderr, "No such operation.\n");
            return NULL;
        }
    }
}