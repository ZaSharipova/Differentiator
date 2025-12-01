#include "Differentiate.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"
#include "DoTex.h"

static DifNode_t *DoCountPowDerivative(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile, VariableArr *Variable_Array);

DifNode_t *NewNode(DifRoot *root, DifTypes type, Value value, DifNode_t *left, DifNode_t *right, VariableArr *Variable_Array);

#define PR node->parent
#define CL CopyNode(root, node->left)
#define CR CopyNode(root, node->right)
#define DL Dif(root, node->left, main_var, texfile, Variable_Array)
#define DR Dif(root, node->right, main_var, texfile, Variable_Array)
#define NEWN(num) NewNode(root, kNumber, ((Value){ .number = (num)}), NULL, NULL, Variable_Array)
#define ADD_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationAdd}, left, right, Variable_Array)
#define SUB_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationSub}, left, right, Variable_Array)
#define MUL_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationMul}, left, right, Variable_Array)
#define DIV_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationDiv}, left, right, Variable_Array)
#define POW_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationPow}, left, right, Variable_Array)
#define SIN_(right) NewNode(root, kOperation, (Value){ .operation = kOperationSin}, NULL, right, Variable_Array)
#define COS_(right) NewNode(root, kOperation, (Value){ .operation = kOperationCos}, NULL, right, Variable_Array)
#define SINH_(right) NewNode(root, kOperation, (Value){ .operation = kOperationSinh}, NULL, right, Variable_Array)
#define COSH_(right) NewNode(root, kOperation, (Value){ .operation = kOperationCosh}, NULL, right, Variable_Array)
#define LN_(right) NewNode(root, kOperation, (Value){ .operation = kOperationLn}, NULL, right, Variable_Array)


DifNode_t *NewNode(DifRoot *root, DifTypes type, Value value, DifNode_t *left, DifNode_t *right,
    VariableArr *Variable_Array) {
    assert(root);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    root->size++;
    new_node->type = type;

    switch (type) {

    case kNumber:
        new_node->value.number = value.number;
        break;

    case kVariable: {
        VariableInfo *addr = NULL;
        for (size_t i = 0; i < Variable_Array->size; i++) {
            if (strcmp(value.variable->variable_name,
                       Variable_Array[i].var_array->variable_name) == 0) {
                addr = Variable_Array[i].var_array;
                break;
            }
        }

        if (!addr) {
            fprintf(stderr, "Unknown variable: %s\n",
                    value.variable->variable_name);
            return NULL;
        }

        new_node->value.variable = addr;
        break;
    }

    case kOperation:
        new_node->value.operation = value.operation;
        new_node->left = left;
        new_node->right = right;

        if (left)  left->parent  = new_node;
        if (right) right->parent = new_node;
        break;
    }

    return new_node;
}

DifNode_t *CopyNode(DifRoot *root, DifNode_t *node){
    if (!node) return NULL;

    DifNode_t *copy = NULL;
    NodeCtor(&copy, NULL);

    root->size++;
    copy->type = node->type;
    copy->parent = NULL;

    switch (node->type) {

    case kNumber:
        copy->value.number = node->value.number;
        break;

    case kVariable:
        // переменная — просто указатель (не копируется)
        copy->value.variable = node->value.variable;
        break;

    case kOperation:
        copy->value.operation = node->value.operation;
        copy->left  = CopyNode(root, node->left);
        copy->right = CopyNode(root, node->right);

        if (copy->left)  copy->left->parent  = copy;
        if (copy->right) copy->right->parent = copy;
        break;
    }

    return copy;
}

DifNode_t *Dif(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile, VariableArr *Variable_Array) {
    assert(root);
    assert(node);
    assert(main_var);
    assert(texfile);
    
    if (node->type == kNumber) {
        return NEWN(0);
    } 
    
    if (node->type == kVariable) {
        if (strncmp(node->value.variable->variable_name, main_var, strlen(main_var)) == 0) {
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
            result = DoCountPowDerivative(root, node, main_var, texfile, Variable_Array);
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

    PrintShrich(node, result, texfile);
    
    return result;
}

DifErrors FindMainVar(DifNode_t *node, const char *main_var, DifNode_t **node_with_main_var) {
    if (!node) {
        return kSuccess;
    }
    assert(main_var);
    assert(node_with_main_var);

    if (!(*node_with_main_var)) {
        if (node->type == kVariable && strncmp(node->value.variable->variable_name, main_var, strlen(main_var)) == 0) {
            *node_with_main_var = node;
            return kSuccess;
        } else {
            FindMainVar(node->left, main_var, node_with_main_var);
            FindMainVar(node->right, main_var, node_with_main_var);
        }
    }
    return kSuccess;
}

static DifNode_t *DoCountPowDerivative(DifRoot *root, DifNode_t *node, const char *main_var, FILE *texfile, VariableArr *Variable_Array) {
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
    return MUL_(POW_(CL, CR), Dif(root, MUL_(LN_(CL), CR), main_var, texfile, Variable_Array));

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

DifNode_t *NewVariable(DifRoot *root, const char *variable, VariableArr *VariableArr) {
    assert(root);
    assert(variable);

    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    root->size ++;
    new_node->type = kVariable;
    VariableInfo *addr = NULL;

    for (size_t i = 0; i < VariableArr->size; i++) {
        if (strcmp(variable, VariableArr[i].var_array->variable_name) == 0) {
           addr = VariableArr[i].var_array;
        }
    }

    new_node->value.variable = (VariableInfo *) calloc (1, sizeof(VariableInfo));
        
    new_node->value.variable = addr;

    return new_node;
}