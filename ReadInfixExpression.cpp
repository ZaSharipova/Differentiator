#include "ReadInfixExpression.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

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

static OpEntry operations[] = {
    {"tg",    kOperationTg},
    {"sin",   kOperationSin},
    {"cos",   kOperationCos},
    {"ln",    kOperationLn},
    {"arctg", kOperationArctg},
    {"pow",   kOperationPow},
    {"sh",    kOperationSinh},
    {"ch",    kOperationCosh},
    {"th",    kOperationTgh},
};
size_t max_op_size = 5; //

static DifNode_t *GetExpression(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetTerm(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetPrimary(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);

static DifNode_t *GetNumber(DifRoot *root, const char **string);
static DifNode_t *GetString(DifRoot *root, const char **string, VariableArr *arr, size_t *pos);
static DifNode_t *GetOperation(DifRoot *root, const char **string, VariableArr *arr, size_t *position);
static OperationTypes ParseOperator(const char *string);


DifNode_t *GetGoal(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);
    
    CHECK_NULL_RETURN(val, GetExpression(root, string, arr, pos));
    
    if (**string == '$') {
        (*string)++;
    } else {
        fprintf(stderr, "SYNTAX_ERROR_G: expected '$', got '%c'", **string);
        return NULL;
    }
    return val;
}

static DifNode_t *GetExpression(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    CHECK_NULL_RETURN(val, GetTerm(root, string, arr, pos));
    root->size++;

    while (**string == '+' || **string == '-') {
        int op = **string;
        (*string)++;
        CHECK_NULL_RETURN(val2, GetTerm(root, string, arr, pos));

        if (op == '+') {
            val = NewOperationNode(root, kOperationAdd, val, val2);
        } else if (op == '-') {
            val = NewOperationNode(root, kOperationSub, val, val2);
        }
        root->size += 1;
    }

    return val;
}

static DifNode_t *GetTerm(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    CHECK_NULL_RETURN(val, GetPrimary(root, string, arr, pos));
    root->size ++;

    while (**string == '*' || **string == '/' || **string == '^') {
        int op = **string;
        (*string)++;
        CHECK_NULL_RETURN(val2, GetPrimary(root, string, arr, pos));

        if (op == '*') {
            val = NewOperationNode(root, kOperationMul, val, val2);
        } else if (op == '/') {
            val = NewOperationNode(root, kOperationDiv, val, val2);
        } else if (op == '^') {
            val = NewOperationNode(root, kOperationPow, val, val2);
        }
        root->size += 1;
    }

    return val;
}

static DifNode_t *GetPrimary(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) {
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    if (**string == '(') {
        (*string)++;
        CHECK_NULL_RETURN(val, GetExpression(root, string, arr, pos));

        if (**string == ')') {
            (*string)++;
        } else {
            fprintf(stderr, "SYNTAX_ERROR_P: expected ')', got '%c'", **string);
            return NULL;
        }
        return val;

    }

    DifNode_t *value_number = GetNumber(root, string);

    if (value_number) {
        return value_number;
    }

    DifNode_t *value_operation = GetOperation(root, string, arr, pos);
    if (value_operation) {
        return value_operation;
    }
    return GetString(root, string, arr, pos);
    
}

static DifNode_t *GetNumber(DifRoot *root, const char **string) {
    assert(root);
    assert(string);

    DifNode_t *val = NULL;
    NodeCtor(&val, 0);
    if (!val) {
        return NULL;
    }

    val->type = kNumber;
    const char *last_string = *string;
    while ('0' <= **string && **string <= '9') {
        val->value.number = val->value.number * 10 + (**string - '0');
        (*string)++;
    }

    if (last_string == *string) {
        return NULL;
    }
    return val;
}

static DifNode_t *GetString(DifRoot *root, const char **string, VariableArr *arr, size_t *pos) { //
    assert(root);
    assert(string);
    assert(arr);
    assert(pos);

    char *buf = (char *) calloc (MAX_TEXT_SIZE, sizeof(char));
    if (!buf) {
        fprintf(stderr, "ERROR no memory.\n");
        return NULL;
    }
    int position = 0;

    const char *last_string = *string;
    while ('a' <= **string && **string <= 'z') {
        buf[position] = **string;
        position ++;
        (*string)++;
    }

    if (last_string == *string) {
        return NULL;
    }

    buf[position] = '\0';

    DifNode_t *var_node = NULL;
    NodeCtor(&var_node, 0);
    if (!var_node) {
        return NULL;
    }

    var_node->type = kVariable;
    
    bool flag_found = false;
    Value val = {};

    for (size_t i = 0; i < *pos; i++) {
        if (strcmp(arr->var_array[i].variable_name, buf) == 0) {
            val.variable = &arr->var_array[i];
            var_node->value = val;
            flag_found = true;
        }
    }

    ResizeArray(arr);

    if (!flag_found) {
        arr->var_array[*pos].variable_name = buf;
        val.variable = &arr->var_array[*pos];
        arr->size ++;
        var_node->value = val;
        (*pos)++;
    }

    return var_node;
}

static DifNode_t *GetOperation(DifRoot *root, const char **string, VariableArr *arr, size_t *position) {
    assert(root);
    assert(string);
    assert(arr);
    assert(position);

    OperationTypes type = kOperationNone;

    char *buf = (char *) calloc (MAX_TEXT_SIZE, sizeof(char));
    if (!buf) {
        fprintf(stderr, "ERROR no memory.\n");
        return NULL;
    }

    size_t pos = 0;
    const char *last_position = *string;

    while (isalpha(**string) && pos <= max_op_size) {
        buf[pos] = **string;
        pos++;
        (*string)++;

        type = ParseOperator(buf);
        if (type != kOperationNone) {
            DifNode_t *new_node = GetPrimary(root, string, arr, position);
            free(buf);
            return NewOperationNode(root, type, NewNumber(root, 0), new_node);
        } else if (type != kOperationPow) {

        }
    }
    *string = last_position;

    free(buf);
    return NULL;
}

static OperationTypes ParseOperator(const char *string) {
    assert(string);

    size_t op_size = sizeof(operations)/sizeof(operations[0]);

    for (size_t i = 0; i < op_size; i++) {
        if (strncmp(string, operations[i].name, strlen(operations[i].name)) == 0) { //
            return operations[i].type;
        }
    }

    return kOperationNone;
}