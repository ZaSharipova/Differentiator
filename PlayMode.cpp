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

static DifErrors DoNDif(Forest *forest, DifRoot *root, DifRoot *root2, size_t ans, FILE *out, DumpInfo *DumpInfo, const char *main_var, VariableArr *Variable_Array);
static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var, Forest *forest);

static DifErrors PrintGnuplot(const char *filename, DifRoot *root, const char *main_var);
static DifErrors DoGnuplot(DifRoot *root_written,  VariableArr *Variable_Array, Forest *forest, FILE *out);

static void DoSystemForGnuplot(const char *main_var);

static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest);
static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out);

static DifErrors DoTaylor(Forest *forest, DifRoot *root, DumpInfo *DumpInfo, FILE *out, VariableArr *Variable_Array);
static DifRoot *CountTaylor(Forest *forest, const char *main_var, size_t number, double num_pos, VariableArr *Variable_Array);
static void ReadDerivativeParameters(char *var, size_t *amount_of_dif);

static DiffModes PrintMenuAndAskMode(void);
static DifErrors DivideChoice(DiffModes ans, DifRoot *root, VariableArr *Variable_Array,
    FILE *out, DumpInfo *DumpInfo, bool *flag_end, Forest *forest);

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
    FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest);

static const char *AskFilename(void);

DifErrors DiffPlay(VariableArr *Variable_Array, DifRoot *root, FILE *out, DumpInfo *Dump_Info) {
    assert(Variable_Array);
    assert(root);
    assert(out);
    assert(Dump_Info);

    DifErrors err = kSuccess;
    // CHECK_ERROR_RETURN(VerifyTree(root));

    Forest forest = {};
    ForestCtor(&forest, 3);
    forest.trees[0] = *root;

    bool flag_end = false;
    while (!flag_end) {
        DiffModes ans = PrintMenuAndAskMode();

        err = DivideChoice(ans, root, Variable_Array, out, Dump_Info, &flag_end, &forest);
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
    FILE *out, DumpInfo *DumpInfo, bool *flag_end, Forest *forest) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);
    assert(flag_end);

    switch (ans) {
        case kAll:
            return DoAllOptions(root, Variable_Array, out, DumpInfo, ans, forest);

        case kDerivativeInPos:
        case kDerivative:
            return DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest);
            
        case kCount:
            return DoCountCase(root, Variable_Array, out);
            
        case kGraph: //TODO 
            // return DoGnuplot(root, Variable_Array, forest);
            return kSuccess;
            
        case kExit:
            *flag_end = true;
            return kSuccess;
            
        case kTeilor:  {    
            DoTaylor(forest, root, DumpInfo, out, Variable_Array);      
            return kSuccess;
        }
            
        default:
            fprintf(stderr, "Неизвестная команда.\n");
            return kFailure;
    }
}

static DifErrors DoAllOptions(DifRoot *root, VariableArr *Variable_Array, 
        FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    ans = kDerivative;
    DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest);
    ans = kDerivativeInPos;
    DoDerivativeCases(root, Variable_Array, out, DumpInfo, ans, forest);
    DoCountCase(root, Variable_Array, out);
    DoTaylor(forest, root, DumpInfo, out, Variable_Array);
    DoGnuplot(root, Variable_Array, forest, out);

    printf(GREEN "Скорее смотрите ДАМП и ТЕХ!!!\n" RESET);

    return kSuccess;

}
static DifErrors DoDerivativeCases(DifRoot *root, VariableArr *Variable_Array, 
        FILE *out, DumpInfo *DumpInfo, DiffModes ans, Forest *forest) {
    assert(ans);
    assert(root);
    assert(Variable_Array);
    assert(out);
    assert(DumpInfo);

    char var[MAX_TEXT_SIZE] = {};
    size_t amount_of_dif = 0;
    ReadDerivativeParameters(var, &amount_of_dif);

    ResizeForest(forest, amount_of_dif + 1);
    
    DifRoot root_last = {};
    DifRootCtor(&root_last);

    DifErrors err = DoNDif(forest, root, &root_last, amount_of_dif, out, DumpInfo, var, Variable_Array);
    if (err != kSuccess) {
        ForestDtor(forest); //
        return err;
    }

    if (ans == kDerivativeInPos) {
        DoDerivativeInPos(&root_last, Variable_Array, DumpInfo, out, amount_of_dif, var, forest);
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

    //DifRoot *node_to_dif = root;
    DifNode_t *new_tree = NULL;

    for (size_t i = 1; i <= ans; i++) {
        //fprintf(out, "\n\n\\textbf{Посчитаем %zu производную:}\n\n", i);
        if (!forest->trees[i].root) {
            fprintf(out, "\\clearpage\\section{Дифференцируем %zu раз}\n\n\\noindent\n", i);
            new_tree = Dif(&forest->trees[i], forest->trees[i-1].root, main_var, out, Variable_Array);
            if (!new_tree) {
                fprintf(stderr, "Dif failed on derivative %zu\n", i);
                ForestDtor(forest);
                return kFailure;
            }
            forest->trees[i].root = new_tree;
            //root_last->root = forest->trees[i].root;

            DumpInfo->tree = &forest->trees[i];
            snprintf(DumpInfo->message, MAX_TEXT_SIZE, " Do (%zu) derivative", i);
            DoTreeInGraphviz(forest->trees[i].root, DumpInfo, forest->trees[i].root);
            DoDump(DumpInfo);
            DoTex(forest->trees[i].root, main_var, out);

            fprintf(out, "\n\\subsection{Попробуем упростить выражение}\n\n\\noindent\n");
            forest->trees[i].root = OptimiseTree(&forest->trees[i], forest->trees[i].root, out, main_var);
            DoTreeInGraphviz(forest->trees[i].root, DumpInfo, forest->trees[i].root);
            snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Optimised tree after counting (%zu) derivative", i);
            DoDump(DumpInfo);
        }
    }

    root_last->root = forest->trees[ans].root;

    return kSuccess;
}

static void DoDerivativeInPos(DifRoot *root2, VariableArr *Variable_Array, DumpInfo *DumpInfo, FILE *out, size_t amount_of_dif, const char *main_var, Forest *forest) {
    assert(root2);
    assert(Variable_Array);
    assert(DumpInfo);
    assert(out);

    ReadVariableValue(Variable_Array);
    double res = SolveEquation(root2);
    printf(YELLOW "Результат вычисления выражения значения %zu производной: %lf\n" RESET, amount_of_dif, res);


    PrintSolutionForDerivative(root2->root, amount_of_dif, res, out, Variable_Array);
    strcpy(DumpInfo->message, " Calculate expression in a position in derived expression");
    DoTreeInGraphviz(root2->root, DumpInfo, root2->root);
    DoDump(DumpInfo);
}

static DifErrors DoGnuplot(DifRoot *root_written,  VariableArr *Variable_Array, Forest *forest, FILE *out) {
    assert(root_written);
    assert(Variable_Array);
    assert(forest);
    assert(out);

    if (Variable_Array->size != 1) {
        fprintf(stderr, "Невозможно построить граф, так как переменная не 1.\n");
        return kFailure;
    }
    const char *main_var = Variable_Array->var_array[0].variable_name;

    PrintGnuplot("gnuplot1.txt", root_written, main_var);
    PrintGnuplot("gnuplot2.txt", &forest->trees[1], main_var);
    PrintGnuplot("gnuplot3.txt", &forest->trees[forest->size - 1], main_var);


    DoSystemForGnuplot(main_var);
    
    UploadGraph(out);
    fprintf(out, "\n\\textcolor{red}{Красный:} \n\\begin{dmath*}");
    DoTexInner(root_written->root, out);
    fprintf(out, "\\end{dmath*}\n\n");
    fprintf(out, "\\textcolor{green}{Зеленый:} \n\\begin{dmath*}");
    DoTexInner(forest->trees[1].root, out);
    fprintf(out, "\\end{dmath*}\n\n");
    fprintf(out, "\\textcolor{blue}{Голубой:} \n\\begin{dmath*}");
    DoTexInner(forest->trees[forest->size - 1].root, out);
    fprintf(out, "\\end{dmath*}\n\n");
    
    return kSuccess;
}

static DifErrors DoCountCase(DifRoot *root, VariableArr *Variable_Array, FILE *out) {
    assert(root);
    assert(Variable_Array);
    assert(out);

    ReadVariableValue(Variable_Array);
    double res = SolveEquation(root);
    printf("Результат вычисления выражения: %lf\n", res);
    PrintSolution(root->root, res, out, Variable_Array);
    return kSuccess;
}

static DifErrors PrintGnuplot(const char *filename, DifRoot *root, const char *main_var) {
    assert(filename);
    assert(root);
    assert(main_var);

    FILE_OPEN_AND_CHECK(gnuplotfile, filename, "w");
    PrintExpressionResultToFile(gnuplotfile, root, main_var);
    fclose(gnuplotfile);

    return kSuccess;
}

void PrintExpressionResultToFile(FILE *out, DifRoot *root, const char *main_var) {
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

    for (double i = -10.0; i < 10.0; i += 0.1) {
        node_var->value.variable->variable_value = i;
        fprintf(out, "%f ", i);
        fprintf(out, "%f\n", SolveEquation(root));
    }
    node_var->value.variable->variable_value = copied_value;
}

static DifErrors DoTaylor(Forest *forest, DifRoot *root, DumpInfo *DumpInfo, FILE *out, VariableArr *Variable_Array) {
    assert(forest);
    assert(root);
    assert(DumpInfo);
    assert(out);

    char main_var[MAX_TEXT_SIZE] = {};
    printf(BLUE "По какой переменной хотите разложить по Тейлору:\n" RESET);
    scanf("%s", main_var);
    printf(BLUE "В окрестности какой точки хотите разложить:\n" RESET);
    double number = 0;
    scanf("%lf", &number);
    size_t num_pos = 0;
    printf(BLUE "Для какой производной вы хотите посчитать:\n" RESET);
    scanf("%zu", &num_pos);

    DifRoot root_last = {};
    fprintf(out, "\\clearpage\n\\section{Разложим по формуле Тейлора}\n\n\\noindent\n");
    DoNDif(forest, root, &root_last, num_pos, out, DumpInfo, main_var, Variable_Array);
    DifRoot *new_root = CountTaylor(forest, main_var, num_pos, number, Variable_Array);
    new_root->root = OptimiseTree(new_root, new_root->root, out, main_var);

    ResizeForest(forest, forest->size + 1);
    forest->trees[forest->size - 1] = *new_root;

    DoTreeInGraphviz(new_root->root, DumpInfo, new_root->root);
    snprintf(DumpInfo->message, MAX_TEXT_SIZE, "Taylor polinomial");
    DumpInfo->tree = new_root;
    DoDump(DumpInfo);
    PrintTaylor(new_root->root, main_var, number, num_pos, out);

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
        root = &forest->trees[i];

        double ans = SolveEquation(root);
        ans /= Factorial(i);

        DifNode_t *term = MUL_(NEWN(ans), POW_(SUB_(NEWV(main_var), NEWN(num_pos)), NEWN(i)));

        result = ADD_(result, term);
    }

    taylor_root->root = result;
    return taylor_root;
}

#undef NEWN_
#undef NEWV_
#undef ADD_
#undef SUB_
#undef DIV_
#undef MUL_
#undef POW_

static void DoSystemForGnuplot(const char *main_var) {
    assert(main_var);

    char command1[10000] = {};
    snprintf(command1, sizeof(command1),
        "gnuplot -e \""
        "set terminal pngcairo size 1200,600;"
        "set output 'plot1.png';"
        "set grid;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set title 'Function';"
        "plot "
        "'gnuplot1.txt' using 1:2 with linespoints lc rgb 'red' lw 2 pt 5 ps 0.5 title 'Function'; "
        "\"",
        main_var
    );

    char command2[10000] = {};
    snprintf(command2, sizeof(command2),
        "gnuplot -e \""
        "set terminal pngcairo size 1200,600;"
        "set output 'plot2.png';"
        "set grid;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set title 'First Derivative';"
        "plot "
        "'gnuplot2.txt' using 1:2 with linespoints lc rgb 'green' lw 2 pt 9 ps 0.5 title '';"
        "\"",
        main_var
    );

    char command3[10000] = {};
    snprintf(command3, sizeof(command3),
        "gnuplot -e \""
        "set terminal pngcairo size 1200,600;"
        "set output 'plot_taylor.png';"
        "set grid;"
        "set xlabel '%s';"
        "set ylabel 'Y';"
        "set title 'Taylor comparison';"
        "plot "
        "'gnuplot1.txt' using 1:2 with linespoints lc rgb 'red' lw 2 pt 5 ps 0.5 title 'function', "
        "'gnuplot3.txt' using 1:2 with linespoints lc rgb 'blue' lw 2 pt 7 ps 0.5 title 'taylor polinomial';"
        "\"",
        main_var
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

// static void PrintTreeSimple(DifNode_t *node, char **buffer) {
//     if (!node) {
//         return;
//     }
//     // assert(node);
//     assert(buffer);
//     size_t buf_size = MAX_TEXT_SIZE;

//     switch (node->type) {
//         case kNumber:
//             snprintf(*buffer + strlen(*buffer), buf_size - strlen(*buffer), "%g", node->value.number);
//             break;
//         case kVariable:
//             snprintf(*buffer + strlen(*buffer), buf_size - strlen(*buffer), "%s", node->value.variable->variable_name);
//             break;
//         default:
//             snprintf(*buffer + strlen(*buffer), buf_size - strlen(*buffer), "(");
//             PrintTreeSimple(node->left, buffer);

//             switch (node->value.operation) {
//                 case kOperationAdd: strcat(*buffer, " + "); break;
//                 case kOperationSub: strcat(*buffer, " - "); break;
//                 case kOperationMul: strcat(*buffer, " * "); break;
//                 case kOperationDiv: strcat(*buffer, " / "); break;
//                 case kOperationPow: strcat(*buffer, " ^ "); break;
//                 default: break;
//             }

//             PrintTreeSimple(node->right, buffer);
//             strcat(*buffer, ")");
//             break;
//     }
// }