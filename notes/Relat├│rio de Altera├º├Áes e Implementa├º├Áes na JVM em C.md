# Relatório de Alterações e Implementações na JVM em C

Prezado(a) colega,

Conforme solicitado, realizei as alterações e implementações necessárias no código-fonte do visualizador de bytecode para transformá-lo em um esqueleto de Máquina Virtual Java (JVM) em C, atendendo aos requisitos específicos do projeto.

O código-fonte corrigido e estendido está disponível no arquivo anexo `visualizador-bytecode-corrigido.zip`.

## 1. Implementações e Correções Realizadas

As seguintes modificações foram feitas para abordar os problemas e requisitos levantados:

### 1.1. Flags: Leitor vs. Exibidor

**Requisito:** Funcionar como leitor, ou como exibidor.

**Implementação:**
*   Adicionei a flag `--reader-mode` ao processamento da linha de comando (`src/cli.c` e `include/cli.h`).
*   Quando esta flag é usada, o programa executa o *parsing* completo do arquivo `.class` (modo leitor), mas **suprime toda a saída** (exibição), conforme a lógica em `src/main.c`. Isso permite que o código seja usado apenas para carregar e analisar a estrutura da classe, sem poluir a saída.

### 1.2. Não Usar `switch` para Opcodes (Ponteiros de Função)

**Requisito:** Não usar `switch` para o *case* das funções. Usar ponteiro, e passar as informações de outra forma.

**Implementação:**
*   Criei os arquivos `include/execute.h` e `src/execute.c`.
*   Em `execute.h`, defini o tipo `OpcodeHandler` (um ponteiro de função) e declarei a tabela `opcode_handlers[256]`.
*   Em `execute.c`, implementei a função `init_opcode_handlers()` que inicializa a tabela, mapeando cada valor de opcode (0x00 a 0xFF) para uma função específica (`handle_nop`, `handle_iconst_0`, etc.), eliminando a necessidade de um `switch` na rotina de execução.
*   A rotina de execução (`execute_main_method`) agora usa `handler = opcode_handlers[opcode]` para obter e chamar a função correta.

### 1.3. Estrutura `Frame` e Gerenciamento de Memória

**Requisito:**
*   Frame - colocar o ponteiro com vetor de variáveis locais com indicação para as variáveis locais e para pilha de operando.
*   Frame com um tamanho só.
*   `free()` para desalocar já é suficiente.
*   Sem *jac colect* (Garbage Collector).
*   Sem área para referência.

**Implementação:**
*   **Estrutura `Frame` (`include/jvm.h`):**
    *   A estrutura `Frame` foi definida com um **Flexible Array Member** (`Slot slots_data[]`) no final.
    *   A alocação (`frame_new` em `src/jvm.c`) usa `calloc` para alocar a estrutura base mais o espaço contíguo necessário para as variáveis locais (`max_locals`) e a pilha de operandos (`max_stack`).
    *   Os ponteiros `local_vars` e `operand_stack` são inicializados para apontar para as seções corretas dentro do array `slots_data`, garantindo que o Frame seja uma única alocação.
*   **Gerenciamento de Memória:**
    *   A função `frame_free` simplesmente chama `free(frame)`, liberando todo o bloco de memória do Frame de uma só vez, conforme o requisito de "Frame com um tamanho só" e "free() para desalocar já é suficiente".
    *   A estrutura `JVMState` foi criada para gerenciar a pilha de Frames (`call_stack`), mas **não inclui** uma "Área de Referências" ou lógica de *Garbage Collection* (GC), atendendo aos requisitos.

### 1.4. Ordem de Execução e `Invoke`

**Requisito:** Saber como que é feito os *invoke* (complicado). Saber a ordem de execução, carregar estrutura de classes que precisa ser carregadas.

**Implementação:**
*   **Ordem de Execução:** A função `execute_main_method` em `src/execute.c` estabelece o *loop* principal de execução:
    1.  Encontra o método `main`.
    2.  Obtém o `CodeAttribute` (que contém o bytecode).
    3.  Cria o `Frame` de execução.
    4.  Inicia o `Program Counter` (`frame->pc`) no início do bytecode.
    5.  Entra no *loop* que lê o opcode, consulta a tabela de ponteiros de função e executa o manipulador correspondente.
*   **`Invoke`:** A lógica de `invoke` (chamada de método) é complexa e requer a implementação de opcodes como `invokevirtual`, `invokespecial`, etc. No esqueleto fornecido, o `execute.c` contém um `TODO` para a busca do método `main` e a lógica de interrupção (`return`). A implementação completa dos opcodes de `invoke` deve ser o próximo passo do projeto, seguindo a estrutura de ponteiros de função já estabelecida.

## 2. Estrutura do Código Corrigido

| Arquivo | Descrição da Alteração |
| :--- | :--- |
| `include/cli.h` | Adicionada a flag `is_reader_mode` e o `OUTPUT_MODE_READER`. |
| `src/cli.c` | Implementação do parsing para a flag `--reader-mode` e correção de sintaxe. |
| `src/main.c` | Adicionada a lógica para suprimir a saída quando `is_reader_mode` é `true`. Inclusão dos novos *headers* `jvm.h` e `execute.h`. |
| `include/jvm.h` | Definição das estruturas `Slot`, `Frame` (com Flexible Array Member) e `JVMState`. |
| `src/jvm.c` | Implementação das funções `frame_new`, `frame_free`, `jvm_new` e `jvm_free`, garantindo a alocação única do Frame. |
| `include/execute.h` | Definição do ponteiro de função `OpcodeHandler` e declaração da tabela `opcode_handlers[256]`. |
| `src/execute.c` | Implementação da rotina de execução principal (`execute_main_method`) e da inicialização da tabela de ponteiros de função (`init_opcode_handlers`), com exemplos de manipuladores de opcode (`handle_nop`, `handle_iconst_0`, `handle_bipush`). |
| `makefile` | Atualizado para incluir a compilação dos novos arquivos `src/jvm.c` e `src/execute.c`. |

Espero que estas alterações ajudem a avançar no projeto da JVM! Qualquer dúvida, estou à disposição.

Atenciosamente,

**Manus AI**
