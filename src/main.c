/* --- src/main.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Módulos principais do projeto
#include "base.h"
#include "cli.h"        // (Pessoa E) Nosso novo módulo de CLI
#include "io.h"         // (Pessoa A) Para carregar o arquivo
#include "classfile.h"  // (Pessoa B) Para o parser
#include "print.h"      // (Pessoa E) Para saida --pretty
#include "json.h"       // (Pessoa E) Para saida --json
#include "jvm.h"        // Novo: Estruturas da JVM
#include "execute.h"    // Novo: Execução

/* logger condicional: escreve no stderr quando --verbose */
#define VLOG(opt_ptr, fmt, ...) \
    do { if ((opt_ptr) && (opt_ptr)->verbose) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__); } while (0)


/**
 * @brief Roda o fluxo principal do programa apos a validacao dos argumentos.
 */
static int run_viewer(const CliOptions *options) {
    Status io_status;
    ClassFileStatus cf_status;
    Buffer buffer;
    ClassFile class_file;

    memset(&buffer, 0, sizeof(Buffer));
    memset(&class_file, 0, sizeof(ClassFile));

    /* A) carregar arquivo */
    VLOG(options, "Abrindo arquivo: %s", options->input_file);
    io_status = buffer_from_file(options->input_file, &buffer);
    if (io_status != OK) {
        fprintf(stderr, "Erro (IO): Nao foi possivel ler o arquivo '%s'. Codigo: %d\n",
                options->input_file, io_status);
        return 1;
    }
    VLOG(options, "Arquivo lido (%u bytes)", (unsigned)buffer.size);

    /* B) parse do .class */
    VLOG(options, "Iniciando parse do ClassFile");
    cf_status = parse_classfile(&class_file, &buffer);
    buffer_free(&buffer); /* buffer nao necessario apos parse */

    if (cf_status != CF_STATUS_OK) {
        fprintf(stderr, "Erro (Parser): Falha ao analisar o arquivo .class. Codigo: %d\n", cf_status);
        free_classfile(&class_file);
        return 1;
    }

    /* logs de cabecalho basicos (seguro imprimir depois do parse) */
    VLOG(options, "Magic = 0x%08X (esperado 0xCAFEBABE)", class_file.magic);
    VLOG(options, "Versao = %u.%u (major=%u, minor=%u)",
         (unsigned)class_file.major_version, (unsigned)class_file.minor_version,
         (unsigned)class_file.major_version, (unsigned)class_file.minor_version);
    VLOG(options, "ConstantPool = %u entradas; methods=%u; fields=%u; attributes=%u",
         (unsigned)class_file.constant_pool_count,
         (unsigned)class_file.methods_count,
         (unsigned)class_file.fields_count,
         (unsigned)class_file.attributes_count);

    /* E) imprimir resultado */
    const char *mode = (options->output_mode == OUTPUT_MODE_JSON) ? "json" : "pretty";
    VLOG(options, "Gerando saida (%s). disassemble_code=%s",
         mode, options->disassemble_code ? "true" : "false");

    // F) Executar código se solicitado (-run ou -debug)
    if (options->execution_mode != MODE_NONE) {
        const char *exec_mode = (options->execution_mode == MODE_DEBUG) ? "DEBUG" : "EXECUTE";
        VLOG(options, "Modo de execucao: %s", exec_mode);
        
        int exec_status = execute_main_method(&class_file, options);
        free_classfile(&class_file);
        return exec_status;
    }

    if (options->output_mode == OUTPUT_MODE_READER) {
        // Modo Leitor: Apenas lê e analisa, sem exibir.
        io_status = OK;
        VLOG(options, "Modo Leitor ativado. Nenhuma saida gerada.");
    } else {
        io_status = (options->output_mode == OUTPUT_MODE_JSON)
                  ? json_classfile(&class_file, options)
                  : print_classfile(&class_file, options);
    }

    if (io_status != OK) {
        fprintf(stderr, "Erro (Impressao): Falha ao gerar a saida. Codigo: %d\n", io_status);
    } else {
        VLOG(options, "Concluido com sucesso");
    }

    free_classfile(&class_file);
    return (io_status == OK) ? 0 : 1;
}


int main(int argc, char *argv[]) {
    
    CliOptions options;
    parse_cli_options(argc, argv, &options);

    // Se parse_cli_options encontrou um erro (ex: sem input)
    if (options.error) {
        // A mensagem de erro já foi impressa pelo cli.c
        return 1; 
    }
    
    // Se o usuário pediu --help
    if (options.show_help) {
        // A mensagem de ajuda já foi impressa pelo cli.c
        return 0; // Sai com sucesso
    }

    // Se os argumentos são válidos, executa o programa
    return run_viewer(&options);
}