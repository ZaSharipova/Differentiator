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

#define CHECK_ERROR_RETURN(cond) \
    err = cond;                  \
    if (err != kSuccess) {       \
        return err;              \
    }


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

    char *buf_in = (char *) calloc(filesize + 2, sizeof(char));
    assert(buf_in != NULL);

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

static OperationTypes ParseOperator(const char *s) {
    assert(s);

    static OpEntry operations[] = {
        {"+",     kAdd},
        {"-",     kSub},
        {"*",     kMul},
        {"/",     kDiv},
        {"tg",    kTg},
        {"sin",   kSin},
        {"cos",   kCos},
        {"ln",    kLn},
        {"arctg", kArctg},
        {"pow",   kPow},
    };

    for (size_t i = 0; i < sizeof(operations)/sizeof(operations[0]); i++) {
        size_t len = strlen(operations[i].name);
        if (strncmp(s, operations[i].name, len) == 0) {
            return operations[i].type;
        }
    }

    return kNone;
}

static int ParseNumber(const char *s, double *out_val) {
    assert(s);
    assert(out_val);

    char *end = NULL;
    double v = strtod(s, &end);

    if (end != s) {
        *out_val = v;
        return 1;
    }
    return 0;
}

static void ParseVariable(char *s, VariableInfo *arr, int *i, Value *val) {
    assert(s);
    assert(arr);
    assert(i);
    assert(val);

    val->variable_name = s;
    val->pos_of_variable = (int)s[0];

    arr[*i].variable_name = s;
    (*i)++;
}

Value *Convert(Dif_t *ptr, DifNode_t *node, VariableInfo *arr, int *i) {
    assert(ptr && node && arr && i);

    char *s = *ptr;
    Value *val = (Value *) calloc(1, sizeof(Value));
    if (!val) return NULL;

    OperationTypes op = ParseOperator(s);
    if (op != kNone) {
        node->operation = kOperation;
        val->type = op;
        return val;
    }

    double num = 0;
    if (ParseNumber(s, &num)) {
        node->operation = kNumber;
        val->number = num;
        return val;
    }

    node->operation = kVariable;
    ParseVariable(s, arr, i, val);
    return val;
}

// Value *Convert(Dif_t *ptr, DifNode_t *node, VariableInfo *arr, int *i) {
//     assert(ptr);
//     //assert(node);
//     assert(arr);
//     assert(i);

//     char *s = *ptr;
//     char *endptr = NULL;
//     double val = strtod(s, &endptr);

//     Value *value = (Value *) calloc(1, sizeof(Value));
//     if (!value) return NULL;

//     if (s[0] == '+') {
//         node->operation = kOperation;
//         value->type = kAdd;
//         return value;
//     }
//     if (s[0] == '-') {
//         node->operation = kOperation;
//         value->type = kSub;
//         return value;
//     }
//     if (s[0] == '*') {
//         node->operation = kOperation;
//         value->type = kMul;
//         return value;
//     }
//     if (s[0] == '/') {
//         node->operation = kOperation;
//         value->type = kDiv;
//         return value;
//     }

//     if (strncmp(s, "tg", sizeof("tg") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kTg;
//         return value;
//     } else if (strncmp(s, "sin", sizeof("sin") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kSin;
//         return value;
//     } else if (strncmp(s, "cos", sizeof("cos") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kCos;
//         return value;
//     } else if (strncmp(s, "ln", sizeof("ln") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kLn;
//         return value;
//     } else if (strncmp(s, "arctg", sizeof("arctg") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kArctg;
//         return value;
//     } else if (strncmp(s, "pow", sizeof("pow") - 1) == 0) {
//         node->operation = kOperation;
//         value->type = kPow;
//         return value;        
//     }
//     else if (endptr != s) {
//         node->operation = kNumber;
//         value->number = val;
//         return value;
//     }

//     else {
//         node->operation = kVariable;
//         value->pos_of_variable = (int)s[0];
//         value->variable_name = s;
//         arr[(*i) ++].variable_name = s;
//         return value;
//     }

//     return NULL;
// }


DifErrors ReadNodeFromFile(DifRoot *tree, FILE *file, FILE *logfile, size_t *pos, DifNode_t *node, Dif_t buffer, DifNode_t **node_to_add, VariableInfo *arr, int *i) {
    assert(tree);
    assert(file);
    assert(logfile);
    //assert(node);
    assert(pos);
    assert(arr);
    assert(i);
    //assert(node_to_add);

    DifErrors err = kSuccess;
    SkipSpaces(buffer, pos);
    fprintf(logfile, "\n%s", buffer + *pos);

    if (buffer[*pos] == '(') {
        tree->size ++;
        DifNode_t *new_node = NULL;
        NodeCtor(&new_node, NULL);
        (*pos)++;
        Dif_t char_to_convert = ReadTitle(logfile, buffer, pos);
        Value *val_ptr = Convert(&char_to_convert, new_node, arr, i);
        if (!val_ptr) {
            fprintf(stderr, "Conversion error.\n");
            return kSyntaxError;
        }
        new_node->value = *val_ptr;
        free(val_ptr);
        new_node->parent = node;

        SkipSpaces(buffer, pos);
        DifNode_t *left_child = NULL;
        CHECK_ERROR_RETURN(ReadNodeFromFile(tree, file, logfile, pos, new_node, buffer, &left_child, arr, i));
        new_node->left = left_child;

        SkipSpaces(buffer, pos);
        DifNode_t *right_child = NULL;
        CHECK_ERROR_RETURN(ReadNodeFromFile(tree, file, logfile, pos, new_node, buffer, &right_child, arr, i));
        new_node->right = right_child;

        SkipSpaces(buffer, pos);
        if (buffer[*pos] == ')') {
            buffer[*pos] = '\0';
            (*pos)++;
        } else {
            fprintf(stderr, "Syntax error: expected ')'\n");
            return kSyntaxError;
        }
        *node_to_add = new_node;
        return kSuccess;

    } else if (strncmp(buffer + *pos, "nil", sizeof("nil") - 1) == 0) {
        *pos += strlen("nil");
        *node_to_add = NULL;
        return kSuccess;

    } else {
        fprintf(stderr, "Syntax error in %zu '%c'\n", *pos, buffer[*pos]);
        return kSyntaxError;
    }

    return kSuccess;
}

void ReadVariableValue(int size, VariableInfo *arr) {
    assert(arr);

    for (int pos = 0; pos < size; pos++) {
        int found = 0;
        for (int i = 0; i < pos; i++) {
            if (strcmp(arr[i].variable_name, arr[pos].variable_name) == 0) {
                found = 1;
                break;
            }
        }
        if (found)
            continue;

        printf("Введите значение переменной %s:\n", arr[pos].variable_name);
        if (scanf("%lf", &arr[pos].variable_value) != 1) {
            fprintf(stderr, "Ошибка ввода значения переменной %s.\n", arr[pos].variable_name);
            arr[pos].variable_value = 0;
        }
        
    }
}
