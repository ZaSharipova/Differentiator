#include "DoTex.h"

#include <stdio.h>
#include <assert.h>

#include "Enums.h"
#include "Structs.h"
 
static void DoTexInner(DifNode_t *node, FILE *out) { ///////
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

    if (node->type == kOperation) {
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
                //fprintf(out, "(");
                DoTexInner(node->left, out);
                
                fprintf(out, "\\cdot ");
                DoTexInner(node->right, out);
                //fprintf(out, ")");
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
                fprintf(out, "\\sin{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;

            case (kOperationCos):
                fprintf(out, "\\cos{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;

            case (kOperationTg):
                fprintf(out, "\\tan{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;
            case (kOperationLn):
                fprintf(out, "\\log{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;
            case (kOperationArctg):
                fprintf(out, "\\arctan{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;
            case (kOperationNone):
            default:
                fprintf(out, "?");
                break;
        }
    }
}

void BeginTex(FILE *out) {
    assert(out);

    fprintf(out, "\\documentclass{article}\n");
    fprintf(out, "\\usepackage{amsmath}\n\\usepackage[left=2cm, top=2cm, right=2cm, bottom=2cm]{geometry}\n\n");
    fprintf(out, "\\begin{document}\n\\fontsize{15}{18}\\selectfont\n");
    fprintf(out, "\n\\begin{align*}\n");
}

void EndTex(FILE *out) {
    assert(out);

    fprintf(out, "\n\\end{align*}\n");
    fprintf(out, "\n\\end{document}");
}

void DoTex(DifNode_t *node, const char *value, FILE *out, bool is_last) {
    assert(node);
    assert(value);
    assert(out);

    // fprintf(out, "\n\\[\n");
    fprintf(out, "\\frac{df}{d%s} &= ", value);
    DoTexInner(node, out);
    if (!is_last) fprintf(out, " \\\\ \n");
}
