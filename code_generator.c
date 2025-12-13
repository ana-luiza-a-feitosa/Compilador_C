#include "code_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variáveis globais
TAC *code_head = NULL; 
TAC *code_tail = NULL;
int temp_counter = 0; 
int label_counter = 0;

// Variável para rastrear a última temporária usada na AST
int next_temp_loc = 0;

// Cria uma nova variável temporária (t1, t2, ...)
char *new_temp() {
    char *temp_name = (char *)malloc(10);
    sprintf(temp_name, "t%d", ++temp_counter);
    return temp_name;
}

// Cria um novo label (L1, L2, ...)
char *new_label() {
    char *label_name = (char *)malloc(10);
    sprintf(label_name, "L%d", ++label_counter);
    return label_name;
}

// Emite uma instrução de 3 endereços e a anexa à lista
void emit_tac(char *op, char *arg1, char *arg2, char *result) {
    TAC *new_tac = (TAC *)malloc(sizeof(TAC));
    if (new_tac == NULL) {
        fprintf(stderr, "Erro de alocacao para instrucao TAC.\n");
        exit(1);
    }
    
    new_tac->op = strdup(op);
    new_tac->arg1 = arg1 ? strdup(arg1) : NULL;
    new_tac->arg2 = arg2 ? strdup(arg2) : NULL;
    new_tac->result = result ? strdup(result) : NULL;
    new_tac->next = NULL;

    if (code_head == NULL) {
        code_head = new_tac;
        code_tail = new_tac;
    } else {
        code_tail->next = new_tac;
        code_tail = new_tac;
    }
}

// Função principal de travessia (pós-ordem para expressões, pré-ordem para comandos)
void generate_code(TreeNode *tree) {
    if (tree == NULL) return;

    char *t1 = NULL, *t2 = NULL, *t3 = NULL;
    char *label1 = NULL, *label2 = NULL;

    // Processa o nó atual
    switch (tree->nodekind) {
        case DeclK: 
            // Declarações (não geram código, apenas alocam espaço)
            break;

        case StmtK:
            switch (tree->kind.stmt) {
                case CompK: 
                    // Bloco (apenas chama recursivamente para os filhos)
                    generate_code(tree->child[0]); // Declarações locais (se houver)
                    generate_code(tree->child[1]); // Comandos
                    break;
                
                case IfK: 
                    // IF (expressao) comando [ELSE comando]
                    label1 = new_label(); // Label para o bloco else/fim
                    label2 = new_label(); // Label para o fim (se houver else)

                    generate_code(tree->child[0]); // Expressão condicional
                    t1 = tree->child[0]->name; // Resultado da condição (temporária)

                    // Se a condição for falsa, pule
                    emit_tac("if_false", t1, NULL, tree->child[2] ? label1 : label2);
                    
                    generate_code(tree->child[1]); // Bloco THEN
                    
                    if (tree->child[2]) {
                        emit_tac("goto", NULL, NULL, label2); // Pula o ELSE
                        emit_tac("label", label1, NULL, NULL); // Label ELSE
                        generate_code(tree->child[2]); // Bloco ELSE
                    } else {
                        emit_tac("label", label1, NULL, NULL); // Label FIM
                    }
                    emit_tac("label", label2, NULL, NULL);
                    break;

                case WhileK: 
                    // WHILE (expressao) comando
                    label1 = new_label(); // Label de Teste (inicio do loop)
                    label2 = new_label(); // Label de Fim

                    emit_tac("label", label1, NULL, NULL); // Marca o início
                    
                    generate_code(tree->child[0]); // Expressão condicional
                    t1 = tree->child[0]->name; // Resultado da condição

                    emit_tac("if_false", t1, NULL, label2); // Se falso, pula para o fim
                    
                    generate_code(tree->child[1]); // Corpo do loop
                    
                    emit_tac("goto", NULL, NULL, label1); // Volta para o teste
                    emit_tac("label", label2, NULL, NULL); // Marca o fim
                    break;

                case AssignK: 
                    // ID = Expressao
                    generate_code(tree->child[1]); // Gera código para a expressão à direita
                    t1 = tree->child[1]->name; // Resultado da expressão
                    t2 = tree->child[0]->name; // ID (variável destino)
                    emit_tac("=", t1, NULL, t2);
                    break;
                
                case RetK:
                    // RETURN [Expressao]
                    if (tree->child[0] != NULL) {
                        generate_code(tree->child[0]);
                        t1 = tree->child[0]->name;
                        emit_tac("return", t1, NULL, NULL);
                    } else {
                        emit_tac("return", NULL, NULL, NULL);
                    }
                    break;

                case InputK:
                    t1 = new_temp();
                    emit_tac("input", NULL, NULL, t1);
                    tree->name = t1;
                    break;

                case OutputK:
                    generate_code(tree->child[0]);
                    t1 = tree->child[0]->name;
                    emit_tac("output", t1, NULL, NULL);
                    break;
                
                default:
                    // Outros comandos (chamada de função, etc.)
                    break;
            }
            break;

        case ExpK:
            switch (tree->kind.exp) {
                case OpK: 
                    // Expressão Binária (pós-ordem)
                    generate_code(tree->child[0]); // Operando 1
                    generate_code(tree->child[1]); // Operando 2
                    
                    t1 = tree->child[0]->name;
                    t2 = tree->child[1]->name;
                    t3 = new_temp();
                    
                    emit_tac(tree->op, t1, t2, t3);
                    tree->name = t3; // Armazena o resultado na AST para o nó pai
                    break;

                case ConstK:
                    // Constante
                    t1 = new_temp();
                    char val_str[12];
                    sprintf(val_str, "%d", tree->val);
                    emit_tac("=", val_str, NULL, t1);
                    tree->name = t1;
                    break;
                
                case IdK:
                    // Variável ID (o nome já é o 'endereço')
                    tree->name = strdup(tree->name);
                    break;
                
                default:
                    break;
            }
            break;
        
        default:
            // Travessia no sibling
            break;
    }

    // Processa o irmão
    generate_code(tree->sibling);
}

// Imprime a lista ligada de instruções de 3 endereços
void print_tac() {
    TAC *current = code_head;
    while (current != NULL) {
        if (strcmp(current->op, "label") == 0) {
            fprintf(stdout, "%s:\n", current->arg1);
        } else if (strcmp(current->op, "goto") == 0) {
            fprintf(stdout, "\tgoto %s\n", current->result);
        } else if (strcmp(current->op, "if_false") == 0) {
            fprintf(stdout, "\tif %s == 0 goto %s\n", current->arg1, current->result);
        } else if (current->arg2 != NULL) {
            // Operação binária
            fprintf(stdout, "\t%s = %s %s %s\n", current->result, current->arg1, current->op, current->arg2);
        } else if (strcmp(current->op, "=") == 0) {
            // Atribuição ou constante
            fprintf(stdout, "\t%s = %s\n", current->result, current->arg1);
        } else if (strcmp(current->op, "return") == 0) {
            fprintf(stdout, "\treturn %s\n", current->arg1 ? current->arg1 : "");
        } else if (strcmp(current->op, "output") == 0) {
            fprintf(stdout, "\toutput %s\n", current->arg1);
        } else {
            fprintf(stdout, "\t[TAC] %s %s %s -> %s\n", 
                    current->op, 
                    current->arg1 ? current->arg1 : "_", 
                    current->arg2 ? current->arg2 : "_", 
                    current->result ? current->result : "_");
        }

        current = current->next;
    }
}