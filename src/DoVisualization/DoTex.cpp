#include "DoTex.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "Enums.h"
#include "Structs.h"

static bool IsNegativeNumber(DifNode_t *node);
void printDouble(FILE *out, double x);

const char *TexPhrasesArray[] = {
    "Путём нетрудных преобразований",
    "Заметим, что",
    "Очевидно, что",
    "Таким образом, получаем",
    "Нетрудно получить, что",
    "После нескольких тривиальных переходов",
    "Проведя мысленный эксперимент, нетрудно убедиться, что",
    "В свете вышесказанного становится ясно, что",
    "Интуитивно понятно, что",
    "Опуская промежуточные преобразования, приходим к",
    "Пристально вглядевшись, можно увидеть, что",
    
    "Следовательно",
    "Отсюда непосредственно",
    "Продолжая упрощения",
    "Упрощая выражение",
    "Без особого труда видим",
    "Легко проверить, что",
    "По очевидным причинам",
    "Естественно следует",
    "Приведём к виду",
    "Проще всего записать",
    "В частности",
    "Из проведённых выкладок",
    "Подставляя значения",
    "С учётом этого",
    "Далее очевидно",
    "Вычислим по частям",
    "Сокращая общие множители",
    "Упростив левую часть"
};

static const int TEX_PHRASES_COUNT = (sizeof(TexPhrasesArray) / sizeof(char*));
size_t pos_in_array = TEX_PHRASES_COUNT;



void BeginTex(FILE *out) {
    assert(out);
    
    fprintf(out, "\\documentclass[12pt]{article}\n");
    fprintf(out, "\\usepackage{amsmath}\n");
    fprintf(out, "\\usepackage{graphicx}\n");
    fprintf(out, "\n\\usepackage[utf8]{inputenc}\n");
    fprintf(out, "\\usepackage[english,russian]{babel}");
    fprintf(out, "\\usepackage{xcolor}\n");
    fprintf(out, "\n\\usepackage{breqn}\n");
    fprintf(out, "\\usepackage[left=2cm, top=2cm, right=2cm, bottom=2cm]{geometry}\n\n");
    fprintf(out, "\\title{<<Дифференцирование головного мозга и матана>>}\n");
    fprintf(out, "\\author{Зарина Шарипова Б05-531}\n\n");

    fprintf(out, "\\begin{document}\n");

    fprintf(out, "\\maketitle\n");
    fprintf(out, "\\centering\\includegraphics[width=0.8\\textwidth]{./data/bibki.jpg}\n\\clearpage");
    fprintf(out, "\n\n\\tableofcontents\n\\clearpage\n");
    fprintf(out, "\\textbf{Дифференцирование - задача непростая, поэтому этот TEX окажется крайне полезным.}\n\n\\vspace{1em}");
}

void EndTex(FILE *out) {
    fprintf(out, "\\clearpage\n");
    fprintf(out, "\n\n\\textbf{Теперь страшное слово под названием ДИФФЕРЕНЦИРОВАНИЕ пугает не так сильно}.\n\n \\textbf{Смелее закрывайте этот ТЕХ, и будет вам счастье!!}\n");
    fprintf(out, "\n\n\\vspace{1em}\n\\centering\\includegraphics[width=0.8\\textwidth]{./data/help.jpg}\n");
    fprintf(out, "\n\\end{document}");

}

void PrintFirstExpression(FILE *out, DifNode_t *node) {
    assert(out);
    assert(node);

    fprintf(out, "\n\nБыло введено такое выражение: \\begin{dmath*}\n\\textcolor{red}{");
    DoTexInner(node, out);
    fprintf(out, "}\n\\end{dmath*}");

    fprintf(out, "\n\n\\vspace{1em}\n\\centering\\includegraphics[width=0.8\\textwidth]{./data/happy_Lukashov.jpg}\n\\clearpage\n");
}

void DoTex(DifNode_t *node, const char *value, FILE *out) {
    assert(node);
    assert(value);
    assert(out);
    
    fprintf(out, "\n\\vspace{1em}\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    
    fprintf(out, "\n\\begin{math} \\frac{df}{d%s} = ", value);
    DoTexInner(node, out);
    fprintf(out, "\\end{math}\n\n");
}

void PrintShrich(DifNode_t *node, DifNode_t *result, FILE *out) {
    assert(node);
    assert(result);
    assert(out);
    
    fprintf(out, "\n\\vspace{1em}\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    fprintf(out, "\n\\begin{math}(");
    DoTexInner(node, out);
    fprintf(out, ")' = ");
    DoTexInner(result, out);
    fprintf(out, "\\end{math}\n");
} 

void PrintTaylor(DifNode_t *node, const char *main_var, double number, size_t num_pos, FILE *out) {
    assert(node);
    assert(main_var);
    assert(out);

    fprintf(out, "\\begin{dmath*} f(%s) = ", main_var);
    DoTexInner(node, out);
    fprintf(out, "\\ + o((x - %.0lf)^%zu) \\end{dmath*}", number, num_pos);
}

void UploadGraph(FILE *out) {
    assert(out);

    fprintf(out, "\n\\clearpage\\section{Чудесные графики чудесных функций!}\n");
    fprintf(out, "\n\n\\includegraphics[width=0.8\\textwidth]{./data/plot1.png}\n");
    fprintf(out, "\n\n\\includegraphics[width=0.8\\textwidth]{./data/plot2.png}\n");
    fprintf(out, "\n\n\\includegraphics[width=0.8\\textwidth]{./data/plot_taylor.png}\n");
}

void PrintSolution(DifNode_t *node, double answer, FILE *out, VariableArr *VariableArr) {
    assert(node);
    assert(out);
    assert(VariableArr);
    
    fprintf(out, "\n\\clearpage\\section{Значение выражения в точке}\n\n");
    fprintf(out, "\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    if (VariableArr->size > 0) fprintf(out, "при %s = %lf", VariableArr->var_array->variable_name, VariableArr->var_array->variable_value);
    
    for (size_t i = 1; i < VariableArr->size; i++) {
        fprintf(out, ", %s = %lf", VariableArr->var_array->variable_name, VariableArr->var_array->variable_value);
    }
    
    fprintf(out, "\n\n\\begin{dmath*}\n");
    DoTexInner(node, out);
    fprintf(out, " = %lf\n\\end{dmath*}\n", answer);
}

void PrintColoredNaming(FILE *out, DifNode_t *node, const char *color, const char *color_rus) {
    assert(node);
    assert(out);
    assert(color);
    assert(color_rus);
    
    fprintf(out, "\n\\textcolor{%s}{%s:} \n\\begin{dmath*}", color, color_rus);
    DoTexInner(node, out);
    fprintf(out, "\\end{dmath*}\n\n");
}

void PrintSolutionForDerivative(DifNode_t *node, size_t num_of_der, double answer, FILE *out, VariableArr *Variable_Array) {
    assert(node);
    assert(out);
    assert(Variable_Array);
    
    fprintf(out, "\n\\clearpage\\section{Значение %zu производной в точке}\n\n", num_of_der);
    fprintf(out, "\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    if (Variable_Array->size > 0) fprintf(out, "при %s = %lf", Variable_Array->var_array->variable_name, Variable_Array->var_array->variable_value);
    
    for (size_t i = 1; i < Variable_Array->size; i++) {
        fprintf(out, ", %s = %lf", Variable_Array->var_array->variable_name, Variable_Array->var_array->variable_value);
    }
    
    fprintf(out, "\n\n\\begin{dmath*}\n");
    DoTexInner(node, out);
    fprintf(out, " = %lf\n\\end{dmath*}\n", answer);
}

void DoTexInner(DifNode_t *node, FILE *out) {
    assert(node);
    assert(out);

    if (node->type == kNumber) {
        printDouble(out, node->value.number);
        return;
    }

    if (node->type == kVariable) {
        fprintf(out, "%s", node->value.variable->variable_name);
        return;
    }

    //fprintf(stderr, "%d ", node->value.operation);
    switch (node->value.operation) {
        case (kOperationAdd):
            if (node->parent && (node->parent->value.operation == kOperationMul || node->parent->value.operation == kOperationDiv
                || node->parent->value.operation == kOperationPow)) fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " + ");
            DoTexInner(node->right, out);
            if (node->parent && (node->parent->value.operation == kOperationMul || node->parent->value.operation == kOperationDiv
                || node->parent->value.operation == kOperationPow)) fprintf(out, ")");
            break;

        case (kOperationSub):
            if (node->parent && (node->parent->value.operation == kOperationMul || node->parent->value.operation == kOperationDiv
                || node->parent->value.operation == kOperationPow)) fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " - ");
            DoTexInner(node->right, out);
            if (node->parent && (node->parent->value.operation == kOperationMul || node->parent->value.operation == kOperationDiv 
                || node->parent->value.operation == kOperationPow)) fprintf(out, ")");
            break;

        case (kOperationMul):
            // if (IsNegativeNumber(node->left) && abs(node->left->value.number + 1) < 1e-11) {
            //     fprintf(out, "-");
            //      if (IsNegativeNumber(node->right)) fprintf(out, "(");
            //     DoTexInner(node->right, out);
            //     if (IsNegativeNumber(node->right)) fprintf(out, ")");
            //     break;
            // }
            if (node->parent && node->parent->value.operation == kOperationPow) fprintf(out, "(");
            if (IsNegativeNumber(node->left)) fprintf(out, "(");
            DoTexInner(node->left, out);
            if (IsNegativeNumber(node->left)) fprintf(out, ")");
            fprintf(out, " \\cdot ");
            if (IsNegativeNumber(node->right)) fprintf(out, "(");
            DoTexInner(node->right, out);
            if (IsNegativeNumber(node->right)) fprintf(out, ")");
            if (node->parent && node->parent->value.operation == kOperationPow) fprintf(out, ")");
            break;

        case (kOperationDiv):
            fprintf(out, "\\frac{");
            DoTexInner(node->left, out);
            fprintf(out, "}{");
            DoTexInner(node->right, out);
            fprintf(out, "}");
            break;

        case (kOperationPow):
            fprintf(out, "{");
            DoTexInner(node->left, out);
            fprintf(out, "}^{");
            DoTexInner(node->right, out);
            fprintf(out, "}");
            break;

        case (kOperationSin):
            fprintf(out, "\\sin(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationCos):
            fprintf(out, "\\cos(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationTg):
            fprintf(out, "\\tan(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationLn):
            fprintf(out, "\\log(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationArctg):
            fprintf(out, "\\arctan(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationSinh):
            fprintf(out, "\\sinh(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;
        case (kOperationCosh):
            fprintf(out, "\\cosh(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;
        case (kOperationTgh):
            fprintf(out, "\\tanh(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationNone):
        default:
            fprintf(out, "?");
            break;
    }
}

static bool IsNegativeNumber(DifNode_t *node) {
    return (node->type == kNumber && node->value.number < 0);
}

void printDouble(FILE *out, double x) {
    assert(out);

    if (fabs(x) < eps) {
        fprintf(out, "0");
        return;
    }

    char buf[64] = {};
    snprintf(buf, sizeof(buf), "%.6f", x);

    char *ptr = buf + strlen(buf) - 1;
    while (*ptr == '0' && ptr > buf) ptr--;

    if (*ptr == '.') ptr--;

    *(ptr + 1) = '\0';

    fprintf(out, "%s", buf);
}
