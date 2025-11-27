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
static DifErrors DoNDif(Forest *forest, DifRoot *root, DifNode_t *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var);
static void DoDerivativeInPos(DifRoot *root, DifNode_t *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out);
static DifErrors DoGnuplot(DifRoot *root);

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

    node->parent = NULL;

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

size_t CountSubTreeSize(DifNode_t *node) {
    if (node == NULL) {
        return 0;
    }

    return 1 + CountSubTreeSize(node->left) + CountSubTreeSize(node->right);
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

DifErrors ForestCtor(Forest *forest, size_t size) {
    assert(forest);

    forest->size = size;

    forest->trees = (DifRoot *) calloc (size, sizeof(DifRoot));
    if (!forest->trees) {
        fprintf(stderr, "Memory error.\n");
        return kNoMemory;
    }

    return kSuccess;
}

void ForestDtor(Forest *forest) {
    assert(forest);

    forest->size = 0;

    free(forest->trees);
}

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *DumpInfo) { //какой ужас, срочно переделать!!!!
    assert(Variable_Array);
    assert(root);
    assert(out);
    assert(DumpInfo);

    DiffModes ans = kDerivative;
    DifErrors err = kSuccess;

    bool flag_end = false;
    while (!flag_end) {
        printf("Введите, что вы хотите сделать с введенным выражением:\n 1. Посчитать (н-ную) производную,\n");
        printf("2. Посчитать значение (н-ной) производной в точке, 3. Посчитать значение выражения, 4. Разложить по формуле Тейлора, \n");
        printf("5. Построить график, 6. Выйти:\n");
        scanf("%d", &ans);

        if (ans == kDerivativeInPos || ans == kCount) {
            ReadVariableValue(Variable_Array);
        }

        switch (ans) {
            case (kDerivativeInPos):
            case (kDerivative): {
                printf("Введите, по какой переменной вы хотите посчитать производную:\n");
                char var[MAX_TEXT_SIZE] = {};
                scanf("%s", var);
                printf("Введите, какую производную вы хотите посчитать:\n");
                size_t ans2 = 0;
                scanf("%zu", &ans2);

                Forest forest = {};
                ForestCtor(&forest, ans2);
                DifNode_t root2 = {};
                CHECK_ERROR_RETURN(DoNDif(&forest, root, &root2, ans2, out, DumpInfo, var));

                if (ans == kDerivativeInPos) {
                    DoDerivativeInPos(root, &root2, Variable_Array, DumpInfo, out);
                }

                printf("Скорее смотрите ДАМП и ТЕХ!!!\n");
                ForestDtor(&forest);
                break;
            }
            case (kCount): {
                double res = SolveEquation(root);
                printf("Результат вычисления выражения: %lf", res);
                PrintSolution(root->root, res, out, Variable_Array);
                break;
            }
            case (kGraph): {
                CHECK_ERROR_RETURN(DoGnuplot(root));
                break;
            }
            case (kExit):
                flag_end = true;
            case (kTeilor):
            default:
                return kSuccess;
        }
    }
    return kSuccess;
}

void PrintExpressionToFile(FILE *out, DifRoot *root) {
    assert(out);
    assert(root);

    DifNode_t *node = root->root;

    DifNode_t *node_var = NULL;
    FindMainVar(node, "x", &node_var);
    double copied_value = node_var->value.variable->variable_value;

    for (double i = -10.0; i < 10.0; i+=0.5) {
        node_var->value.variable->variable_value = i;
        fprintf(out, "%f ", i);
        fprintf(out, "%f\n", SolveEquation(root));
    }
    node_var->value.variable->variable_value = copied_value;
}

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifNode_t *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var) {
    assert(forest);
    assert(root);
    assert(out);
    assert(DumpInfo);
    assert(main_var);

    DifRoot *node_to_dif = root;

    for (size_t i = 1; i <= ans; i++) {
        DifNode_t *new_tree = Dif(&forest->trees[i - 1], node_to_dif->root, main_var, out);
        forest->trees[i - 1].root = new_tree;
        root2 = forest->trees[i - 1].root;
        DumpInfo->tree = &forest->trees[i - 1];

        snprintf(DumpInfo->message, MAX_TEXT_SIZE, " Do (%zu) derivative", i);
        DoTreeInGraphviz(root2, DumpInfo, root2);
        DoDump(DumpInfo);
        fprintf(out, "\n\nПосчитаем %zu производную:\n\n", i);
        DoTex(forest->trees[i - 1].root, main_var, out);

        forest->trees[i - 1].root = OptimiseTree(&forest->trees[i - 1], forest->trees[i - 1].root, out, main_var);
        DoTreeInGraphviz(forest->trees[i - 1].root, DumpInfo, forest->trees[i - 1].root);
        snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Optimised tree after counting (%zu) derivative", i);
        DoDump(DumpInfo);

        node_to_dif = &forest->trees[i - 1];
    }

    return kSuccess;
}

static void DoDerivativeInPos(DifRoot *root, DifNode_t *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out) {
    assert(root);
    assert(root2);
    assert(Variable_Array);
    assert(DumpInfo);
    assert(out);

    printf("Введите, для какой производной вы хотите посчитать значение в точке:\n");
    size_t ans = 0;
    scanf("%zu", &ans); //
    double res = SolveEquation(root);
    printf("Результат вычисления выражения значения %zu производной:\n %lf", ans, res);

    PrintSolution(root->root, res, out, Variable_Array);
    strcpy(DumpInfo->message, " Calculate expression in a position in derived expression");
    DoTreeInGraphviz(root2, DumpInfo, root2);
    DoDump(DumpInfo);
}

static DifErrors DoGnuplot(DifRoot *root) {
    assert(root);

    FILE_OPEN_AND_CHECK(gnuplotfile, "gnuplot.txt", "w");
    PrintExpressionToFile(gnuplotfile, root);
    fclose(gnuplotfile);
    system("gnuplot -e \"" "set terminal pngcairo size 800,600;" "set output 'my_points_plot.png';"
    "set grid;" "set xlabel 'X';" "set ylabel 'Y';"
    "plot 'gnuplot.txt' using 1:2 with linespoints pointtype 7 pointsize 1.5;" "set output\"");
    printf("Отлично, смотрите график функции в файле %s.\n", "my_points_plot.png");

    return kSuccess;
}