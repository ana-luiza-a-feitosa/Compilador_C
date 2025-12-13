#include "semantic_analyzer.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Variável para rastrear se houve algum erro semântico
extern int Error;

// Rastreador para o tipo da função atual (para checar o 'return')
static ExpType current_func_type = VOID;

// Função auxiliar para exibir erro semântico [cite: 40, 41]
static void semantic_error(TreeNode *t, const char *msg) {
    if (t == NULL) return;
    fprintf(stderr, "ERRO SEMANTICO: identificador '%s' LINHA: %d. %s\n", t->name ? t->name : "N/A", t->lineno, msg);
    Error = 1; 
}

// 1. Inserção de Símbolos e Checagem de Declaração/Tipo
void process_declaration(TreeNode *t) {
    if (t == NULL) return;
    
    // 1. Verifica declaração duplicada no escopo atual [cite: 37]
    if (lookup_symbol_current_scope(t->name) != NULL) {
        semantic_error(t, "Declaracao duplicada no mesmo escopo.");
        return;
    }
    
    // 2. Insere na Tabela de Símbolos
    bool is_func = (t->nodekind == DeclK && t->kind.decl == FuncDeclK);
    insert_symbol(t->name, t->type, t->lineno, is_func);

    // 3. Checagem de parâmetros (se for função)
    if (is_func) {
        // Entrar no escopo da função para inserir parâmetros
        enter_scope();
        TreeNode *param = t->child[0]; // child[0] é a lista de parâmetros
        while (param != NULL) {
            if (param->nodekind == ParamK) {
                // Checar duplicatas de parâmetros (no escopo atual)
                if (lookup_symbol_current_scope(param->name) != NULL) {
                    semantic_error(param, "Parametro duplicado.");
                } else {
                    insert_symbol(param->name, param->type, param->lineno, false);
                }
            }
            param = param->sibling;
        }
    }
}

// 2. Checagem de Comandos (Statements) e Expressões (Expressions)
void check_node(TreeNode *t) {
    if (t == NULL) return;

    // A. Descida e Processamento dos filhos
    for (int i = 0; i < MAX_CHILDREN; i++) {
        // Tratamento especial para o corpo da função (compound_stmt)
        if (t->nodekind == DeclK && t->kind.decl == FuncDeclK && i == 1) {
            current_func_type = t->type; // Define o tipo de retorno esperado
            check_node(t->child[i]);
            // Não precisa sair do escopo, pois compound_stmt fará isso.
        } else {
            check_node(t->child[i]);
        }
    }
    
    // B. Ação no nó atual (Pós-Ordem para Expressões)
    switch (t->nodekind) {
        case DeclK: 
            process_declaration(t);
            break;
        
        case ExpK:
            if (t->kind.exp == IdK || t->kind.exp == ArrayK) {
                // Checa uso de variável não declarada [cite: 36]
                Symbol *s = lookup_symbol(t->name);
                if (s == NULL) {
                    semantic_error(t, "Uso de identificador nao declarado.");
                    t->type = INTEGER; // Default para continuar
                } else {
                    t->type = s->type; // Atualiza o tipo do nó
                    if (t->kind.exp == ArrayK && s->type != ARRAY) {
                        semantic_error(t, "Tentativa de indexar um identificador que nao e array.");
                    } else if (t->kind.exp == IdK && s->type == FUNCTION) {
                        semantic_error(t, "Uso incorreto: Funcao nao pode ser usada como variavel simples.");
                    }
                }
            } else if (t->kind.exp == OpK) {
                // Checa incompatibilidade de tipos em expressões [cite: 38]
                if (t->child[0]->type != INTEGER || t->child[1]->type != INTEGER) {
                    semantic_error(t, "Operacao binaria exige operandos inteiros (ou Array indexado).");
                }
                t->type = INTEGER; // Resultado de op. é sempre int
            }
            break;

        case StmtK:
            if (t->kind.stmt == AssignK) {
                // Checa atribuição
                TreeNode *lhs = t->child[0];
                TreeNode *rhs = t->child[1];
                
                if (lhs->type == FUNCTION) {
                     semantic_error(lhs, "Nao e possivel atribuir a uma funcao.");
                }
                if (lhs->type != INTEGER || rhs->type != INTEGER) {
                    semantic_error(lhs, "Incompatibilidade de tipo na atribuicao. Esperado int.");
                }
            } else if (t->kind.stmt == RetK) {
                // Checa retorno
                TreeNode *ret_val = t->child[0];
                if (ret_val == NULL && current_func_type != VOID) {
                    semantic_error(t, "Funcao nao-void exige valor de retorno.");
                } else if (ret_val != NULL && current_func_type == VOID) {
                    semantic_error(t, "Funcao void nao pode retornar valor.");
                } else if (ret_val != NULL && ret_val->type != current_func_type) {
                     semantic_error(ret_val, "Tipo de retorno incompativel com o tipo da funcao.");
                }
            } else if (t->kind.stmt == CallK) {
                 // Checa se é função e se a quantidade/tipo dos argumentos coincide (simplificado)
                 Symbol *s = lookup_symbol(t->name);
                 if (s == NULL || s->type != FUNCTION) {
                    semantic_error(t, "Chamada a identificador nao declarado ou que nao e funcao.");
                 }
                 // **Nota:** A checagem de argumentos é complexa e requer rastrear a assinatura completa (não implementado aqui).
            }
            break;
        
        default:
            break;
    }

    // C. Processamento do irmão
    check_node(t->sibling);
}

// Função principal de análise semântica
void semantic_analysis(TreeNode *tree) {
    // 1. Inicia o escopo global
    enter_scope();
    
    // 2. Insere as funções built-in [cite: 39]
    insert_symbol("input", INTEGER, 0, true);
    insert_symbol("output", VOID, 0, true);
    
    // 3. Percorre a AST
    check_node(tree);
    
    // 4. Checa se 'main' existe (Requisito mínimo)
    if (lookup_symbol("main") == NULL) {
         fprintf(stderr, "ERRO SEMANTICO: Funcao 'main' nao declarada.\n");
         Error = 1;
    }
    
    // 5. Sai do escopo global
    leave_scope(); 
}