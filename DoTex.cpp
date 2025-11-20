#include "DoTex.h"

#include <stdio.h>
#include <assert.h>

#include "Enums.h"
#include "Structs.h"

#include <stdio.h>

static void DoTexInner(DifNode_t *node, FILE *out) {
    assert(node);
    assert(out);

    if (node->operation == kNumber) {
        fprintf(out, "%lg", node->value.number);
        return;
    }

    if (node->operation == kVariable) {
        fprintf(out, "%s", node->value.variable_name);
        return;
    }

    if (node->operation == kOperation) {
        switch (node->value.type) {
            case (kAdd):
                fprintf(out, "(");
                DoTexInner(node->left, out);
                fprintf(out, " + ");
                DoTexInner(node->right, out);
                fprintf(out, ")");
                break;

            case (kSub):
                fprintf(out, "(");
                DoTexInner(node->left, out);
                fprintf(out, " - ");
                DoTexInner(node->right, out);
                fprintf(out, ")");
                break;

            case (kMul):
                fprintf(out, "(");
                DoTexInner(node->left, out);
                fprintf(out, "\\cdot ");
                DoTexInner(node->right, out);
                fprintf(out, ")");
                break;

            case (kDiv):
                fprintf(out, "\\frac{");
                DoTexInner(node->left, out);
                fprintf(out, "}{");
                DoTexInner(node->right, out);
                fprintf(out, "}");
                break;

            case (kPow):
                fprintf(out, "{");
                DoTexInner(node->left, out);
                fprintf(out, "}^{");
                DoTexInner(node->right, out);
                fprintf(out, "}");
                break;

            case (kSin):
                fprintf(out, "\\sin{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;

            case (kCos):
                fprintf(out, "\\cos{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;

            case (kTg):
                fprintf(out, "\\tan{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
                break;
            case (kLn):
                fprintf(out, "\\log{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
            case (kArctg):
                fprintf(out, "\\arctan{(");
                DoTexInner(node->right, out);
                fprintf(out, ")}");
            case (kNone):
            default:
                fprintf(out, "?");
                break;
        }
    }
}

void DoTex(DifNode_t *node, const char *value) {
    assert(node);
    FILE *out = fopen("diftex.tex", "w");
    fprintf(out, "\\documentclass{article}\n\\begin{document}\n\\fontsize{30}{36}\\selectfont\n\\[\n");
    fprintf(out, "\\frac{d}{d%s} = ", value);
    DoTexInner(node, out);
    fprintf(out, "\n");
    fprintf(out, "\\]\n\\end{document}");
    fclose(out);
}
