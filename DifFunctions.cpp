#include "DifFunctions.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "Enums.h"
#include "Structs.h"

#include "ReadExpression.h"
#include "Calculate.h"
#include "DoTex.h"
#include "DoGraph.h"
#include "DoDump.h"
#include "Differentiate.h"
#include "Optimise.h"

size_t DEFAULT_SIZE = 1;
double EvaluateExpression(DifNode_t *node, VariableInfo *arr);

DifErrors NodeCtor(DifNode_t **node, Value *value) {
    assert(node);

    *node = (DifNode_t *) calloc (DEFAULT_SIZE, sizeof(DifNode_t));
    if (!*node) {
        fprintf(stderr, "No memory to calloc NODE.\n");
        return kNoMemory;
    }

    if (value) {
        (*node)->value = *value; 
    } 
    
    (*node)->left =  NULL;
    (*node)->right =  NULL;
    (*node)->parent = NULL;

    return kSuccess;
}

DifErrors DifRootCtor(DifRoot *root) {
    assert(root);

    root->root = NULL;
    root->size = 0;

    return kSuccess;
}

DifErrors DeleteNode(DifNode_t *node) {
    if (!node)
        return kSuccess;

    if (node->left) {
        DeleteNode(node->left);
        node->left = NULL;
    }

    if (node->right) {
        DeleteNode(node->right);
        node->right = NULL;
    }

    // if (node->type == kVariable) {
    //     free(node->value.variable_name);
    // }
    free(node);

    return kSuccess;
}

DifErrors TreeDtor(DifRoot *tree) {
    assert(tree);

    DeleteNode(tree->root);

    tree->root =  NULL;
    tree->size = 0;

    return kSuccess;
}

DifErrors InitArrOfVariable(VariableArr *arr, size_t capacity) {
    assert(arr);

    arr->capacity = capacity;
    arr->size = 0;

    arr->var_array = (VariableInfo *) calloc (capacity, sizeof(VariableInfo));
    if (!arr->var_array) {
        fprintf(stderr, "Memory error.\n");
        return kNoMemory;
    }

    return kSuccess;
}

DifErrors ResizeArray(VariableArr *arr)  {
    assert(arr);

    if (arr->size + 2 > arr->capacity) {
        VariableInfo *ptr = (VariableInfo *) calloc (arr->capacity += 2, sizeof(VariableInfo));
        if (!ptr) {
            fprintf(stderr, "Memory error.\n");
            return kNoMemory;
        }
        arr->var_array = ptr;
    }

    return kSuccess;
}

DifErrors DtorVariableArray(VariableArr *arr) {
    assert(arr);

    arr->capacity = 0;
    arr->size = 0;

    free(arr->var_array);

    return kSuccess;
}

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *DumpInfo) {
    assert(Variable_Array);
    assert(root);
    assert(out);
    assert(DumpInfo);

    DiffModes ans = kDerivative;
    printf("Введите, что вы хотите сделать с введенным выражением: 1. Посчитать (н-ную) производную,\n");
    printf("2. Посчитать значение (н-ной) производной в точке, 3. Посчитать значение выражения, 4. Разложить по формуле Тейлора, \n");
    printf("5. Построить график.\n");
    scanf("%d", &ans);

    if (ans == kDerivativeInPos || ans == kCount) {
        ReadVariableValue(Variable_Array);
    }
    switch (ans) {
        case (kDerivativeInPos):
        case (kDerivative): {
            printf("Введите, какую производную вы хотите посчитать:\n");
            int ans2 = 0;
            scanf("%d", &ans2); // пока только 1
            DifNode_t *new_tree = Dif(root->root, root->root, "x", out);
            DifRoot root2 = {};
            root2.root = new_tree;
            DumpInfo->tree = &root2;

            strcpy(DumpInfo->message, " Do derivative");
            DoTreeInGraphviz(root2.root, DumpInfo, root2.root);
            DoDump(DumpInfo);
            DoTex(root2.root, "x", out, false);

            root2.root = OptimiseTree(root2.root, out);
            DoTreeInGraphviz(root2.root, DumpInfo, root2.root);
            strcpy(DumpInfo->message, " Optimised tree");
            DoDump(DumpInfo);
            DoTex(root2.root, "x", out, false);

            if (ans == kDerivativeInPos) {
                double res = SolveEquation(root);
                printf("Результат вычисления выражения: %lf", res);

                PrintSolution(root->root, res, out);
                strcpy(DumpInfo->message, " Calculate expression");
                DoTreeInGraphviz(root2.root, DumpInfo, root2.root);
                DoDump(DumpInfo);
            }

            printf("Скорее смотрите ДАМП и ТЕХ!!!\n");
            TreeDtor(&root2);
            break;
        }
        case (kCount): {
            double res = SolveEquation(root);
            printf("Результат вычисления выражения: %lf", res);
            PrintSolution(root->root, res, out);
            break;
        }
        case (kTeilor):
        case (kGraph):
        default:
            return kSuccess;
    }
    return kSuccess;
}