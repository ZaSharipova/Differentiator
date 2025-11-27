#include "ReadInfixExpression.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "Differentiate.h"
#include "DifFunctions.h"
#include "Calculate.h"

#define CHECK_NULL_RETURN(name, cond) \
    DifNode_t *name = cond;           \
    if (name == NULL) {               \
        return NULL;                  \
    }

DifNode_t *GetExpression(DifRoot *root, const char **string);

DifNode_t *GetNumber(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    DifNode_t *val = NULL;
    NodeCtor(&val, 0);
    val->type = kNumber;
    while ('0' <= **string && **string <= '9') {
        val->value.number = val->value.number * 10 + (**string - '0');
        (*string)++;
    }
    return val;
}

DifNode_t *GetString(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    //static size_t pos_in_arr = 0;
    char *buf = (char *) calloc (MAX_TEXT_SIZE, sizeof(char));
    int pos = 0;

    while ('a' <= **string && **string <= 'z') {
        buf[pos++] = **string;
        (*string)++;
    }
    buf[pos] = '\0';

    DifNode_t *var_node = NULL;
    NodeCtor(&var_node, 0);
    var_node->type = kVariable;
    
    var_node->value.variable->variable_name = strdup(buf);
    if (var_node->value.variable->variable_name == NULL) {
        free(var_node);
        return NULL;
    }
    
    return var_node;
}

DifNode_t *GetPrimary(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    if (**string == '(') {
        (*string)++;
        CHECK_NULL_RETURN(val, GetExpression(root, string));

        if (**string == ')') {
            (*string)++;
        } else {
            fprintf(stderr, "SYNTAX_ERROR_P: expected ')', got '%c'", **string);
            return NULL;
        }
        return val;

    }

    CHECK_NULL_RETURN(value_number, GetNumber(root, string));
    if (value_number) {
        return value_number;
    }

    return GetString(root, string);
    
}

DifNode_t *GetTerm(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    CHECK_NULL_RETURN(val, GetPrimary(root, string));

    while (**string == '*' || **string == '/') {
        int op = **string;
        (*string)++;
        CHECK_NULL_RETURN(val2, GetPrimary(root, string));

        if (op == '*') {
            val = NewOperationNode(root, kOperationMul, val, val2);
        } else if (op == '/') {
            val = NewOperationNode(root, kOperationDiv, val, val2);
        }
    }

    return val;
}

DifNode_t *GetExpression(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    CHECK_NULL_RETURN(val, GetTerm(root, string));

    while (**string == '+' || **string == '-') {
        int op = **string;
        (*string)++;
        CHECK_NULL_RETURN(val2, GetTerm(root, string));

        if (op == '+') {
            val = NewOperationNode(root, kOperationAdd, val, val2);
        } else if (op == '-') {
            val = NewOperationNode(root, kOperationSub, val, val2);
        }
    }

    return val;
}

DifNode_t *GetGoal(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    CHECK_NULL_RETURN(val, GetExpression(root, string));

    if (**string == '$') {
        (*string)++;
    } else {
        fprintf(stderr, "SYNTAX_ERROR_G: expected '$', got '%c'", **string);
        return NULL;
    }
    return val;
}
