#include "PlayMode.h"

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
#include "ReadInfixExpression.h"
#include "DifFunctions.h"

#define RESET   "\033[0m"
#define WHITE   "\033[1;30m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

static FILE *SelectInputFile(void);

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifRoot *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var);
static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var);
static DifErrors DoGnuplot(DifRoot *root, VariableArr *Variable_Array);
DifErrors CountInfix(DifRoot *root, DumpInfo *dump_info, VariableArr *Variable_Array);

static void DoSystemForGnuplot(const char *main_var);

static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
                                      FILE *out, DumpInfo *DumpInfo, DiffModes ans);
static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out);
static void ReadDerivativeParameters(char *var, size_t *amount_of_dif);

static void PrintMenu(void);
static DifErrors DivideChoice(DiffModes ans, DifRoot *root, VariableArr *Variable_Array,
                                  FILE *out, DumpInfo *DumpInfo, bool *flag_end);

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans);

static const char *AskFilename(void);

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *Dump_Info) {
    assert(Variable_Array);
    assert(root);
    assert(out);
    assert(Dump_Info);

    DifErrors err = kSuccess;
    // CHECK_ERROR_RETURN(VerifyTree(root));

    bool flag_end = false;
    while (!flag_end) {
        PrintMenu();
        DiffModes ans = kDerivative;
        scanf("%d", &ans);

        err = DivideChoice(ans, root, Variable_Array, out, Dump_Info, &flag_end);
        if (err != kSuccess) return err;
    }
    return kSuccess;
}

static void PrintMenu(void) {
    printf(CYAN "Введите, что вы хотите сделать с введенным выражением:\n"
            YELLOW "1." RESET "Посчитать все, \n"
            YELLOW "2." RESET " Посчитать (н-ную) производную,\n"
            YELLOW "3." RESET " Посчитать значение (н-ной) производной в точке,\n" 
            YELLOW "4." RESET " Посчитать значение выражения,\n"
            YELLOW "5." RESET " Разложить по формуле Тейлора,\n"
            YELLOW "6." RESET " Построить график,\n"
            YELLOW "7." RESET " Выйти:\n");
}

static DifErrors DivideChoice(DiffModes ans, DifRoot *root, VariableArr *Variable_Array,
                                  FILE *out, DumpInfo *DumpInfo, bool *flag_end) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);
    assert(flag_end);

    switch (ans) {
        case kAll:
            return DoAllOptions(root, Variable_Array, out, DumpInfo, ans);

        case kDerivativeInPos:
        case kDerivative:
            return DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans);
            
        case kCount:
            return DoCountCase(root, Variable_Array, out);
            
        case kGraph:
            return DoGnuplot(root, Variable_Array);
            
        case kExit:
            *flag_end = true;
            return kSuccess;
            
        case kTeilor:
            // TODO: реализовать разложение Тейлора
            printf("Разложение Тейлора пока не реализовано.\n");
            return kSuccess;
            
        default:
            printf("Неизвестная команда.\n");
            return kSuccess;
    }
}

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans);
    DoCountCase(root, Variable_Array, out);
    DoGnuplot(root, Variable_Array);

    return kSuccess;

}
static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
                                      FILE *out, DumpInfo *DumpInfo, DiffModes ans) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    char var[MAX_TEXT_SIZE] = {};
    size_t amount_of_dif = 0;
    ReadDerivativeParameters(var, &amount_of_dif);

    Forest forest = {};
    ForestCtor(&forest, amount_of_dif);
    
    DifRoot root2 = {};
    DifRootCtor(&root2);
    //NodeCtor(&root2.root, 0);
    DifErrors err = DoNDif(&forest, root, &root2, amount_of_dif, out, DumpInfo, var);
    if (err != kSuccess) {
        ForestDtor(&forest);
        return err;
    }

    if (ans == kDerivativeInPos) {
        //printf()
        DoDerivativeInPos(&root2, Variable_Array, DumpInfo, out, amount_of_dif, var);
    }

    printf("Скорее смотрите ДАМП и ТЕХ!!!\n");
    ForestDtor(&forest);
    return kSuccess;
}

static void ReadDerivativeParameters(char *main_var, size_t *amount_of_dif) {
    assert(main_var);
    assert(amount_of_dif);

    printf(MAGENTA "Введите, по какой переменной вы хотите посчитать производную:\n" RESET);
    scanf("%s", main_var);
    printf(MAGENTA "Введите, какую производную вы хотите посчитать:\n" RESET);
    scanf("%zu", amount_of_dif);
}

static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out) {
    assert(root);
    assert(Variable_Array);
    assert(out);

    double res = SolveEquation(root);
    printf("Результат вычисления выражения: %lf\n", res);
    PrintSolution(root->root, res, out, Variable_Array);
    return kSuccess;
}

void PrintExpressionResultToFile(FILE *out, DifRoot *root, const char *main_var) {
    assert(out);
    assert(root);
    assert(main_var);

    DifNode_t *node = root->root;

    DifNode_t *node_var = NULL;
    FindMainVar(node, main_var, &node_var);
    double copied_value = node_var->value.variable->variable_value;

    for (double i = -50.0; i < 50.0; i += 0.5) {
        node_var->value.variable->variable_value = i;
        fprintf(out, "%f ", i);
        fprintf(out, "%f\n", SolveEquation(root));
    }
    node_var->value.variable->variable_value = copied_value;
}

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifRoot *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var) {
    assert(forest);
    assert(root);
    assert(out);
    assert(DumpInfo);
    assert(main_var);

    DifRoot *node_to_dif = root;

    for (size_t i = 1; i <= ans; i++) {
        fprintf(out, "\n\n\\textbf{Посчитаем %zu производную:}\n\n", i);
        DifNode_t *new_tree = Dif(&forest->trees[i - 1], node_to_dif->root, main_var, out);
        if (!new_tree) {
            fprintf(stderr, "Dif failed on derivative %zu\n", i);
            ForestDtor(forest);
            return kFailure;
        }
        forest->trees[i - 1].root = new_tree;
        root2->root = forest->trees[i - 1].root;

        DumpInfo->tree = &forest->trees[i - 1];
        snprintf(DumpInfo->message, MAX_TEXT_SIZE, " Do (%zu) derivative", i);
        DoTreeInGraphviz(root2->root, DumpInfo, root2->root);
        DoDump(DumpInfo);
        DoTex(forest->trees[i - 1].root, main_var, out);

        forest->trees[i - 1].root = OptimiseTree(&forest->trees[i - 1], forest->trees[i - 1].root, out, main_var);
        DoTreeInGraphviz(forest->trees[i - 1].root, DumpInfo, forest->trees[i - 1].root);
        snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Optimised tree after counting (%zu) derivative", i);
        DoDump(DumpInfo);

        root2->root = forest->trees[ans-1].root;
        node_to_dif = &forest->trees[i - 1];
    }

    return kSuccess;
}

static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var) {
    assert(root2);
    assert(Variable_Array);
    assert(DumpInfo);
    assert(out);

    ReadVariableValue(Variable_Array);
    double res = SolveEquation(root2);
    printf("Результат вычисления выражения значения %zu производной: %lf\n", amount_of_dif, res);

    PrintSolution(root2->root, res, out, Variable_Array);
    strcpy(DumpInfo->message, " Calculate expression in a position in derived expression");
    DoTreeInGraphviz(root2->root, DumpInfo, root2->root);
    PrintExpressionResultToFile(out, root2, main_var);
    DoDump(DumpInfo);
}

static DifErrors DoGnuplot(DifRoot *root, VariableArr *Variable_Array) {
    assert(root);
    assert(Variable_Array);

    if (Variable_Array->size != 1) {
        fprintf(stderr, "Невозможно построить граф, так как переменная не 1.\n");
        return kFailure;
    }
    const char *main_var = Variable_Array->var_array[0].variable_name;

    FILE_OPEN_AND_CHECK(gnuplotfile, "gnuplot.txt", "w");
    PrintExpressionResultToFile(gnuplotfile, root, main_var);
    fclose(gnuplotfile);

    DoSystemForGnuplot(main_var);
    printf("Отлично, смотрите график функции в файле %s.\n", "my_points_plot.png");

    return kSuccess;
}

static void DoSystemForGnuplot(const char *main_var) {
    assert(main_var);

    char *command = (char *) calloc (1000, sizeof(char));

    snprintf(command, 1000, 
    "gnuplot -e \""
    "set terminal pngcairo size 800,600;"
    "set output 'my_points_plot.png';"
    "set grid;"
    "set xlabel '%s';"
    "set ylabel 'Y';"
    "plot 'gnuplot.txt' using 1:2 with linespoints pointtype 7 pointsize 1.5;"
    "set output\"",
    main_var);

    system(command);
    free(command);
}

static FILE *SelectInputFile(void) {
    printf(MAGENTA "Вы хотите ввести выражение из консоли или из файла?\n"
           "1. Консоль,\n"
           "2. Файл:\n" RESET);

    int ans = 0;
    scanf("%d", &ans);

    if (ans == 1) {
        return stdin;
    }

    if (ans == 2) {
        const char *filename = AskFilename();
        FILE *file = fopen(filename, "r");
        return file;
    }

    fprintf(stderr, "Кажется, вы ввели невалидную букву.\n");
    return NULL;
}

static const char *AskFilename(void) {
    printf(MAGENTA "Введите название файла, из которого нужно считать выражение:" RESET);
    char filename[MAX_TEXT_SIZE] = {};
    scanf("%s", filename);
    const char *filename_out = filename;

    return filename_out;
}