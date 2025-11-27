#include "ReadExpression.h"

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"
#include "DifFunctions.h"


long long SizeOfFile(const char *filename) {
    assert(filename);

    struct stat stbuf = {};

    int err = stat(filename, &stbuf);
    if (err != kSuccess) {
        perror("stat() failed");
        return kErrorStat;
    }

    return stbuf.st_size;
}

char *ReadToBuf(const char *filename, FILE *file, size_t filesize) {
    assert(filename);
    assert(file);

    char *buf_in = (char *) calloc (filesize + 2, sizeof(char));
    if (!buf_in) {
        fprintf(stderr, "ERROR while calloc.\n");
        return NULL;
    }

    size_t bytes_read = fread(buf_in, sizeof(buf_in[0]), filesize, file);
    if (bytes_read == 0) {
        buf_in[0] = '\n';
        buf_in[1] = '\0';

    } else {
        if (buf_in[bytes_read - 1] != '\n') {
            buf_in[bytes_read] = '\n';
            bytes_read++;
        }

        buf_in[bytes_read] = '\0';
    }

    return buf_in;
}

void DoBufRead(FILE *file, const char *filename, FileInfo *Info) {
    assert(file);
    assert(filename);
    assert(Info);

    Info->filesize = (size_t)SizeOfFile(filename) * 4;

    Info->buf_ptr = ReadToBuf(filename, file, Info->filesize);
    assert(Info->buf_ptr != NULL);
}

void SkipSpaces(char *buffer, size_t *pos) {
    assert(buffer);
    assert(pos);

    while (buffer[*pos] == '\0' || isspace((unsigned char)buffer[*pos])) {
        (*pos)++;
    }
}

Dif_t ReadTitle(FILE *logfile, char *buffer, size_t *pos) {
    assert(logfile);
    assert(buffer);
    assert(pos);

    SkipSpaces(buffer, pos);

    size_t start = *pos;
    int cnt = 0;

    if (buffer[start] == '\0' || buffer[start] == '\n') {
        fprintf(stderr, "Syntax error: empty title at position %zu\n", *pos);
        return NULL;
    }

    int result = sscanf(buffer + *pos, "%*[^' ']%n", &cnt);
    if (result < 0) {
        fprintf(stderr, "Sscanf error.");
        return NULL;
    }
    *(buffer + *pos + cnt) = '\0';

    fprintf(logfile, "'%s'\n", buffer + *pos);
    *pos = start + (size_t)cnt;

    return buffer + start;
}

static OpEntry operations[] = {
    {"+",     kOperationAdd},
    {"-",     kOperationSub},
    {"*",     kOperationMul},
    {"/",     kOperationDiv},
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

static OperationTypes ParseOperator(const char *string) {
    assert(string);

    size_t op_size = sizeof(operations)/sizeof(operations[0]);

    for (size_t i = 0; i < op_size; i++) {
        if (strncmp(string, operations[i].name, sizeof(*operations[i].name)) == 0) {
            return operations[i].type;
        }
    }

    return kOperationNone;
}

static bool ParseNumber(const char *string, double *out_val) {
    assert(string); 
    assert(out_val);

    char *end = NULL;
    double v = strtod(string, &end);

    if (end != string) {
        *out_val = v;
        return true;
    }
    return false;
}

static DifErrors ParseVariable(char *string, VariableArr *arr, int *pos, Value *val) {
    assert(string);
    assert(arr);
    assert(pos);

    bool flag_found = false;

    for (int i = 0; i < *pos; i++) {
        if (strcmp(arr->var_array[i].variable_name, string) == 0) {
            val->variable = &arr->var_array[i];
            flag_found = true;
        }
    }

    DifErrors err = kSuccess;
    CHECK_ERROR_RETURN(ResizeArray(arr));

    if (!flag_found) {
        arr->var_array[*pos].variable_name = string;
        val->variable = &arr->var_array[*pos];
        arr->size ++;
        (*pos)++;
    }

    return kSuccess;
}

Value CheckType(Dif_t *ptr, DifNode_t *node, VariableArr *arr, int *pos) {
    assert(ptr);
    assert(node);
    assert(arr);
    assert(pos);

    char *string = *ptr;
    Value val = {};

    OperationTypes op = ParseOperator(string);
    if (op != kOperationNone) {
        node->type = kOperation;
        val.operation = op;
        return val;
    }

    double num = 0;
    if (ParseNumber(string, &num)) {
        node->type = kNumber;
        val.number = num;
        return val;
    }

    node->type = kVariable;
    ParseVariable(string, arr, pos, &val); //
    return val;
}


static DifErrors ParseMainNode(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *parent, Dif_t buffer,
    DifNode_t **node_to_add, VariableArr *arr, int *i) { //
    assert(tree);
    assert(file);
    assert(logfile);
    assert(pos);
    assert(arr);
    assert(i);

    DifErrors err = kSuccess;
    tree->size++;
    DifNode_t *new_node = NULL;
    NodeCtor(&new_node, NULL);

    (*pos)++;
    Dif_t char_to_convert = ReadTitle(logfile, buffer, pos);

    Value val_ptr = CheckType(&char_to_convert, new_node, arr, i);

    new_node->value = val_ptr;
    new_node->parent = parent;

    DifNode_t *left_child = NULL;
    CHECK_ERROR_RETURN(ReadNodeFromFile(tree, file, logfile, pos, new_node, buffer, &left_child, arr, i));
    new_node->left = left_child;

    DifNode_t *right_child = NULL;
    CHECK_ERROR_RETURN(ReadNodeFromFile(tree, file, logfile, pos, new_node, buffer, &right_child, arr, i));
    new_node->right = right_child;

    SkipSpaces(buffer, pos);
    if (buffer[*pos] != ')') {
        fprintf(stderr, "Syntax error: expected ')'\n"); // return node
        return kSyntaxError;
    }

    (*pos)++;
    *node_to_add = new_node;
    return kSuccess;
}

static DifErrors ParseNilNode(size_t *pos, DifNode_t **node_to_add) {
    assert(pos);
    assert(node_to_add);

    *pos += strlen("nil"); //
    *node_to_add = NULL;
    return kSuccess;
}

static DifErrors SyntaxErrorNode(size_t pos, char c) {

    fprintf(stderr, "Syntax error in %zu '%c'\n", pos, c);
    return kSyntaxError;
}


DifErrors ReadNodeFromFile(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *parent, 
    Dif_t buffer, DifNode_t **node_to_add, VariableArr *arr, int *i) {
    assert(tree);
    assert(file);
    assert(logfile);
    assert(pos);
    assert(arr);
    assert(i);

    SkipSpaces(buffer, pos);
    fprintf(logfile, "\n%s", buffer + *pos);

    if (buffer[*pos] == '(') {
        return ParseMainNode(tree, file, logfile, pos, parent, buffer, node_to_add, arr, i);
    }

    if (strncmp(buffer + *pos, "nil", sizeof("nil") - 1) == 0) {
        return ParseNilNode(pos, node_to_add);
    }

    return SyntaxErrorNode(*pos, buffer[*pos]);
}


void ReadVariableValue(VariableArr *arr) {
    assert(arr);

    for (size_t pos = 0; pos < arr->size; pos++) {
        printf("Введите значение переменной %s:\n", arr->var_array[pos].variable_name);
        if (scanf("%lf", &arr->var_array[pos].variable_value) != 1) {
            fprintf(stderr, "Ошибка ввода значения переменной %s.\n", arr->var_array[pos].variable_name);
            arr->var_array[pos].variable_value = 0; //
        }
        
    }
}