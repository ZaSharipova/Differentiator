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
    "Нетрудно заметить",
    "После нескольких тривиальных переходов",
};
static const int TEX_PHRASES_COUNT = sizeof(TexPhrasesArray) / sizeof(char*);


void DoTexInner(DifNode_t *node, FILE *out) { // очень плохо, переделать !!!!
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
        case kOperationAdd:
            fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " + ");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationSub:
            fprintf(out, "(");
            DoTexInner(node->left, out);
            fprintf(out, " - ");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationMul:
            DoTexInner(node->left, out);
            fprintf(out, " \\cdot ");
            DoTexInner(node->right, out);
            break;

        case kOperationDiv:
            fprintf(out, "\\frac{");
            DoTexInner(node->left, out);
            fprintf(out, "}{");
            DoTexInner(node->right, out);
            fprintf(out, "}");
            break;

        case kOperationPow:
            fprintf(out, "{");
            DoTexInner(node->left, out);
            fprintf(out, "}^{");
            DoTexInner(node->right, out);
            fprintf(out, "}");
            break;

        case kOperationSin:
            fprintf(out, "\\sin(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationCos:
            fprintf(out, "\\cos(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationTg:
            fprintf(out, "\\tan(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationLn:
            fprintf(out, "\\log(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        case kOperationArctg:
            fprintf(out, "\\arctan(");
            DoTexInner(node->right, out);
            fprintf(out, ")");
            break;

        default:
            fprintf(out, "?");
            break;
    }
}

static void DoTexInnerHighlight(DifNode_t *node, DifNode_t *highlight, FILE *out) {
    assert(node);
    assert(out);
    
    if (node == highlight) {
        fprintf(out, "\\color{red}{(");
        DoTexInner(node, out);
        fprintf(out, ")'}");
        return;
    }

    // DoTexInner(node, out);
    if (node->type == kNumber) {
        fprintf(out, "%lg", node->value.number);
        return;
    }

    if (node->type == kVariable) {
        fprintf(out, "%s", node->value.variable->variable_name);
        return;
    }

    switch (node->value.operation) {
        case kOperationAdd:
            fprintf(out, "\\left(");
            DoTexInnerHighlight(node->left,  highlight, out);
            fprintf(out, " + ");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, "\\right)");
            break;

        case kOperationSub:
            fprintf(out, "\\left(");
            DoTexInnerHighlight(node->left,  highlight, out);
            fprintf(out, " - ");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, "\\right)");
            break;

        case kOperationMul:
            DoTexInnerHighlight(node->left,  highlight, out);
            fprintf(out, " \\cdot ");
            DoTexInnerHighlight(node->right, highlight, out);
            break;

        case kOperationDiv:
            fprintf(out, "\\frac{");
            DoTexInnerHighlight(node->left,  highlight, out);
            fprintf(out, "}{");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, "}");
            break;

        case kOperationPow:
            fprintf(out, "{");
            DoTexInnerHighlight(node->left, highlight, out);
            fprintf(out, "}^{");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, "}");
            break;

        case kOperationSin:
            fprintf(out, "\\sin(");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, ")");
            break;

        case kOperationCos:
            fprintf(out, "\\cos(");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, ")");
            break;

        case kOperationTg:
            fprintf(out, "\\tan(");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, ")");
            break;

        case kOperationLn:
            fprintf(out, "\\log(");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, ")");
            break;

        case kOperationArctg:
            fprintf(out, "\\arctan(");
            DoTexInnerHighlight(node->right, highlight, out);
            fprintf(out, ")");
            break;

        default:
            fprintf(out, "?");
            break;
    }
}

void DoTexStep(DifNode_t *root, DifNode_t *current, const char *var, FILE *out) {
    fprintf(out, "\\text{%s} \\\\ \n", TexPhrasesArray[rand() % TEX_PHRASES_COUNT]);
    fprintf(out, "\\[\\frac{df}{d%s} = ", var);
    // fprintf(out, "$");

    DoTexInnerHighlight(root, current, out);

    fprintf(out, " \\]\\\\ \n");
}

void BeginTex(FILE *out) {
    fprintf(out, "\\documentclass{article}\n");
    fprintf(out, "\\usepackage{amsmath}\n");
    fprintf(out, "\n\\usepackage[utf8]{inputenc}\n");
    fprintf(out, "\\usepackage[english,russian]{babel}");
    fprintf(out, "\\usepackage{xcolor}\n");
    fprintf(out, "\\usepackage[left=2cm, top=2cm, right=2cm, bottom=2cm]{geometry}\n\n");
    fprintf(out, "\\begin{document}\n\\fontsize{15}{18}\\selectfont\n");
}

void EndTex(FILE *out) {
    fprintf(out, "\n\\end{document}\n");
}

void DoTex(DifNode_t *node, const char *value, FILE *out, bool is_last) {
    assert(node);
    assert(value);
    assert(out);

    //fprintf(out, "\n\\[\n");
    fprintf(out, "$\\frac{df}{d%s} = ", value);
    DoTexInnerHighlight(node, NULL, out);
    fprintf(out, "$");
    if (!is_last) fprintf(out, " \\\\ \n");
}

void PrintSolution(DifNode_t *node, double answer, FILE *out) {
    assert(node);
    assert(out);

    fprintf(out, "\\[");
    DoTexInnerHighlight(node, NULL, out);
    fprintf(out, " = %lf\\]", answer);
}
