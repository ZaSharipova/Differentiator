#include "DoTex.h"

#include <stdio.h>
#include <assert.h>
#include "Enums.h"
#include "Structs.h"

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
};
static const int TEX_PHRASES_COUNT = (sizeof(TexPhrasesArray) / sizeof(char*));
size_t pos_in_array = TEX_PHRASES_COUNT;


void DoTexInner(DifNode_t *node, FILE *out) {
    assert(node);
    assert(out);

    if (node->type == kNumber) {
        fprintf(out, "%lg", node->value.number);
        return;
    }

    if (node->type == kVariable) {
        fprintf(out, "%s", node->value.variable->variable_name);
        return;
    }

    switch (node->value.operation) {
        case (kOperationAdd):
            fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " + ");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationSub):
            fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " - ");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case (kOperationMul):
            if (node->left->type == kNumber && node->left->value.number < 0) fprintf(out, "("); //function
            DoTexInner(node->left, out);
            if (node->left->type == kNumber && node->left->value.number < 0) fprintf(out, ")");
            fprintf(out, " \\cdot ");
            if (node->right->type == kNumber && node->right->value.number < 0) fprintf(out, "(");
            DoTexInner(node->right, out);
            if (node->right->type == kNumber && node->right->value.number < 0) fprintf(out, ")");
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

void BeginTex(FILE *out, DifNode_t *node) {
    assert(out);
    assert(node);

    fprintf(out, "\\documentclass{article}\n");
    fprintf(out, "\\usepackage{amsmath}\n");
    fprintf(out, "\n\\usepackage[utf8]{inputenc}\n");
    fprintf(out, "\\usepackage[english,russian]{babel}");
    fprintf(out, "\\usepackage{xcolor}\n");
    fprintf(out, "\n\\usepackage{breqn}\n");
    fprintf(out, "\\usepackage[left=2cm, top=2cm, right=2cm, bottom=2cm]{geometry}\n\n");
    fprintf(out, "\\begin{document}\n\\fontsize{7}{9}\\selectfont\n");

    fprintf(out, "Дифференцирование - задача непростая, поэтому этот TEX окажется крайне полезным. \n\n\\vspace{1em}");
    fprintf(out, "Будем работать с таким выражением: \n \\begin{dmath*}");
    DoTexInner(node, out);
    fprintf(out, "\\end{dmath*}\n\n");
}

void EndTex(FILE *out) {
    fprintf(out, "\n\nТеперь страшное слово под названием ДИФФЕРЕНЦИРОВАНИЕ пугает не так сильно.\n\n Смелее закрывайте этот ТЕХ, и будет вам счастье!!\n");
    fprintf(out, "\n\\end{document}\n");
}

void DoTex(DifNode_t *node, const char *value, FILE *out) {
    assert(node);
    assert(value);
    assert(out);

    fprintf(out, "\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    //fprintf(out, "\n\\[\n");
    fprintf(out, "\\begin{dmath*} \\frac{df}{d%s} = ", value);
    DoTexInner(node, out);
    fprintf(out, "\\end{dmath*}\n");
    //if (!is_last) fprintf(out, " \\\\ \n");
}

void PrintSolution(DifNode_t *node, double answer, FILE *out, VariableArr *VariableArr) {
    assert(node);
    assert(out);
    assert(VariableArr);

    fprintf(out, "\n\\text{%s} \n", TexPhrasesArray[(pos_in_array += 1) % TEX_PHRASES_COUNT]);
    if (VariableArr->size > 0) fprintf(out, "при %s = %lf", VariableArr->var_array->variable_name, VariableArr->var_array->variable_value);

    for (size_t i = 1; i < VariableArr->size; i++) {
        fprintf(out, ", %s = %lf", VariableArr->var_array->variable_name, VariableArr->var_array->variable_value);
    }

    fprintf(out, "\n\n\\begin{dmath*}\n");
    DoTexInner(node, out);
    fprintf(out, " = %lf\n\\end{dmath*}\n", answer);
}
