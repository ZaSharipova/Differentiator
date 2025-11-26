#include "Differentiate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "DoTex.h"

static DifNode_t *DoCountPowDerivative(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile);

DifNode_t *NewNumber(DifRoot *root, double value);
DifNode_t *NewOperationNode(DifRoot *root, OperationTypes op_type, DifNode_t *left, DifNode_t *right);

#define PR node->parent
#define CL CopyNode(root, node->left)
#define CR CopyNode(root, node->right)
#define DL Dif(root, node->left, main_var, texfile)
#define DR Dif(root, node->right, main_var, texfile)
#define ADD_(left, right) NewOperationNode(root, kOperationAdd, left, right)
#define SUB_(left, right) NewOperationNode(root, kOperationSub, left, right)
#define MUL_(left, right) NewOperationNode(root, kOperationMul, left, right) 
#define DIV_(left, right) NewOperationNode(root, kOperationDiv, left, right) 
#define POW_(left, right) NewOperationNode(root, kOperationPow, left, right)
#define SIN_(right) NewOperationNode(root, kOperationSin, NULL, right)
#define COS_(right) NewOperationNode(root, kOperationCos, NULL, right)
#define SINH_(right) NewOperationNode(root, kOperationSinh, NULL, right)
#define COSH_(right) NewOperationNode(root, kOperationCosh, NULL, right)
#define LN_(right) NewOperationNode(root, kOperationLn, NULL, right)
#define NEWN(number) NewNumber(root, number)

DifNode_t *NewNumber(DifRoot *root, double value) {
    assert(root);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL); //

    root->size ++;
    new_node->type = kNumber;
    new_node->value.number = value;

    return new_node;
}

DifNode_t *NewOperationNode(DifRoot *root, OperationTypes op_type, DifNode_t *left, DifNode_t *right) {
    assert(root);
    assert(right);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    new_node->type = kOperation;
    new_node->value.operation = op_type;

    root->size ++;
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

DifNode_t *CopyNode(DifRoot *root, DifNode_t *node) {
    assert(root);
    if (node == NULL) {
        return NULL;
    }

    DifNode_t *new_node = NULL;
    if (NodeCtor(&new_node, &node->value) != kSuccess) {
        fprintf(stderr, "Failed to allocate node in CopyNode.\n");
        return NULL;
    }

    root->size ++;
    new_node->type = node->type;
    new_node->parent = NULL;

    new_node->left = CopyNode(root, node->left); //
    if (new_node->left) {
        new_node->left->parent = new_node;
    }

    new_node->right = CopyNode(root, node->right);
    if (new_node->right) {
        new_node->right->parent = new_node;
    }

    return new_node;
}


DifNode_t *Dif(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile) {
    assert(root);
    assert(node);
    assert(main_var);
    assert(texfile);

    // Вывод ДО дифференцирования текущего узла
    //DoTexStep(root, node, main_var, texfile);
    
    if (node->type == kNumber) {
        return NEWN(0);
    } 
    
    if (node->type == kVariable) {
        if (strcmp(node->value.variable->variable_name, main_var) == 0) {
            return NEWN(1);
        } else {
            return NEWN(0);
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
            result = MUL_(DIV_(NEWN(1), ADD_(NEWN(1), POW_(CR, NEWN(2)))), DR);
            break;
        case (kOperationPow):
            result = DoCountPowDerivative(root, node, main_var, texfile);
            break;
        case (kOperationSinh):
            result = MUL_(COSH_(CR), DR);
            break;
        case (kOperationCosh):
            result = MUL_(SINH_(CR), DR);
            break; 
        case (kOperationTgh):
            result = MUL_(DIV_(NEWN(1), POW_(CR, NEWN(2))), DR);
            break;
        case (kOperationNone):
        default: 
            fprintf(stderr, "No such operation.\n");
            return NULL;
    }
    
    // Вывод ПОСЛЕ дифференцирования текущего узла
    // if (result) {
    //     printf("asdfgh\n");
    //     DoTexStep(result, result, main_var, texfile);
    // }
    
    return result;
}

DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var) {
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

static DifNode_t *DoCountPowDerivative(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile) {
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
#undef SINH_
#undef COSH_
#undef LN_
#undef NEWN