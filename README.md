Fico muito feliz em ter ajudado\! É um projeto ambicioso e você deve se orgulhar do trabalho que fez. 💪

Para finalizar, criei um arquivo `README.md` completo e bem formatado para o seu repositório, organizando as instruções de configuração, uso do visualizador e as etapas de desenvolvimento do projeto.

-----

# ☕ Java Virtual Machine (JVM) Feita em C

Um projeto acadêmico de Sistemas Operacionais e Linguagens (SOL) para criar um visualizador de Bytecode e um *runtime* rudimentar para o Java, totalmente implementado na linguagem C.

-----

## 🚀 Configuração e Primeiros Passos (Ambiente Linux/WSL)

O projeto foi desenvolvido para ser compilado e executado em ambientes Linux ou no **Windows Subsystem for Linux (WSL)**, usando o GCC.

### 1\. Pré-requisitos de Instalação

Certifique-se de que o `make`, o `gcc` e o `javac` (para gerar arquivos `.class` de teste) estejam instalados:

```bash
# Instalar utilitários essenciais
sudo apt install make
sudo apt install gcc

# Instalar o JDK (Java Development Kit) para ter o javac
sudo apt install default-jdk
```

### 2\. Compilação do Projeto

Use o `Makefile` para limpar (remover binários antigos) e compilar a aplicação:

```bash
make clean
make
```

O binário principal do visualizador será gerado como `./visualizador-bytecode`.

-----

## 💻 Uso: O Visualizador de Bytecode

O programa principal é o visualizador, que analisa a estrutura interna do arquivo `.class`.

### Sintaxe de Uso

```bash
./visualizador-bytecode [opcoes] <arquivo.class>
```

### Exemplos e Opções Principais

| Comando | Descrição |
| :--- | :--- |
| `./visualizador-bytecode tests/samples/Example.class` | **Modo Normal/Pretty** (Padrão) |
| `./visualizador-bytecode tests/samples/Example.class --json` | Saída formatada como **objeto JSON** |
| `./visualizador-bytecode tests/samples/Example.class --no-code` | Oculta o disassembly do bytecode (apenas a estrutura) |
| `./visualizador-bytecode tests/samples/Example.class --verbose` | Mostra logs de depuração detalhados no `stderr` |
| `./visualizador-bytecode --help` | Mostra todas as opções de ajuda |

### Testando a Geração de Bytecode (`javac`)

Você pode usar o `javac` para compilar um código Java simples e testar o visualizador ou o modo de depuração da JVM:

1.  **Crie e compile um arquivo Java:**
    ```bash
    javac TestIntegration.java
    ```
2.  **Execute o visualizador/debug com o novo arquivo:**
    ```bash
    ./visualizador-bytecode TestIntegration.class -debug
    ```
    *(O modo `-debug` inicia a execução rudimentar da JVM no arquivo.)*

-----

## 🛠️ Etapas de Desenvolvimento (Testes Unitários)

Durante o desenvolvimento, foram usadas diversas etapas para testar módulos específicos, que podem ser executadas no Windows através do MinGW/GCC (ou no WSL, se configurado).

### Parte 1: Teste de Pilha (Stack)

Testa a implementação da estrutura de dados de pilha (usada para a Pilha de Operandos):

```bash
# Compila e cria o executável 'test_stack'
gcc -c -std=c99 -Wall -Wextra -Iinclude src/stack.c -o src/stack.o
gcc -std=c99 -Iinclude -o test_stack src/test_stack.c src/stack.c src/jvm.c src/classfile.c src/attributes.c src/io.c

# Executa os testes
./test_stack
```

### Parte 2: Teste de Empilhamento (Stack Flow)

Testa operações básicas de manipulação de pilha (assumindo que o binário `teststacks` já existe):

```bash
./teststacks
```

### Parte 3: Teste de Fluxo de Execução

Testa o fluxo de execução principal e as interações com o gerenciador de memória:

```bash
gcc -g test_pessoa3.c exec_flow.c memory_manager_mock.c class_manager_mock.c -o test_flow
./test_flow
```

### Parte 4: Teste de Gerenciamento de Heap

Testa a implementação do gerenciador de memória (Heap), que simula a alocação de objetos:

```bash
gcc -g test_pessoa4.c heap_manager.c class_loader_mock.c -o test_heap_manager
./test_heap_manager
```
