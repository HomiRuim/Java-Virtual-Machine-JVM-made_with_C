#!/bin/bash
# tests/run.sh - Harness de Testes (Pessoa E)
# Executa o visualizador e compara com os "golden files".

# 'set -e' para o script se qualquer comando falhar
set -e 

# --- Configuração ---
VISUALIZADOR="./visualizador-bytecode.exe"
SAMPLES_DIR="tests/samples"
GOLDEN_DIR="tests/golden"  # Gabaritos
OUTPUT_DIR="tests/output"  # Saída do teste atual

# Nossos arquivos de teste (nomes base, sem .class)
# Vamos usar o Example (simples) e o ExampleJava8 (complexo)
TEST_FILES=("Example" "ExampleJava8")

# Cores para a saída
GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m" # No Color

# --- Etapa 1: Compilação ---
echo "[+] Compilando o projeto..."
# O '-s' (silent) vem do seu script original
make -s clean
make -s
if [ ! -f "$VISUALIZADOR" ]; then
    echo -e "${RED}[!] Falha na compilacao! O executavel '$VISUALIZADOR' nao foi encontrado.${NC}"
    exit 1
fi
echo -e "${GREEN}[✓] Compilacao concluida.${NC}"

# --- Etapa 2: Preparação ---
# Garante que os diretórios de golden e output existem
mkdir -p "$GOLDEN_DIR"
mkdir -p "$OUTPUT_DIR"

# --- Etapa 3: Modo de Geração (Setup inicial) ---
# Se o script for chamado com --generate, ele CRIA os golden files
if [ "$1" == "--generate" ]; then
    echo "[!] Modo de GERACAO DE GOLDEN FILES..."
    for base_name in "${TEST_FILES[@]}"; do
        CLASS_FILE="$SAMPLES_DIR/$base_name.class"
        if [ ! -f "$CLASS_FILE" ]; then
            echo -e "${RED}[!] Arquivo de sample necessario '$CLASS_FILE' nao encontrado!${NC}"
            echo "    (Voce compilou o $base_name.java?)"
            continue
        fi
        
        # Gera Golden Pretty
        GOLDEN_PRETTY="$GOLDEN_DIR/$base_name.pretty.golden"
        echo "    -> Gerando $GOLDEN_PRETTY"
        "$VISUALIZADOR" --pretty "$CLASS_FILE" > "$GOLDEN_PRETTY"
        
        # Gera Golden JSON
        GOLDEN_JSON="$GOLDEN_DIR/$base_name.json.golden"
        echo "    -> Gerando $GOLDEN_JSON"
        "$VISUALIZADOR" --json "$CLASS_FILE" > "$GOLDEN_JSON"
    done
    echo -e "${GREEN}[✓] Golden files gerados! Verifique-os manualmente e faca o commit.${NC}"
    exit 0
fi

# --- Etapa 4: Modo de Teste (Padrão) ---
echo "[+] Executando testes de regressao..."
FAILED_TESTS=0

for base_name in "${TEST_FILES[@]}"; do
    CLASS_FILE="$SAMPLES_DIR/$base_name.class"
    echo "  --- Testando: $base_name ---"
    
    if [ ! -f "$CLASS_FILE" ]; then
        echo -e "    ${RED}FALHOU: Arquivo de sample '$CLASS_FILE' nao encontrado.${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        continue
    fi

    # Teste 1: Modo Pretty
    GOLDEN_PRETTY="$GOLDEN_DIR/$base_name.pretty.golden"
    OUTPUT_PRETTY="$OUTPUT_DIR/$base_name.pretty.out"
    if [ ! -f "$GOLDEN_PRETTY" ]; then
        echo -e "    ${RED}FALHOU (Pretty): Golden file '$GOLDEN_PRETTY' nao existe.${NC}"
        echo "    Execute 'bash tests/run.sh --generate' primeiro."
        FAILED_TESTS=$((FAILED_TESTS + 1))
    else
        "$VISUALIZADOR" --pretty "$CLASS_FILE" > "$OUTPUT_PRETTY"
        # Compara o gabarito com a saída atual
        if diff -u "$GOLDEN_PRETTY" "$OUTPUT_PRETTY"; then
            echo -e "    ${GREEN}PASSOU (Pretty)${NC}"
        else
            echo -e "    ${RED}FALHOU (Pretty): Saida difere do golden file.${NC}"
            echo "    Veja 'diff -u \"$GOLDEN_PRETTY\" \"$OUTPUT_PRETTY\"' para detalhes."
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi

    # Teste 2: Modo JSON
    GOLDEN_JSON="$GOLDEN_DIR/$base_name.json.golden"
    OUTPUT_JSON="$OUTPUT_DIR/$base_name.json.out"
    if [ ! -f "$GOLDEN_JSON" ]; then
        echo -e "    ${RED}FALHOU (JSON): Golden file '$GOLDEN_JSON' nao existe.${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    else
        "$VISUALIZADOR" --json "$CLASS_FILE" > "$OUTPUT_JSON"
        # Compara o gabarito com a saída atual
        if diff -u "$GOLDEN_JSON" "$OUTPUT_JSON"; then
            echo -e "    ${GREEN}PASSOU (JSON)${NC}"
        else
            echo -e "    ${RED}FALHOU (JSON): Saida difere do golden file.${NC}"
            echo "    Veja 'diff -u \"$GOLDEN_JSON\" \"$OUTPUT_JSON\"' para detalhes."
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
done

# --- Etapa 5: Relatório Final ---
if [ "$FAILED_TESTS" -eq 0 ]; then
    echo -e "\n${GREEN}[✓] Todos os testes passaram!${NC}"
    exit 0
else
    echo -e "\n${RED}[!] $FAILED_TESTS teste(s) falharam.${NC}"
    exit 1
fi