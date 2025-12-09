# 1. Compilador e Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -g -O2 -m32        ### + -O2 + -m32 para compilar em 32 bits
LDFLAGS = -lm -m32

# 2. Nome do Binário Principal
BIN_NAME = visualizador-bytecode

ifeq ($(OS),Windows_NT)
    EXE_EXT = .exe
else
    EXE_EXT =
endif

TARGET_EXE = $(BIN_NAME)$(EXE_EXT)

# 3. Fontes
APP_SRCS = src/main.c \
           src/cli.c \
           src/io.c \
           src/classfile.c \
           src/attributes.c \
           src/parse_code.c \
           src/resolve.c \
           src/disasm.c \
           src/print.c \
           src/json.c \
           src/jvm.c \
           src/stack.c \
           src/heap_manager.c \
           src/execute.c

CORE_SRCS = src/io.c \
            src/classfile.c \
            src/attributes.c \
            src/parse_code.c \
            src/resolve.c \
            src/disasm.c

# 4. Objetos + deps automáticas
APP_OBJS = $(APP_SRCS:.c=.o)
CORE_OBJS = $(CORE_SRCS:.c=.o)

### deps: gera .d ao compilar e inclui se existirem
CFLAGS += -MMD -MP
DEPS := $(APP_OBJS:.o=.d) $(CORE_OBJS:.o=.d)

# 5. Regra principal
.PHONY: all
all: $(TARGET_EXE)

# 6. Linkagem do binário principal
$(TARGET_EXE): $(APP_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Executavel principal '$(TARGET_EXE)' criado com sucesso."

# 7. Alvos de testes auxiliares
.PHONY: validate_class test_attributes
validate_class: src/validate_class.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o validate_class_runner$(EXE_EXT) $^ $(LDFLAGS)
	@echo "Executavel de teste 'validate_class_runner$(EXE_EXT)' criado."

test_attributes: src/test_attributes.c $(CORE_OBJS)
	$(CC) $(CFLAGS) -o test_attributes_runner$(EXE_EXT) $^ $(LDFLAGS)
	@echo "Executavel de teste 'test_attributes_runner$(EXE_EXT)' criado."

# 8. Compile qualquer src/%.c em src/%.o
src/%.o: src/%.c $(wildcard include/*.h) include/execute.h
	$(CC) $(CFLAGS) -c $< -o $@

### inclui .d se existirem (não quebra na primeira execução)
-include $(DEPS)

# =============================
# Testes de integração (golden)
# =============================

TEST_DIR := tests
SAMPLES_DIR := $(TEST_DIR)/samples
GOLDEN_DIR := $(TEST_DIR)/golden
BUILD_TEST_DIR := build/test

SAMPLE := $(SAMPLES_DIR)/Example.class
PRETTY_OUT := $(BUILD_TEST_DIR)/Example.pretty.out
JSON_OUT := $(BUILD_TEST_DIR)/Example.json.out
PRETTY_GOLDEN := $(GOLDEN_DIR)/Example.pretty.golden
JSON_GOLDEN := $(GOLDEN_DIR)/Example.json.golden

### mkdir cross-platform (Bash OU PowerShell)

ifeq ($(OS),Windows_NT)
$(BUILD_TEST_DIR):
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(BUILD_TEST_DIR)' | Out-Null"
else
$(BUILD_TEST_DIR):
	@mkdir -p "$(BUILD_TEST_DIR)"
endif


.PHONY: test test-pretty test-json golden-update _diff

test: ./$(TARGET_EXE) test-pretty test-json
	@echo "Todos os testes de integracao passaram."

### se o pretty já for o padrão, não precisa --pretty
test-pretty: $(BUILD_TEST_DIR) $(TARGET_EXE) $(SAMPLE) $(PRETTY_GOLDEN)
	@echo [TEST pretty] $(SAMPLE)
	@$./(TARGET_EXE) $(SAMPLE) > "$(PRETTY_OUT)"
	@$(MAKE) _diff FILE1="$(PRETTY_OUT)" FILE2="$(PRETTY_GOLDEN)"

test-json: $(BUILD_TEST_DIR) $(TARGET_EXE) $(SAMPLE) $(JSON_GOLDEN)
	@echo [TEST json] $(SAMPLE)
	@$./(TARGET_EXE) $(SAMPLE) --json > "$(JSON_OUT)"
	@$(MAKE) _diff FILE1="$(JSON_OUT)" FILE2="$(JSON_GOLDEN)"

### diff: tenta 'diff' (bash). Se não tiver, usa PowerShell Compare-Object
# --- comparação de arquivos (golden) ---
# --- comparação de arquivos (golden) ---
ifeq ($(OS),Windows_NT)
_diff:
	@powershell -NoProfile -ExecutionPolicy Bypass -Command "$$gold = '$(FILE2)'; $$out = '$(FILE1)'; \
		if (-not (Test-Path $$gold)) { Write-Host 'ERROR: golden file não existe:' $$gold -ForegroundColor Red; exit 1 } ; \
		if (-not (Test-Path $$out))  { Write-Host 'ERROR: output não existe:' $$out  -ForegroundColor Red; exit 1 } ; \
		$$a = Get-Content -Raw $$gold; $$b = Get-Content -Raw $$out; \
		if ($$a -eq $$b) { Write-Host 'Diff OK' } else { \
			Write-Host '--- MISMATCH ---' -ForegroundColor Red; \
			Write-Host 'Golden: ' $$gold; \
			Write-Host 'Output: ' $$out; \
			$$ra = Get-Content $$gold; $$rb = Get-Content $$out; \
			$$d = Compare-Object -ReferenceObject $$ra -DifferenceObject $$rb; \
			Write-Host 'Primeiras diferencas:'; $$d | Select-Object -First 20 | Format-Table | Out-String | Write-Host; \
			exit 1 \
		}"
else
_diff:
	@diff -u "$(FILE2)" "$(FILE1)"
endif




golden-update: $(BUILD_TEST_DIR) $(TARGET_EXE) $(SAMPLE)
	@echo "Atualizando golden..."
	@$(TARGET_EXE) $(SAMPLE) > "$(PRETTY_GOLDEN)"
	@$(TARGET_EXE) $(SAMPLE) --json > "$(JSON_GOLDEN)"
	@echo "Golden atualizado com sucesso."

# 9. Limpeza cross-platform
.PHONY: clean
ifeq ($(OS),Windows_NT)
clean:
	-powershell -Command "Remove-Item -Recurse -Force src\*.o 2>$null; exit 0"
	-powershell -Command "Remove-Item -Recurse -Force $(TARGET_EXE) 2>$null; exit 0"
	-powershell -Command "Remove-Item -Recurse -Force validate_class_runner$(EXE_EXT),test_attributes_runner$(EXE_EXT),test_runner$(EXE_EXT) 2>$null; exit 0"
	-powershell -Command "Remove-Item -Recurse -Force $(BIN_NAME) 2>$null; exit 0"
	@echo "Arquivos compilados removidos."
else
clean:
	rm -f src/*.o
	rm -f $(TARGET_EXE)
	rm -f validate_class_runner$(EXE_EXT) test_attributes_runner$(EXE_EXT) test_runner$(EXE_EXT)
	rm -f $(BIN_NAME) validate_class_runner test_attributes_runner test_runner
	@echo "Arquivos compilados removidos."
endif
