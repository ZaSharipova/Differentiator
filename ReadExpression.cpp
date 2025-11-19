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

    while (buffer[*pos] != '\0' && isspace((unsigned char)buffer[*pos])) {
        (*pos)++;
    }
}

Dif_t ReadTitle(FILE *logfile, char *buffer, size_t *pos) {
    assert(logfile);
    assert(buffer);
    assert(pos);

    SkipSpaces(buffer, pos);

    size_t start = *pos;
    size_t i = start;

    if (buffer[i] == '\0' || buffer[i] == '\n') {
        fprintf(stderr, "Syntax error: empty title at position %zu\n", *pos);
        return NULL;
    }

    if (buffer[i] == '+' || buffer[i] == '-' || buffer[i] == '*' || buffer[i] == '/') {
        i++;
    } else {
        while (buffer[i] != '\0' && buffer[i] != '\n' &&
               buffer[i] != ' ' && buffer[i] != '\t' &&
               buffer[i] != '(' && buffer[i] != ')') {
            i++;
        }
    }

    if (i == start) {
        fprintf(stderr, "Syntax error: expected title at position %zu\n", *pos);
        return NULL;
    }

    size_t len = i - start;

    char saved = buffer[start + len];
    buffer[start + len] = '\0';

    fprintf(logfile, "Read title: '%s'\n", buffer + start);
    //fprintf(logfile, "ReadTitle: '%s' at %zu (len=%zu)\n", buffer + start, start, len);

    buffer[start + len] = saved;
    *pos = start + len;

    return buffer + start;
}


Value *Convert(Dif_t *ptr, DifNode_t *node, VariableInfo *arr, int *i) {
    assert(ptr);
    //assert(node);
    assert(arr);
    assert(i);

    const char *s = *ptr;
    char *endptr = NULL;

    Value *value = (Value *) calloc(1, sizeof(Value));
    if (!value) return NULL;

    if (s[0] == '+') {
        node->operation = kOperation;
        value->type = kAdd;
        return value;
    }
    if (s[0] == '-') {
        node->operation = kOperation;
        value->type = kSub;
        return value;
    }
    if (s[0] == '*') {
        node->operation = kOperation;
        value->type = kMul;
        return value;
    }
    if (s[0] == '/') {
        node->operation = kOperation;
        value->type = kDiv;
        return value;
    }

    if (isalpha((unsigned char)s[0])) {
        node->operation = kVariable;
        value->pos_of_variable = (int)s[0];
        value->variable_name = s[0];
        arr[(*i) ++].variable_name = s[0];
        return value;
    }

    double val = strtod(s, &endptr);
    if (endptr != s) {
        node->operation = kNumber;
        value->number = val;
        return value;
    }

    free(value);
    return NULL;
}


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
        fprintf(stderr, "Syntax error in %zu %c", *pos, buffer[*pos]);
        return kSyntaxError;
    }

    return kSuccess;
}

void ReadVariableValue(int size, VariableInfo *arr) {
    assert(arr);

    for (int pos = 0; pos < size; pos++) {
        int found = 0;
        for (int i = 0; i < pos; i++) {
            if (arr[i].variable_name == arr[pos].variable_name) {
                found = 1;
                break;
            }
        }
        if (found)
            continue;

        printf("Введите значение переменной %c:\n", arr[pos].variable_name);
        if (scanf("%lf", &arr[pos].variable_value) != 1) {
            fprintf(stderr, "Ошибка ввода значения переменной %c.\n", arr[pos].variable_name);
            arr[pos].variable_value = 0;
        }
        
    }
}
