
# Bytecode Viewer (Java Class File Visualizer)

Um visualizador e analisador de arquivos **`.class`** (bytecode Java), desenvolvido em **C**, que interpreta o formato binário do Java ClassFile e exibe suas estruturas internas em formato legível ou JSON.
O projeto inclui um sistema de build automatizado via `make`, suporte a testes de integração com “golden files” e logging opcional de depuração.

---

## Estrutura do Projeto

```
visualizador-bytecode/
│
├── include/                # Headers (.h) de cada módulo
│   ├── attributes.h
│   ├── base.h
│   ├── classfile.h
│   ├── cli.h
│   ├── disasm.h
│   ├── io.h
│   ├── json.h
│   ├── print.h
│   └── resolve.h
│
├── src/                    # Implementações em C
│   ├── main.c              # Ponto de entrada (parse CLI, orquestra o fluxo)
│   ├── cli.c               # Parser de argumentos da linha de comando
│   ├── io.c                # Leitura e bufferização do arquivo .class
│   ├── classfile.c         # Parsing do formato binário ClassFile
│   ├── attributes.c        # Leitura dos atributos do ClassFile
│   ├── parse_code.c        # Parse do atributo "Code" e bytecode interno
│   ├── resolve.c           # Resolução de referências no Constant Pool
│   ├── disasm.c            # Disassembly de bytecode (mnemonics)
│   ├── print.c             # Impressão legível (modo pretty)
│   ├── json.c              # Geração de saída JSON
│   ├── test_attributes.c   # Teste unitário auxiliar
│   ├── validate_class.c    # Teste de validação do arquivo .class
│   └── ...
│
├── tests/
│   ├── samples/            # Arquivos de entrada de teste (.class, .java)
│   │   ├── Example.class
│   │   └── Example.java
│   ├── golden/             # Saídas de referência ("golden files")
│       ├── Example.pretty.golden
│       └── Example.json.golden
│   
│
├── build/                  # Diretórios de build/test são criados automaticamente
│   └── test/
│
├── makefile                # Sistema de build e testes cross-platform
└── README.md               # Este arquivo
```

---

## ⚙️ Requisitos

* **Compilador C (GCC ou Clang)**
* **Make** (GNU Make 4.x+)
* Windows, Linux ou macOS

  * No Windows, usa **PowerShell** para criar diretórios e comparar arquivos.

---

## 🏗️ Compilação

Para gerar o executável principal (`visualizador-bytecode`):

```bash
make
```

Saída esperada:

```
Executavel principal 'visualizador-bytecode.exe' criado com sucesso.
```

Isso compila todos os arquivos `.c` e pode gerar dependências automáticas (`.d`) para recompilação incremental.

---

## Testes de Integração

O projeto inclui testes automatizados que comparam a saída do programa com arquivos “golden” (resultados esperados).

### Rodar todos os testes:

```bash
make test
```

Exemplo de saída:

```
[TEST pretty] tests/samples/Example.class
Diff OK
[TEST json] tests/samples/Example.class
Diff OK
"Todos os testes de integracao passaram."
```

Se houver diferença entre a saída e o golden:

```
--- MISMATCH ---
Golden:  tests/golden/Example.json.golden
Output:  build/test/Example.json.out
Primeiras diferencas:
InputObject                SideIndicator
-----------                -------------
"magic": "0xCAFEBABE",     =>
"magic": "0xCAFEBAB0"      <=
```

### Atualizar arquivos “golden”:

Se você mudou a formatação de saída e quer atualizar as referências:

```bash
make golden-update
```

---

## Limpeza

Para remover todos os binários e arquivos intermediários:

```bash
make clean
```

---

## Execução do Programa

Depois de compilado, o binário principal é `visualizador-bytecode` (ou `visualizador-bytecode.exe` no Windows).

### Uso básico

```bash
./visualizador-bytecode <arquivo.class>
```

### Opções disponíveis

```
Uso: visualizador-bytecode [opcoes] <arquivo.class>

Opcoes principais:
  --pretty         Formata a saida de forma legivel (padrao).
  --json           Formata a saida como um objeto JSON.
  --no-code        Oculta o disassembly do bytecode dos metodos.
  --verbose        Mostra logs de depuracao no stderr.
  --help, -h       Mostra esta mensagem de ajuda.
```

### Exemplos

#### Saída legível (default)

```bash
./visualizador-bytecode tests/samples/Example.class
```

#### Saída JSON

```bash
./visualizador-bytecode tests/samples/Example.class --json
```

#### Ocultar bytecode dos métodos

```bash
./visualizador-bytecode tests/samples/Example.class --no-code
```

#### Logs detalhados (stderr)

```bash
./visualizador-bytecode tests/samples/Example.class --verbose
```

Saída:

```
[DEBUG] Abrindo arquivo: tests/samples/Example.class
[DEBUG] Arquivo lido (708 bytes)
[DEBUG] Iniciando parse do ClassFile
[DEBUG] Magic = 0xCAFEBABE (esperado 0xCAFEBABE)
[DEBUG] Versao = 69.0 (major=69, minor=0)
[DEBUG] ConstantPool = 29 entradas; methods=2; fields=0; attributes=1
[DEBUG] Gerando saida (pretty). disassemble_code=true
[DEBUG] Concluido com sucesso
```

Os logs aparecem apenas no `stderr`, então não alteram a saída “real” do programa — isso garante que os testes continuem funcionando.

---

##  Estrutura Interna

O fluxo principal (`main.c`) segue as etapas:

1. **CLI Parsing** (`cli.c`)
   Interpreta as opções (`--json`, `--no-code`, `--verbose`, etc.) e prepara a struct `CliOptions`.

2. **Leitura do Arquivo** (`io.c`)
   Carrega o conteúdo binário do `.class` em memória (`Buffer`).

3. **Parsing do ClassFile** (`classfile.c`)
   Analisa o cabeçalho (`magic`, versões), constant pool, campos, métodos e atributos.

4. **Disassembly e Atributos**

   * `parse_code.c` e `disasm.c`: analisam e traduzem bytecodes em mnemonics.
   * `attributes.c`: trata atributos comuns (`Code`, `LineNumberTable`, etc.).

5. **Impressão / Saída**

   * `print.c`: modo “pretty” (texto legível).
   * `json.c`: modo estruturado (JSON).

6. **Logs (`--verbose`)**

   * São emitidos no `stderr` com prefixo `[DEBUG]`, sem interferir na saída normal (`stdout`).

---

##  Dicas

* Para debug detalhado, combine flags:

  ```bash
  make clean && make && ./visualizador-bytecode --json --verbose tests/samples/Example.class
  ```

* O campo `magic` correto em um `.class` sempre deve ser:

  ```
  0xCAFEBABE
  ```

* Os testes “golden” garantem estabilidade da saída entre versões — só use `make golden-update` quando você **quiser mudar** o formato de saída oficial.

---


## Compilador e padrão

* Compilador: **GCC 11+**
* Padrão C: **C99**
* Flags recomendadas: `-Wall -Wextra -O2`
* Sem dependências externas.
* Arquivos `.class` podem ser gerados com:

```bash
javac  Example.java
```

---

## Referências

* *The Java Virtual Machine Specification – Java SE 8 Edition*
  [https://docs.oracle.com/javase/specs/jvms/se8/html/](https://docs.oracle.com/javase/specs/jvms/se8/html/)

* [jclasslib Bytecode Viewer](https://github.com/ingokegel/jclasslib)

* Materiais da disciplina de Software Basico

---

## Autores

Projeto desenvolvido por:

| Membro   | Responsável por                  |
| -------- | -------------------------------- |
| Pessoa 1 | IO / Base                        |
| Pessoa 2 | Parser ClassFile / Constant Pool |
| Pessoa 3 | Atributos (Code)                 |
| Pessoa 4 | Disassembler / Resolve           |
| Pessoa 5 | CLI / Print / JSON / Testes      |
