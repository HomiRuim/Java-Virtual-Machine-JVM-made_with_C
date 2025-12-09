#include "cli.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Imprime a mensagem de uso/ajuda do programa.
 */
void print_cli_usage(const char *prog_name) {
    fprintf(stderr, "Uso: %s [opcoes] <arquivo.class>\n\n", prog_name);
    fprintf(stderr, "Opcoes principais:\n");
    fprintf(stderr, "  --pretty         Formata a saida de forma legivel (padrao).\n");
    fprintf(stderr, "  --reader-mode    Funciona apenas como leitor (sem exibição).\n");
    fprintf(stderr, "  --json           Formata a saida como um objeto JSON.\n");
    fprintf(stderr, "  --no-code        Oculta o disassembly do bytecode dos metodos.\n");
    fprintf(stderr, "  -run             Executa o metodo main da classe.\n");
    fprintf(stderr, "  -debug           Executa o metodo main com saida de depuracao.\n");
    fprintf(stderr, "  --help, -h       Mostra esta mensagem de ajuda.\n");
    fprintf(stderr, "  --verbose        Mostra logs de depuracao no stderr.\n");

    
    /* Nota: O documento de divisao  tambem menciona --cp (constant pool), etc. 
     * Essas flags podem ser adicionadas aqui e na funcao parse_cli_options 
     * para controlar as flags bool 'print_constant_pool', 'print_fields', etc.
     */
}

/**
 * @brief Define os valores padrao para a estrutura CliOptions.
 */
static void set_default_options(CliOptions *options) {
    options->input_file = NULL;
    options->output_mode = OUTPUT_MODE_PRETTY; // "pretty" é o padrão
    options->is_reader_mode = false; // Modo exibidor é o padrão
    

    // Por padrão, imprimimos tudo
    options->print_header = true;
    options->print_constant_pool = true;
    options->print_interfaces = true;
    options->print_constant_pool = true;
    options->print_fields = true;
    options->print_methods = true;
    options->print_attributes = true;
    options->disassemble_code = true; // "no-code"  desativa isso

    // Modo de execução padrão: nenhum
    options->execution_mode = MODE_NONE;

    options->show_help = false;
    options->error = false;
    options->error_message = NULL;
    options->verbose = false;
}

/**
 * @brief Analisa os argumentos da linha de comando (argc, argv) 
 * e preenche a estrutura CliOptions.
 */
void parse_cli_options(int argc, char *argv[], CliOptions *options) {
    
    set_default_options(options);
    
    const char *prog_name = argv[0];

    // Itera por todos os argumentos, exceto o nome do programa
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "--json") == 0) {
            options->output_mode = OUTPUT_MODE_JSON;
        } else if (strcmp(arg, "--pretty") == 0) {
            options->output_mode = OUTPUT_MODE_PRETTY;
        } else if (strcmp(arg, "--reader-mode") == 0) {
            options->is_reader_mode = true;
            options->output_mode = OUTPUT_MODE_READER; // O modo leitor não exibe nada
        } else if (strcmp(arg, "--no-code") == 0) {
            options->disassemble_code = false;
        } else if (strcmp(arg, "-run") == 0) {
            options->execution_mode = MODE_EXECUTE;
        } else if (strcmp(arg, "-debug") == 0) {
            options->execution_mode = MODE_DEBUG;
        } else if (strcmp(arg, "--verbose") == 0) {
            options->verbose = true;
        } else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            options->show_help = true;
        } else if (arg[0] == '-') {
            // Flag desconhecida
            fprintf(stderr, "Aviso: Flag '%s' desconhecida ou nao implementada.\n", arg);
        } else {
            // Nao é uma flag, deve ser o arquivo de entrada
            if (options->input_file != NULL) {
                options->error = true;
                options->error_message = "Erro: Multiplos arquivos de entrada nao sao suportados.";
                print_cli_usage(prog_name);
                return;
            }
            options->input_file = arg;
        }
    }

    // Validacao final
    if (options->show_help) {
        print_cli_usage(prog_name);
        return; // Nao processa mais nada se for --help
    }

    if (options->input_file == NULL && !options->error) {
        options->error = true;
        options->error_message = "Erro: Nenhum arquivo .class fornecido.";
        print_cli_usage(prog_name);
    }
}