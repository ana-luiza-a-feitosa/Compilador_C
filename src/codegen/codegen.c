#include "codegen.h"
#include "globals.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void emitIndent(FILE *out, int n) {
    for (int i = 0; i < n; i++) fputc(' ', out);
}

static const char *typeToC(TypeSpec ty) {
    return (ty == TY_INT) ? "int" : "void";
}

static const char *opToC(TokenType op) {
    switch (op) {
        case TK_PLUS:  return "+";
        case TK_MINUS: return "-";
        case TK_TIMES: return "*";
        case TK_OVER:  return "/";
        case TK_LT:    return "<";
        case TK_LE:    return "<=";
        case TK_GT:    return ">";
        case TK_GE:    return ">=";
        case TK_EQ:    return "==";
        case TK_NE:    return "!=";
        default:       return "?";
    }
}

static void genExpr(TreeNode *t, FILE *out);

static void genStmt(TreeNode *t, FILE *out, int indent) {
    for (; t; t = t->sibling) {
        if (!t) continue;

        if (t->nodekind == ND_STMT) {
            switch (t->kind.stmt) {
                case ST_EXPR:
                    emitIndent(out, indent);
                    if (t->child[0]) genExpr(t->child[0], out);
                    fprintf(out, ";\n");
                    break;

                case ST_RETURN:
                    emitIndent(out, indent);
                    fprintf(out, "return");
                    if (t->child[0]) {
                        fprintf(out, " ");
                        genExpr(t->child[0], out);
                    }
                    fprintf(out, ";\n");
                    break;

                case ST_IF:
                    emitIndent(out, indent);
                    fprintf(out, "if (");
                    genExpr(t->child[0], out);
                    fprintf(out, ")\n");
                    genStmt(t->child[1], out, indent + 2);
                    if (t->child[2]) {
                        emitIndent(out, indent);
                        fprintf(out, "else\n");
                        genStmt(t->child[2], out, indent + 2);
                    }
                    break;

                case ST_WHILE:
                    emitIndent(out, indent);
                    fprintf(out, "while (");
                    genExpr(t->child[0], out);
                    fprintf(out, ")\n");
                    genStmt(t->child[1], out, indent + 2);
                    break;

                case ST_COMPOUND: {
                    emitIndent(out, indent);
                    fprintf(out, "{\n");

                    /* child[0] = decls locais, child[1] = stmts */
                    TreeNode *d = t->child[0];
                    while (d) {
                        if (d->nodekind == ND_DECL && d->kind.decl == DECL_VAR) {
                            emitIndent(out, indent + 2);
                            fprintf(out, "%s %s", typeToC(d->type), d->name);
                            if (d->arraySize >= 0) fprintf(out, "[%d]", d->arraySize);
                            fprintf(out, ";\n");
                        }
                        d = d->sibling;
                    }

                    genStmt(t->child[1], out, indent + 2);

                    emitIndent(out, indent);
                    fprintf(out, "}\n");
                    break;
                }

                default:
                    break;
            }
        }
    }
}

static void genExpr(TreeNode *t, FILE *out) {
    if (!t || t->nodekind != ND_EXPR) return;

    switch (t->kind.expr) {
        case EX_CONST:
            fprintf(out, "%d", t->val);
            return;

        case EX_ID:
            fprintf(out, "%s", t->name);
            return;

        case EX_INDEX:
            fprintf(out, "%s[", t->name);
            genExpr(t->child[0], out);
            fprintf(out, "]");
            return;

        case EX_ASSIGN:
            genExpr(t->child[0], out);
            fprintf(out, " = ");
            genExpr(t->child[1], out);
            return;

        case EX_OP:
            fprintf(out, "(");
            genExpr(t->child[0], out);
            fprintf(out, " %s ", opToC(t->op));
            genExpr(t->child[1], out);
            fprintf(out, ")");
            return;

        case EX_CALL: {
            if (t->name && strcmp(t->name, "output") == 0) {
                fprintf(out, "output(");
                if (t->child[0]) genExpr(t->child[0], out);
                fprintf(out, ")");
                return;
            }
            if (t->name && strcmp(t->name, "input") == 0) {
                fprintf(out, "input()");
                return;
            }

            fprintf(out, "%s(", t->name ? t->name : "<?>");
            TreeNode *a = t->child[0];
            while (a) {
                genExpr(a, out);
                if (a->sibling) fprintf(out, ", ");
                a = a->sibling;
            }
            fprintf(out, ")");
            return;
        }

        default:
            return;
    }
}

static void genDecl(TreeNode *t, FILE *out) {
    for (; t; t = t->sibling) {
        if (!t || t->nodekind != ND_DECL) continue;

        if (t->kind.decl == DECL_VAR) {
            fprintf(out, "%s %s", typeToC(t->type), t->name);
            if (t->arraySize >= 0) fprintf(out, "[%d]", t->arraySize);
            fprintf(out, ";\n");
        } else if (t->kind.decl == DECL_FUN) {
            fprintf(out, "%s %s(", typeToC(t->type), t->name);

            TreeNode *p = t->child[0];
            if (!p) fprintf(out, "void");
            else {
                while (p) {
                    fprintf(out, "%s %s", typeToC(p->type), p->name);
                    if (p->arraySize == 0) fprintf(out, "[]");
                    if (p->sibling) fprintf(out, ", ");
                    p = p->sibling;
                }
            }
            fprintf(out, ")\n");

            genStmt(t->child[1], out, 0);
            fprintf(out, "\n");
        }
    }
}

void codegen(TreeNode *syntaxTree, const char *filename) {
    FILE *out = fopen("out/out.c", "w");
    if (!out) {
        printf("Erro ao criar out/out.c\n");
        return;
    }

    /* INCLUDE DO RUNTIME */
    fprintf(out, "#include \"../runtime/runtime.h\"\n\n");

    genDecl(syntaxTree, out);

    fclose(out);
}

