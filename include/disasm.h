// include/disasm.h

#ifndef DISASM_H
#define DISASM_H
#include "attributes.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
// TIPOS E ESTRUTURAS AUXILIARES
// --------------------------------------------------------------------------

// Forward declarations para evitar a inclusao de headers de outros modulos aqui
typedef struct class_file ClassFile;



/**
 * @brief Representa uma instrucao de bytecode apos o desassembleamento.
 */
typedef struct {
    uint32_t pc;        // Program Counter (endereco) onde a instrucao comeca.
    uint8_t length;     // Comprimento total da instrucao (opcode + args).
    
    // Mnemonic (Ex: "invokestatic")
    char *mnemonic;     
    
    // String bruta dos argumentos (Ex: "#5" ou "10, -3")
    char *args_str;     
    
    // Informacao resolvida (Ex: "System.out:LPrintStream;") - ALOCADA
    char *resolved_info;
} DisasmInstruction;


/**
 * @brief Estrutura de output que contem todas as instrucoes de um metodo.
 */
typedef struct {
    DisasmInstruction *instructions;
    uint32_t count;
} DisasmOutput;


// --------------------------------------------------------------------------
// API PÚBLICA (Contrato da Pessoa D)
// --------------------------------------------------------------------------

/**
 * @brief Executa o desassembleamento do Bytecode contido em um CodeAttribute.
 *
 * Le o array de bytes do codigo, traduz cada opcode para o mnemônico
 * correspondente, le e resolve os argumentos, e popula a estrutura DisasmOutput.
 *
 * @param cf Estrutura ClassFile completa (necessaria para resolucao do CP).
 * @param code_attr O atributo Code do metodo (contem o array de bytes).
 * @param output Ponteiro para a estrutura onde os resultados serao armazenados.
 * @return true em sucesso, false em erro de alocacao de memoria.
 */
bool disassemble_method(ClassFile *cf, CodeAttribute *code_attr, DisasmOutput *output);


/**
 * @brief Libera a memoria alocada pela estrutura DisasmOutput e suas strings.
 *
 * Deve ser chamada pela Pessoa E após a impressao para evitar vazamentos.
 *
 * @param output A estrutura DisasmOutput a ser liberada.
 */
void free_disasm_output(DisasmOutput *output);


#endif // DISASM_H