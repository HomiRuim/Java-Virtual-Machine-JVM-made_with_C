#ifndef EXECUTE_H
#define EXECUTE_H

#include "jvm.h"
#include "cli.h" // Para CliOptions

/**
 * @brief Assinatura de um manipulador de opcode.
 *
 * Cada função recebe o Frame atual e as opções de CLI (para modo debug).
 * É responsável por:
 * 1. Ler os operandos do bytecode (se houver) a partir de frame->pc.
 * 2. Executar a operação (manipular a pilha de operandos e/ou variáveis locais).
 * 3. Atualizar o Program Counter (frame->pc) para o próximo opcode.
 *
 * @param frame O Frame de execução atual.
 * @param options As opções de CLI (para debug).
 * @return 0 em caso de sucesso, 1 para sinalizar return, ou um código negativo para erro.
 */
typedef int (*OpcodeHandler)(Frame *frame, const CliOptions *options);

/**
 * @brief Tabela de ponteiros de função para os 256 opcodes (Dispatch Table).
 *
 * O índice da tabela corresponde ao valor do opcode (0x00 a 0xFF).
 * Esta abordagem substitui grandes estruturas switch para melhor performance.
 */
extern OpcodeHandler opcode_handlers[256];

/**
 * @brief Inicializa a tabela de manipuladores de opcode.
 */
void init_opcode_handlers();

/**
 * @brief Executa o método principal (main) da classe carregada.
 *
 * @param class_file O ClassFile carregado.
 * @param options As opções de linha de comando.
 * @return O código de saída da JVM (0 para sucesso).
 */
int execute_main_method(ClassFile *class_file, const CliOptions *options);

#endif // EXECUTE_H
