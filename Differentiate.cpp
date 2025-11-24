#include "Differentiate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "DoTex.h"

static DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var);
static DifNode_t *DoCountPowDerivative(DifNode_t *root, DifNode_t *node, const char *main_var, FILE *texfile);

DifNode_t *NewNumber(double value);
DifNode_t *NewOperationNode(OperationTypes op_type, DifNode_t *left, DifNode_t *right);

#define PR node->parent
#define CL CopyNode(node->left)
#define CR CopyNode(node->right)
#define DL Dif(root, node->left, main_var, texfile)
#define DR Dif(root, node->right, main_var, texfile)
#define ADD_(left, right) NewOperationNode(kOperationAdd, left, right)
#define SUB_(left, right) NewOperationNode(kOperationSub, left, right)
#define MUL_(left, right) NewOperationNode(kOperationMul, left, right) 
#define DIV_(left, right) NewOperationNode(kOperationDiv, left, right) 
#define POW_(left, right) NewOperationNode(kOperationPow, left, right)
#define SIN_(right) NewOperationNode(kOperationSin, NULL, right)
#define COS_(right) NewOperationNode(kOperationCos, NULL, right)
#define LN_(right) NewOperationNode(kOperationLn, NULL, right)
#define NEWN(number) NewNumber(number)

DifNode_t *NewNumber(double value) {

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL); //

    new_node->type = kNumber;
    new_node->value.number = value;

    return new_node;
}

DifNode_t *NewOperationNode(OperationTypes op_type, DifNode_t *left, DifNode_t *right) {
    assert(right);
    assert(right);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->type = kOperation;
    new_node->value.operation = op_type;

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

    new_node->type = node->type;
    new_node->parent = NULL;

    new_node->left = CopyNode(node->left); //
    if (new_node->left) {
        new_node->left->parent = new_node;
    }

    new_node->right = CopyNode(node->right);
    if (new_node->right) {
        new_node->right->parent = new_node;
    }

    return new_node;
}


DifNode_t *Dif(DifNode_t *root, DifNode_t *node, const char *main_var, FILE *texfile) {
    assert(root);
    assert(node);
    assert(main_var);
    assert(texfile);

    // Вывод ДО дифференцирования текущего узла
    DoTexStep(root, node, main_var, texfile);
    
    if (node->type == kNumber) {
        return NewNumber(0);
    } 
    
    if (node->type == kVariable) {
        if (strcmp(node->value.variable->variable_name, main_var) == 0) {
            return NewNumber(1);
        } else {
            return NewNumber(0);
        }
    }

    DifNode_t *result = NULL;
    
    switch (node->value.operation) {
        case (kOperationAdd):
            result = ADD_(DL, DR);
            break;
        case (kOperationSub):
            result = SUB_(DL, DR);
            break;
        case (kOperationMul):
            result = ADD_(MUL_(DL, CR), MUL_(CL, DR));
            break;
        case (kOperationDiv):
            result = DIV_(SUB_(MUL_(DL, CR), MUL_(CL, DR)), POW_(CR, NEWN(2)));
            break;
        case (kOperationSin):
            result = MUL_(COS_(CR), DR);
            break;
        case (kOperationCos):
            result = MUL_(MUL_(NEWN(-1), SIN_(CR)), DR);
            break;
        case (kOperationTg):
            result = MUL_(DIV_(NEWN(1), POW_(COS_(CR), NEWN(2))), DR);
            break;
        case (kOperationLn):
            result = MUL_(DIV_(NEWN(1), CR), DR);
            break;
        case (kOperationArctg):
            result = MUL_(DIV_(NEWN(1), ADD_(NEWN(1), POW_((CR), NEWN(2)))), DR);
            break;
        case (kOperationPow):
            result = DoCountPowDerivative(root, node, main_var, texfile);
            break;
        case (kOperationNone):
        default: 
            fprintf(stderr, "No such operation.\n");
            return NULL;
    }
    
    // Вывод ПОСЛЕ дифференцирования текущего узла
    if (result) {
        DoTexStep(root, result, main_var, texfile);
    }
    
    return result;
}

static DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var) {
    if (!node) {
        return kSuccess;
    }
    assert(main_var);
    assert(node_with_main_var);

    if (!(*node_with_main_var)) {
        if (node->type == kVariable && strcmp(node->value.variable->variable_name, main_var) == 0) {
            *node_with_main_var = node;
            return kSuccess;
        } else {
            FindMainVar(node->left, main_var, node_with_main_var);
            FindMainVar(node->right, main_var, node_with_main_var);
        }
    }
    return kSuccess;
}

static DifNode_t *DoCountPowDerivative(DifNode_t *root, DifNode_t *node, const char *main_var, FILE *texfile) {
    assert(root);
    assert(node);
    assert(main_var);
    assert(texfile);

    DifNode_t *node_left_main = NULL;
    FindMainVar(node->left, main_var, &node_left_main);
    
    DifNode_t *node_right_main = NULL;
    FindMainVar(node->right, main_var, &node_right_main);

    if (!node_left_main) {
        if (!node_right_main) {
            return NEWN(0);
        }
        return MUL_(MUL_(POW_(CL, CR), LN_(CR)), DR);
    } 
    if (!node_right_main) {
        return MUL_(MUL_(CR, POW_(CL, SUB_(CR, NEWN(1)))), DL);
    } 
    return MUL_(POW_(CL, CR), Dif(root, MUL_(LN_(CL), CR), main_var, texfile));

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