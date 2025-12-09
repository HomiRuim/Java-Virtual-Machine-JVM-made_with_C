# Guia Rápido de Uso - JVM Integrada

## Pré-requisitos

Para compilar e executar a JVM, você precisa ter instalado:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y gcc gcc-multilib g++-multilib default-jdk
```

## Compilação

### 1. Limpar compilação anterior
```bash
make clean
```

### 2. Compilar o projeto
```bash
make
```

Isso irá gerar o executável `visualizador-bytecode` compilado em 32 bits.

## Uso Básico

### Executar um arquivo .class
```bash
./visualizador-bytecode arquivo.class -run
```

### Executar com modo debug (mostra cada instrução)
```bash
./visualizador-bytecode arquivo.class -debug
```

### Visualizar bytecode sem executar
```bash
./visualizador-bytecode arquivo.class --pretty
```

### Exportar para JSON
```bash
./visualizador-bytecode arquivo.class --json
```

## Teste de Integração

Um arquivo de teste já está incluído: `TestIntegration.java`

### Compilar o teste
```bash
javac TestIntegration.java
```

### Executar o teste
```bash
./visualizador-bytecode TestIntegration.class -debug
```

Este teste verifica:
- ✅ Operações aritméticas (iadd, isub, imul, idiv, irem, ineg)
- ✅ Controle de fluxo (if-else)
- ✅ TABLESWITCH (switch-case)

## Funcionalidades Integradas

### Pessoa 4 - Gerenciamento de Heap
- **NEW (0xBB)**: Cria novos objetos
- **NEWARRAY (0xBC)**: Cria arrays primitivos
- **GETFIELD (0xB4)**: Lê campos de objetos
- **PUTFIELD (0xB5)**: Escreve campos de objetos
- **ARETURN (0xB0)**: Retorna referência de objeto

### Instrução TABLESWITCH
- **TABLESWITCH (0xAA)**: Switch otimizado com tabela de saltos

## Estrutura do Projeto

```
Ultimate-JVM-25.2-main/
├── include/              # Headers
│   ├── heap_manager.h   # [NOVO] Gerenciamento de heap
│   └── ...
├── src/                 # Código fonte
│   ├── heap_manager.c   # [NOVO] Implementação do heap
│   ├── execute.c        # [MODIFICADO] Novos opcodes
│   └── ...
├── makefile             # [MODIFICADO] Compilação em 32 bits
├── TestIntegration.java # Arquivo de teste
└── RELATORIO_INTEGRACAO.md  # Relatório completo
```

## Opções de Linha de Comando

```
Uso: ./visualizador-bytecode [opcoes] <arquivo.class>

Opções principais:
  --pretty         Formata a saída de forma legível (padrão)
  --reader-mode    Funciona apenas como leitor (sem exibição)
  --json           Formata a saída como um objeto JSON
  --no-code        Oculta o disassembly do bytecode dos métodos
  -run             Executa o método main da classe
  -debug           Executa o método main com saída de depuração
  --help, -h       Mostra esta mensagem de ajuda
  --verbose        Mostra logs de depuração no stderr
```

## Exemplos de Uso

### Exemplo 1: Executar programa Java simples
```bash
# Criar arquivo Java
cat > Hello.java << 'EOF'
public class Hello {
    public static void main(String[] args) {
        int x = 10;
        int y = 20;
        int z = x + y;
    }
}
EOF

# Compilar
javac Hello.java

# Executar com debug
./visualizador-bytecode Hello.class -debug
```

### Exemplo 2: Testar TABLESWITCH
```bash
# O arquivo TestIntegration.java já contém um switch-case
javac TestIntegration.java
./visualizador-bytecode TestIntegration.class -debug | grep TABLESWITCH
```

### Exemplo 3: Visualizar estrutura do .class
```bash
./visualizador-bytecode TestIntegration.class --pretty
```

## Solução de Problemas

### Erro: "gcc: command not found"
```bash
sudo apt-get install -y gcc gcc-multilib
```

### Erro: "bits/libc-header-start.h: No such file"
```bash
sudo apt-get install -y gcc-multilib g++-multilib
```

### Erro: "javac: command not found"
```bash
sudo apt-get install -y default-jdk
```

### Aviso: "Clock skew detected"
Este é um aviso inofensivo relacionado ao timestamp dos arquivos. Pode ser ignorado.

## Observações Importantes

1. **Compilação em 32 bits**: O projeto está configurado para compilar em 32 bits conforme requisito do professor.

2. **Simplificações**: 
   - Objetos são alocados com 10 campos por padrão
   - Offset de campos usa simplificação `index % 10`
   - Não há garbage collection

3. **TABLESWITCH**: Implementado com alinhamento correto e suporte completo a tabelas de salto.

## Suporte

Para mais detalhes sobre a integração, consulte:
- `RELATORIO_INTEGRACAO.md` - Relatório completo da integração
- `Guia.pdf` - Especificação técnica original
- `Observacoes.txt` - Observações do professor

---

**Última atualização:** 08 de Dezembro de 2025
