#!/bin/bash

# Script de teste automático para o compilador C-Minus
# Execute: chmod +x test_all.sh && ./test_all.sh

echo "=========================================="
echo "  SUITE DE TESTES - COMPILADOR C-MINUS"
echo "=========================================="
echo ""

# Cores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Contador de testes
TOTAL=0
PASSED=0
FAILED=0

# Função para executar teste
run_test() {
    local test_name=$1
    local test_file=$2
    local should_fail=$3
    
    TOTAL=$((TOTAL + 1))
    echo "----------------------------------------"
    echo "Teste $TOTAL: $test_name"
    echo "Arquivo: $test_file"
    echo ""
    
    if [ ! -f "$test_file" ]; then
        echo -e "${YELLOW}AVISO${NC}: Arquivo não encontrado, criando..."
        create_test_file "$test_file" "$test_name"
    fi
    
    ./cminus "$test_file" > "output_$TOTAL.txt" 2>&1
    exit_code=$?
    
    if [ "$should_fail" = "yes" ]; then
        if [ $exit_code -ne 0 ]; then
            echo -e "${GREEN}✓ PASSOU${NC}: Erro detectado corretamente"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}✗ FALHOU${NC}: Deveria ter falhado mas passou"
            FAILED=$((FAILED + 1))
        fi
    else
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}✓ PASSOU${NC}: Compilou com sucesso"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}✗ FALHOU${NC}: Deveria ter passado mas falhou"
            FAILED=$((FAILED + 1))
            cat "output_$TOTAL.txt"
        fi
    fi
    
    echo ""
}

# Função para criar arquivos de teste
create_test_file() {
    local filename=$1
    local test_name=$2
    
    case "$test_name" in
        "Programa Simples")
            cat > "$filename" << 'EOF'
int main(void) {
    int x;
    x = 5 + 3;
    output = x;
    return 0;
}
EOF
            ;;
        "Função com Parâmetros")
            cat > "$filename" << 'EOF'
int soma(int a, int b) {
    return a + b;
}

int main(void) {
    int resultado;
    resultado = soma(10, 20);
    output = resultado;
    return 0;
}
EOF
            ;;
        "Arrays")
            cat > "$filename" << 'EOF'
int main(void) {
    int v[10];
    int i;
    i = 0;
    while (i < 10) {
        v[i] = i * 2;
        i = i + 1;
    }
    output = v[5];
    return 0;
}
EOF
            ;;
        "If-Else")
            cat > "$filename" << 'EOF'
int main(void) {
    int x;
    x = 10;
    if (x > 5)
        output = 1;
    else
        output = 0;
    return 0;
}
EOF
            ;;
        "While Loop")
            cat > "$filename" << 'EOF'
int main(void) {
    int i;
    int soma;
    i = 1;
    soma = 0;
    while (i <= 10) {
        soma = soma + i;
        i = i + 1;
    }
    output = soma;
    return 0;
}
EOF
            ;;
        "Função Recursiva")
            cat > "$filename" << 'EOF'
int factorial(int n) {
    int result;
    if (n <= 1)
        result = 1;
    else
        result = n * factorial(n - 1);
    return result;
}

int main(void) {
    output = factorial(5);
    return 0;
}
EOF
            ;;
        "Input/Output")
            cat > "$filename" << 'EOF'
int main(void) {
    int x;
    int y;
    x = input;
    y = x * 2;
    output = y;
    return 0;
}
EOF
            ;;
        "Múltiplas Funções")
            cat > "$filename" << 'EOF'
int dobro(int x) {
    return x * 2;
}

int triplo(int x) {
    return x * 3;
}

int main(void) {
    int a;
    a = 5;
    output = dobro(a);
    output = triplo(a);
    return 0;
}
EOF
            ;;
        "Erro: Variável Não Declarada")
            cat > "$filename" << 'EOF'
int main(void) {
    output = x;
    return 0;
}
EOF
            ;;
        "Erro: Declaração Duplicada")
            cat > "$filename" << 'EOF'
int main(void) {
    int x;
    int x;
    return 0;
}
EOF
            ;;
        "Erro: Função Não Declarada")
            cat > "$filename" << 'EOF'
int main(void) {
    int x;
    x = foo();
    return 0;
}
EOF
            ;;
        "Erro: Main Ausente")
            cat > "$filename" << 'EOF'
int foo(void) {
    return 0;
}
EOF
            ;;
        "Erro: Sintaxe Inválida")
            cat > "$filename" << 'EOF'
int main(void) {
    int x
    x = 5;
    return 0;
}
EOF
            ;;
        "Erro: Return Inválido")
            cat > "$filename" << 'EOF'
int soma(int a, int b) {
    return;
}

int main(void) {
    return 0;
}
EOF
            ;;
    esac
}

# Compilar o projeto
echo "Compilando o projeto..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}ERRO: Falha na compilação do projeto${NC}"
    exit 1
fi

echo -e "${GREEN}Compilação bem-sucedida!${NC}"
echo ""

# Criar diretório para testes
mkdir -p test_cases

# ===================================
# TESTES QUE DEVEM PASSAR
# ===================================

echo "=========================================="
echo "  TESTES DE SUCESSO"
echo "=========================================="
echo ""

run_test "Programa Simples" "test_cases/test1.cm" "no"
run_test "Função com Parâmetros" "test_cases/test2.cm" "no"
run_test "Arrays" "test_cases/test3.cm" "no"
run_test "If-Else" "test_cases/test4.cm" "no"
run_test "While Loop" "test_cases/test5.cm" "no"
run_test "Função Recursiva" "test_cases/test6.cm" "no"
run_test "Input/Output" "test_cases/test7.cm" "no"
run_test "Múltiplas Funções" "test_cases/test8.cm" "no"

# ===================================
# TESTES QUE DEVEM FALHAR
# ===================================

echo "=========================================="
echo "  TESTES DE ERRO (Devem Falhar)"
echo "=========================================="
echo ""

run_test "Erro: Variável Não Declarada" "test_cases/error1.cm" "yes"
run_test "Erro: Declaração Duplicada" "test_cases/error2.cm" "yes"
run_test "Erro: Função Não Declarada" "test_cases/error3.cm" "yes"
run_test "Erro: Main Ausente" "test_cases/error4.cm" "yes"
run_test "Erro: Sintaxe Inválida" "test_cases/error5.cm" "yes"
run_test "Erro: Return Inválido" "test_cases/error6.cm" "yes"

# ===================================
# RESUMO
# ===================================

echo "=========================================="
echo "  RESUMO DOS TESTES"
echo "=========================================="
echo "Total de testes: $TOTAL"
echo -e "${GREEN}Passaram: $PASSED${NC}"
echo -e "${RED}Falharam: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ TODOS OS TESTES PASSARAM!${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}✗ ALGUNS TESTES FALHARAM${NC}"
    echo "Verifique os arquivos output_*.txt para detalhes"
    exit 1
fi