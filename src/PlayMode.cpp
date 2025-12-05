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

#define MAX_COMMAND_SIZE 10000

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifRoot *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var, VariableArr *Variable_Array);
static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var, Forest *forest, Positions *positions);

static DifErrors PrintGnuplot(const char *filename, DifRoot *root, const char *main_var, double border1, double border2);
static DifErrors DoGnuplot(DifRoot *root_written,  VariableArr *Variable_Array, Forest *forest, FILE *out, Positions *positions);
static DifErrors PrintGnuplotInPos(const char *filename, DifRoot *root, const char *main_var, double border_1, double border_2);

static void DoSystemForGnuplot(DifRoot *root, const char *main_var, Positions *position);

static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest, char *main_var, Positions *positions);
static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out, Positions *positions);

static DifErrors DoTaylor(Forest *forest, DifRoot *root, DumpInfo *DumpInfo, FILE *out, VariableArr *Variable_Array, Positions *positions);
static DifRoot *CountTaylor(Forest *forest, const char *main_var, size_t number, double num_pos, VariableArr *Variable_Array);
static void ReadDerivativeParameters(char *var, size_t *amount_of_dif);

static DiffModes PrintMenuAndAskMode(void);
static DifErrors DivideChoice(DiffModes ans, DifRoot *root, VariableArr *Variable_Array,
    FILE *out, DumpInfo *DumpInfo, bool *flag_end, Forest *forest, char *string);

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest, char *string);

static DifNode_t *BuildTangent(Forest *forest, DifRoot *root, DifRoot *derivative, double pos, const char *main_var, VariableArr *Variable_Array);
static Positions ReadPositions(char *string);
static void ScanfTwoPositions(double *pos1, double *pos2, char **string);

static size_t ReadTaylorParams(char *main_var, double *number,
    VariableArr *Variable_Array, double *position_taylor);

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *Dump_Info, char *string) {
    assert(Variable_Array);
    assert(root);
    assert(out);
    assert(Dump_Info);

    DifErrors err = kSuccess;

    Forest forest = {};
    ForestCtor(&forest, 3);
    forest.trees[0] = *root;

    bool flag_end = false;
    while (!flag_end) {
        DiffModes ans = PrintMenuAndAskMode();

        err = DivideChoice(ans, root, Variable_Array, out, Dump_Info, &flag_end, &forest, string);
        if (err != kSuccess) return err;
    }

    ForestDtor(&forest);
    return kSuccess;
}

static DiffModes PrintMenuAndAskMode(void) {
    printf(CYAN "Введите, что вы хотите сделать с введенным выражением:\n"
        YELLOW "1." RESET " Посчитать все, \n"
        YELLOW "2." RESET " Посчитать (н-ную) производную,\n"
        YELLOW "3." RESET " Посчитать значение (н-ной) производной в точке,\n" 
        YELLOW "4." RESET " Посчитать значение выражения,\n"
        YELLOW "5." RESET " Разложить по формуле Тейлора,\n"
        YELLOW "6." RESET " Построить график,\n"
        YELLOW "7." RESET " Выйти:\n");

    DiffModes ans = kDerivative;
    scanf("%d", &ans);
    return ans;
}

static DifErrors DivideChoice(DiffModes ans, DifRoot *root, VariableArr *Variable_Array,
    FILE *out, DumpInfo *DumpInfo, bool *flag_end, Forest *forest, char *string) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);
    assert(flag_end);

    Positions positions = {};
    char *main_var = positions.main_var;

    switch (ans) {
        case kAll:
            return DoAllOptions(root, Variable_Array, out, DumpInfo, ans, forest, string);

        case kDerivativeInPos:
        case kDerivative:
            return DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest, main_var, &positions);
            
        case kCount:
            return DoCountCase(root, Variable_Array, out, &positions);
            
        case kGraph: 
            return kSuccess;
            
        case kExit:
            *flag_end = true;
            return kSuccess;
            
        case kTeilor:  {     
            return kSuccess;
        }

        default:
            fprintf(stderr, "Неизвестная команда.\n");
            return kFailure;
    }
}

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
        FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest, char *string) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    ans = kDerivative;

    Positions positions = ReadPositions(string);
    char *main_var = positions.main_var;

    DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest, main_var, &positions);
    ans = kDerivativeInPos;
    DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest, main_var, &positions);
    DoCountCase(root, Variable_Array, out, &positions);

  
    DoTaylor(forest, root, DumpInfo, out, Variable_Array, &positions);
    BuildTangent(forest, root, &forest->trees[1], positions.taylor_pos, main_var, Variable_Array);
    DoGnuplot(root, Variable_Array, forest, out, &positions);

    PrintAllResults(forest, out, positions.taylor_pos, main_var);

    printf(GREEN "Скорее смотрите ДАМП и ТЕХ!!!\n" RESET);

    return kSuccess;

}
static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
        FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest, char *main_var, Positions *positions) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    size_t amount_of_dif = positions->simple_der_pos;
    //ReadDerivativeParameters(main_var, &amount_of_dif);

    
    DifRoot root_last = {};
    DifRootCtor(&root_last);

    DifErrors err = DoNDif(forest, root, &root_last, amount_of_dif, out, DumpInfo, main_var, Variable_Array);
    if (err != kSuccess) {
        ForestDtor(forest);
        return err;
    }

    if (ans == kDerivativeInPos) {
        DoDerivativeInPos(&root_last, Variable_Array, DumpInfo, out, amount_of_dif, main_var, forest, positions);
    }
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

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifRoot *root_last, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var, VariableArr *Variable_Array) {
    assert(forest);
    assert(root);
    assert(out);
    assert(DumpInfo);
    assert(main_var);

    DifNode_t *new_tree = NULL;

    ResizeForest(forest, ans + 1);
    for (size_t i = 1; i <= ans; i++) {
        if (!forest->trees[i].root) {
            printf("%zu\n", i);
            fprintf(out, "\\clearpage\\section{Дифференцируем %zu раз}\n\n\\noindent\n", i);
            new_tree = Dif(&forest->trees[i], forest->trees[i-1].root, main_var, out, Variable_Array);
            if (!new_tree) {
                fprintf(stderr, "Dif failed on derivative %zu\n", i);
                ForestDtor(forest);
                return kFailure;
            }
            forest->trees[i].root = new_tree;

            DumpInfo->tree = &forest->trees[i];
            snprintf(DumpInfo->message, MAX_TEXT_SIZE, " Do (%zu) derivative", i);
            //DoTreeInGraphviz(forest->trees[i].root, DumpInfo, forest->trees[i].root);
            //DoDump(DumpInfo);
            //DoTex(forest->trees[i].root, main_var, out);

            fprintf(out, "\n\\subsection{Попробуем упростить выражение}\n\n\\noindent\n");
            forest->trees[i].root = OptimiseTree(&forest->trees[i], forest->trees[i].root, out, main_var);
            //DoTreeInGraphviz(forest->trees[i].root, DumpInfo, forest->trees[i].root);
            snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Optimised tree after counting (%zu) derivative", i);
            //DoDump(DumpInfo);

            fprintf(out, "\n\\vspace{1em}\\text{Изначальное выражение:}\n\\begin{math}");
            DoTexInner(forest->trees[0].root, out);
            fprintf(out, "\n\\end{math}\n\n");
            fprintf(out, "\n\\vspace{1em}\\text{%zu производная:}\n\n\\begin{math}\n", i);
            DoTexInner(forest->trees[i].root, out);
            fprintf(out, "\n\\end{math}\n\n");
        }
    }

    root_last->root = forest->trees[ans].root;

    return kSuccess;
}

static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var, Forest *forest, Positions *positions) {
    assert(root2);
    assert(Variable_Array);
    assert(DumpInfo);
    assert(out);

    //ReadVariableValue(Variable_Array);
    Variable_Array->var_array[0].variable_value = positions->der_solve_pos;
    double res = SolveEquation(root2);
    printf(YELLOW "Результат вычисления выражения значения %zu производной: %lf\n" RESET, amount_of_dif, res);


    PrintSolutionForDerivative(root2->root, amount_of_dif, res, out, Variable_Array);
    strcpy(DumpInfo->message, " Calculate expression in a position in derived expression");
    //DoTreeInGraphviz(root2->root, DumpInfo, root2->root);
    //DoDump(DumpInfo);
}

static DifErrors DoGnuplot(DifRoot *root_written,  VariableArr *Variable_Array, Forest *forest, FILE *out, Positions *positions) {
    assert(root_written);
    assert(Variable_Array);
    assert(forest);
    assert(out);

    if (Variable_Array->size != 1) {
        fprintf(stderr, "Невозможно построить граф, так как переменная не 1.\n");
        return kFailure;
    }
    const char *main_var = Variable_Array->var_array[0].variable_name;

    // Positions positions = ReadPositions(string);

    PrintGnuplot("./data/gnuplot1.txt", root_written, main_var, positions->x_left_1, positions->x_right_1);
    PrintGnuplot("./data/gnuplot2.txt", &forest->trees[1], main_var, positions->x_left_2, positions->x_right_2);
    PrintGnuplot("./data/gnuplot3.txt", &forest->trees[forest->size - 2], main_var, positions->x_left_3, positions->x_right_3);
    PrintGnuplot("./data/gnuplot4.txt", &forest->trees[forest->size - 1], main_var, positions->x_left_3, positions->x_right_3);

    DoSystemForGnuplot(&forest->trees[forest->size - 2], main_var, positions);
    
    UploadGraph(out);
    PrintColoredNaming(out, root_written->root, "red", "Красный");
    PrintColoredNaming(out, forest->trees[1].root, "green", "Зеленый");
    PrintColoredNaming(out, forest->trees[forest->size - 2].root, "blue", "Голубой");
    
    return kSuccess;
}

static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out, Positions *positions) {
    assert(root);
    assert(Variable_Array);
    assert(out);

    //ReadVariableValue(Variable_Array);
    Variable_Array[0].var_array->variable_value = positions->solve_pos;
    double res = SolveEquation(root);
    printf("Результат вычисления выражения: %lf\n", res);
    PrintSolution(root->root, res, out, Variable_Array);
    return kSuccess;
}

// static DifErrors PrintGnuplotInPos(const char *filename, DifRoot *root, const char *main_var, double border_1, double border_2) {
//     assert(filename);
//     assert(root);
//     assert(main_var);

//     FILE_OPEN_AND_CHECK(out, filename, "w");

//     DifNode_t *node = root->root;

//     DifNode_t *node_var = NULL;
//     FindMainVar(node, main_var, &node_var);
//     if (!node_var) {
//         fprintf(stderr, "Error: variable '%s' not found in expression tree!\n", main_var);
//         return kFailure;
//     }
//     double copied_value = node_var->value.variable->variable_value;


//     for (double i = border_1; i < border_2; i += 0.001) {
//         node_var->value.variable->variable_value = i;
//         fprintf(out, "%f ", i);
//         fprintf(out, "%f\n", SolveEquation(root)); //
//     }
//     node_var->value.variable->variable_value = copied_value;
//     fclose(out);

//     return kSuccess;
// }

static DifErrors PrintGnuplot(const char *filename, DifRoot *root, const char *main_var, double border1, double border2) {
    assert(filename);
    assert(root);
    assert(main_var);

    FILE_OPEN_AND_CHECK(gnuplotfile, filename, "w");

    PrintExpressionResultToFile(gnuplotfile, root, main_var, border1, border2);
    fclose(gnuplotfile);

    return kSuccess;
}

void PrintExpressionResultToFile(FILE *out, DifRoot *root, const char *main_var, double border1, double border2) {
    assert(out);
    assert(root);
    assert(main_var);

    DifNode_t *node = root->root;

    DifNode_t *node_var = NULL;
    FindMainVar(node, main_var, &node_var);
    if (!node_var) {
        fprintf(stderr, "Error: variable '%s' not found in expression tree!\n", main_var);
        return;
    }
    double copied_value = node_var->value.variable->variable_value;

    for (double i = border1; i < border2; i += 0.0001) {
        node_var->value.variable->variable_value = i;
        fprintf(out, "%f ", i);
        fprintf(out, "%f\n", SolveEquation(root));
    }
    node_var->value.variable->variable_value = copied_value;
}

static DifErrors DoTaylor(Forest *forest, DifRoot *root, DumpInfo *DumpInfo, FILE *out, VariableArr *Variable_Array, Positions *positions) {
    assert(forest);
    assert(root);
    assert(DumpInfo);
    assert(out);
    assert(positions);

    char *main_var = positions->main_var;
    double position_taylor = positions->taylor_pos;
    size_t num_pos = positions->taylor_der;
    Variable_Array[0].var_array->variable_value = position_taylor;
    //printf("!!!%lf %zu\n", position_taylor, num_pos);

    //size_t num_pos = ReadTaylorParams(main_var, &number, Variable_Array, position_taylor);


    DifRoot root_last = {};
    fprintf(out, "\\clearpage\n\\section{Разложим по формуле Тейлора в окрестности точки %lf}\n\n\\noindent\n", position_taylor);
    assert(DumpInfo);
    DoNDif(forest, root, &root_last, num_pos, out, DumpInfo, main_var, Variable_Array);
    DifRoot *new_root = CountTaylor(forest, main_var, num_pos, position_taylor, Variable_Array);
    new_root->root = OptimiseTree(new_root, new_root->root, out, main_var);

    ResizeForest(forest, forest->size + 1);
    forest->trees[forest->size - 1] = *new_root;

    DoTreeInGraphviz(new_root->root, DumpInfo, new_root->root);
    snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Taylor polinomial");
    DumpInfo->tree = new_root;
    //DoDump(DumpInfo);
    PrintTaylor(new_root->root, main_var, position_taylor, num_pos, out);

    return kSuccess;
}

static double Factorial(size_t n) {
    double res = 1.0;
    for (size_t i = 2; i <= n; i++) {
        res *= (double) i;
    }

    return res;
}

#define NEWN(num) NewNode(root, kNumber, (Value){ .number = num}, NULL, NULL, Variable_Array)
#define NEWV(var) NewVariable(root, var, Variable_Array)
#define ADD_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationAdd}, left, right, Variable_Array)
#define SUB_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationSub}, left, right, Variable_Array)
#define MUL_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationMul}, left, right, Variable_Array) 
#define DIV_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationDiv}, left, right, Variable_Array) 
#define POW_(left, right) NewNode(root, kOperation, (Value){ .operation = kOperationPow}, left, right, Variable_Array)

static DifRoot *CountTaylor(Forest *forest, const char *main_var, size_t number, double num_pos, VariableArr *Variable_Array) {
    assert(forest);
    assert(main_var);
    assert(Variable_Array);

    DifRoot *taylor_root = (DifRoot *) calloc (1, sizeof(DifRoot));
    if (!taylor_root) return NULL;
    DifRootCtor(taylor_root);

    DifRoot *root = &forest->trees[0];

    DifNode_t *result = NEWN(0);
    taylor_root->root = result;
    

    for (size_t i = 0; i <= number; i++) {
        printf("%zu", i);
        root = &forest->trees[i];

        double ans = SolveEquation(root);
        ans /= Factorial(i);

        DifNode_t *term = MUL_(NEWN(ans), POW_(SUB_(NEWV(main_var), NEWN(num_pos)), NEWN((double)i)));

        result = ADD_(result, term);
    }

    taylor_root->root = result;
    return taylor_root;
}

static void DoSystemForGnuplot(DifRoot *root, const char *main_var, Positions *positions) {
    assert(root);
    assert(main_var);
    assert(positions);

    char command1[MAX_COMMAND_SIZE] = {};
    snprintf(command1, sizeof(command1),
        "gnuplot -e \""
        "set terminal pngcairo size 1800,800;"
        "set output './data/plot1.png';"
        "set grid;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set xrange[%lf:%lf];"
        "set yrange[%lf:%lf];"
        "set title 'Function';"
        "plot "
        "'./data/gnuplot1.txt' using 1:2 with linespoints lc rgb 'red' lw 2 pt 5 ps 0.5 title 'Function'; "
        "\"",
        main_var, positions->x_left_1, positions->x_right_1, positions->y_bottom_1, positions->y_top_1
    );

    char command2[MAX_COMMAND_SIZE] = {};
    snprintf(command2, sizeof(command2),
        "gnuplot -e \""
        "set terminal pngcairo size 1800,800;"
        "set output './data/plot2.png';"
        "set grid;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set xrange[%lf:%lf];"
        "set yrange[%lf:%lf];"
        "set title 'First Derivative';"
        "plot "
        "'./data/gnuplot2.txt' using 1:2 with linespoints lc rgb 'green' lw 2 pt 9 ps 0.5 title '';"
        "\"",
        main_var, positions->x_left_2, positions->x_right_2, positions->y_bottom_2, positions->y_top_2
    );

    char command3[MAX_COMMAND_SIZE] = {};
    snprintf(command3, sizeof(command3),
        "gnuplot -e \""
        "set terminal pngcairo size 1800,800;"
        "set output './data/plot_taylor.png';"
        "set grid;"
        "set size ratio 0.4;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set xrange[%lf:%lf];"
        "set yrange[%lf:%lf];"
        "set title 'Taylor comparison';"
        "plot "
        "'./data/gnuplot1.txt' using 1:2 with linespoints lc rgb 'red' lw 2 pt 5 ps 0.5 title 'function', "
        "'./data/gnuplot3.txt' using 1:2 with linespoints lc rgb 'blue' lw 2 pt 7 ps 0.5 title 'taylor polinomial',"
        "'./data/gnuplot4.txt' using 1:2 with linespoints lc rgb 'orange' lw 1 pt 3 ps 0.5 title 'tangent',"
        "'+' using (%lf):(%lf) with point lc rgb 'green' pt 7 ps 2 title 'position';"
        "\"",
        main_var, positions->x_left_3, positions->x_right_3, positions->y_bottom_3, positions->y_top_3, positions->taylor_pos, SolveEquation(root)
    );


    int result1 = system(command1);
    int result2 = system(command2);
    int result3 = system(command3);

    if (result1 != 0) {
        fprintf(stderr, "Failed to run gnuplot command 1.\n");
    }
    if (result2 != 0) {
        fprintf(stderr, "Failed to run gnuplot command 2.\n");
    }
    if (result3 != 0) {
        fprintf(stderr, "Failed to run gnuplot command 3.\n");
    }
}

static DifNode_t *BuildTangent(Forest *forest, DifRoot *root, DifRoot *derivative, double pos, const char *main_var, VariableArr *Variable_Array) {
    assert(root);

    double derivative_ans = SolveEquation(derivative);

    ResizeForest(forest, forest->size + 1);
    DifNode_t *new_node = ADD_(MUL_(NEWN(derivative_ans), SUB_(NEWV(main_var), NEWN(pos))), NEWN(SolveEquation(root)));

    forest->trees[forest->size - 1].root = new_node;
    return new_node;
}

#undef NEWN_
#undef NEWV_
#undef ADD_
#undef SUB_
#undef DIV_
#undef MUL_
#undef POW_

static size_t ReadTaylorParams(char *main_var, double *number,
    VariableArr *Variable_Array, double *position_taylor) {
    assert(main_var);
    assert(number);
    assert(Variable_Array);
    assert(position_taylor);

    printf(BLUE "По какой переменной хотите разложить по Тейлору:\n" RESET);
    scanf("%s", main_var);

    printf(BLUE "В окрестности какой точки хотите разложить:\n" RESET);
    scanf("%lf", number);

    for (size_t i = 0; i < Variable_Array->size; i++) {
        if (strcmp(main_var, Variable_Array->var_array[i].variable_name) == 0) {
            Variable_Array->var_array[i].variable_value = *number;
        }
    }

    *position_taylor = *number;

    size_t num_pos = 0;
    printf(BLUE "Для какой производной вы хотите посчитать:\n" RESET);
    scanf("%zu", &num_pos);

    return num_pos;
}


static Positions ReadPositions(char *string) {
    assert(string);

    Positions positions = {};

    int cnt = 0;
    sscanf(string, "%s%n", positions.main_var, &cnt);
    (string) += cnt;

    sscanf(string, "%zu%n", &positions.simple_der_pos, &cnt);
    (string) += cnt;
//     int n = sscanf(string, "%zu%n", &positions.simple_der_pos, &cnt);
// printf("sscanf returned %d, cnt=%d, remaining: '%s'\n", n, cnt, string);

    ScanfTwoPositions(&positions.der_in_pos, &positions.der_solve_pos, &string);

    sscanf(string, "%lf%n", &positions.solve_pos, &cnt);
    (string) += cnt;

    sscanf(string, "%lf %zu%n", &positions.taylor_pos,  &positions.taylor_der, &cnt);
    (string) += cnt;

    ScanfTwoPositions(&positions.x_left_1, &positions.x_right_1, &string);
    ScanfTwoPositions(&positions.y_bottom_1, &positions.y_top_1, &string);
    ScanfTwoPositions(&positions.x_left_2, &positions.x_right_2, &string);
    ScanfTwoPositions(&positions.y_bottom_2, &positions.y_top_2, &string);
    ScanfTwoPositions(&positions.x_left_3, &positions.x_right_3, &string);
    ScanfTwoPositions(&positions.y_bottom_3, &positions.y_top_3, &string);

    return positions;
}

static void ScanfTwoPositions(double *pos1, double *pos2, char **string) {
    assert(pos1);
    assert(pos2);
    assert(string);

    int cnt = 0;
    sscanf(*string, "%lf %lf%n", pos1, pos2, &cnt);
    (*string) += cnt;
}