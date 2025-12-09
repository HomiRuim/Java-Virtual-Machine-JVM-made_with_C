#ifndef CLI_H
#define CLI_H

#include <stdbool.h> // Para usar true/false

/**
 * @brief Define os modos de saída principais.
 */
typedef enum {
    OUTPUT_MODE_PRETTY, // Impressão legível [cite: 41]
    OUTPUT_MODE_JSON,    // Impressão em JSON [cite: 42]
    OUTPUT_MODE_READER   // Modo Leitor (apenas leitura, sem exibição)
} OutputMode;

/**
 * @brief Define os modos de execução da JVM.
 */
typedef enum {
    MODE_NONE,      // Sem execução (apenas visualização)
    MODE_EXECUTE,   // Execução normal (flag -run)
    MODE_DEBUG      // Execução com depuração (flag -debug)
} ExecutionMode;


/**
 * @brief Estrutura para armazenar todas as opções parseadas da linha de comando.
 */
typedef struct {
    const char *input_file; // Caminho para o arquivo .class
    OutputMode output_mode;
    bool is_reader_mode; // Flag para o modo leitor (sem exibição)

    // Flags de controle de impressão (baseado nas flags --cp, --no-code, etc) 
    bool print_header;
    bool print_constant_pool;
    bool print_interfaces;
    bool print_fields;
    bool print_methods;
    bool print_attributes;
    bool disassemble_code; // Controlado por --no-code 

    // Modo de execução da JVM (Pessoa 2)
    ExecutionMode execution_mode; // MODE_NONE, MODE_EXECUTE, MODE_DEBUG

    // Status
    bool show_help;
    bool error;
    const char *error_message;

    bool verbose;

} CliOptions;

/**
 * @brief Analisa os argumentos da linha de comando (argc, argv) 
 * e preenche a estrutura CliOptions.
 *
 * @param argc Contagem de argumentos (de main).
 * @param argv Vetor de argumentos (de main).
 * @param options Ponteiro para a estrutura a ser preenchida.
 */
void parse_cli_options(int argc, char *argv[], CliOptions *options);

/**
 * @brief Imprime a mensagem de uso/ajuda do programa.
 *
 * @param prog_name O nome do executável (argv[0]).
 */
void print_cli_usage(const char *prog_name);

#endif // CLI_H