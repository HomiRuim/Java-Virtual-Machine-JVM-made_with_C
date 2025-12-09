#ifndef STACK_H
#define STACK_H

#include "jvm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * OPERAÇÕES DA PILHA DE OPERANDOS (Operand Stack)
 * ============================================================================ */

/**
 * @brief Empilha um valor de 32 bits na pilha de operandos.
 * 
 * @param frame O frame atual
 * @param value O valor a ser empilhado
 * @return 0 em sucesso, -1 se houver stack overflow
 */
int frame_push(Frame *frame, Slot value);

/**
 * @brief Desempilha um valor de 32 bits da pilha de operandos.
 * 
 * @param frame O frame atual
 * @param out Ponteiro para armazenar o valor desempilhado
 * @return 0 em sucesso, -1 se a pilha estiver vazia
 */
int frame_pop(Frame *frame, Slot *out);

/**
 * @brief Obtém o valor no topo da pilha sem desempilhar.
 * 
 * @param frame O frame atual
 * @param out Ponteiro para armazenar o valor
 * @return 0 em sucesso, -1 se a pilha estiver vazia
 */
int frame_peek(Frame *frame, Slot *out);

/**
 * @brief Verifica se a pilha de operandos está vazia.
 * 
 * @param frame O frame atual
 * @return 1 se vazia, 0 caso contrário
 */
int frame_is_stack_empty(Frame *frame);

/**
 * @brief Retorna o número de elementos na pilha de operandos.
 * 
 * @param frame O frame atual
 * @return Número de elementos
 */
int frame_stack_size(Frame *frame);

/**
 * @brief Empilha um valor de 64 bits (Long/Double) na pilha de operandos.
 * 
 * Valores de 64 bits ocupam 2 slots: primeiro o high word, depois o low word.
 * 
 * @param frame O frame atual
 * @param high_word Os 32 bits superiores
 * @param low_word Os 32 bits inferiores
 * @return 0 em sucesso, -1 se houver stack overflow
 */
int frame_push_long(Frame *frame, Slot high_word, Slot low_word);

/**
 * @brief Desempilha um valor de 64 bits (Long/Double) da pilha de operandos.
 * 
 * @param frame O frame atual
 * @param high_word Ponteiro para armazenar os 32 bits superiores
 * @param low_word Ponteiro para armazenar os 32 bits inferiores
 * @return 0 em sucesso, -1 se a pilha não tiver elementos suficientes
 */
int frame_pop_long(Frame *frame, Slot *high_word, Slot *low_word);

/* ============================================================================
 * OPERAÇÕES DE VARIÁVEIS LOCAIS (Local Variables)
 * ============================================================================ */

/**
 * @brief Define o valor de uma variável local.
 * 
 * @param frame O frame atual
 * @param index Índice da variável local (0 a max_locals-1)
 * @param value O valor a ser armazenado
 * @return 0 em sucesso, -1 se o índice for inválido
 */
int frame_set_local(Frame *frame, u2 index, Slot value);

/**
 * @brief Obtém o valor de uma variável local.
 * 
 * @param frame O frame atual
 * @param index Índice da variável local (0 a max_locals-1)
 * @param out Ponteiro para armazenar o valor
 * @return 0 em sucesso, -1 se o índice for inválido
 */
int frame_get_local(Frame *frame, u2 index, Slot *out);

/**
 * @brief Define um valor de 64 bits em uma variável local.
 * 
 * Valores de 64 bits ocupam 2 slots consecutivos: index e index+1.
 * 
 * @param frame O frame atual
 * @param index Índice inicial da variável local
 * @param high_word Os 32 bits superiores
 * @param low_word Os 32 bits inferiores
 * @return 0 em sucesso, -1 se o índice for inválido
 */
int frame_set_local_long(Frame *frame, u2 index, Slot high_word, Slot low_word);

/**
 * @brief Obtém um valor de 64 bits de uma variável local.
 * 
 * @param frame O frame atual
 * @param index Índice inicial da variável local
 * @param high_word Ponteiro para armazenar os 32 bits superiores
 * @param low_word Ponteiro para armazenar os 32 bits inferiores
 * @return 0 em sucesso, -1 se o índice for inválido
 */
int frame_get_local_long(Frame *frame, u2 index, Slot *high_word, Slot *low_word);

/* ============================================================================
 * OPERAÇÕES DA CALL STACK (Pilha de Frames)
 * ============================================================================ */

/**
 * @brief Empilha um novo frame na call stack.
 * 
 * Cria um novo frame e o adiciona ao topo da pilha de frames.
 * O novo frame se torna o frame atual.
 * 
 * @param jvm O estado da JVM
 * @param class_file O ClassFile da classe do método
 * @param method_info O MethodInfo do método a ser executado
 * @param max_locals Número máximo de variáveis locais
 * @param max_stack Tamanho máximo da pilha de operandos
 * @return 0 em sucesso, -1 em caso de erro de alocação
 */
int jvm_push_frame(JVMState *jvm, ClassFile *class_file, MethodInfo *method_info,
                   u2 max_locals, u2 max_stack);

/**
 * @brief Desempilha o frame atual da call stack.
 * 
 * Remove e libera o frame no topo da pilha.
 * O frame anterior se torna o frame atual.
 * 
 * @param jvm O estado da JVM
 */
void jvm_pop_frame(JVMState *jvm);

/**
 * @brief Retorna o frame atual (topo da call stack).
 * 
 * @param jvm O estado da JVM
 * @return Ponteiro para o frame atual, ou NULL se a pilha estiver vazia
 */
Frame* jvm_current_frame(JVMState *jvm);

/**
 * @brief Retorna o número de frames na call stack.
 * 
 * @param jvm O estado da JVM
 * @return Número de frames
 */
int jvm_frame_count(JVMState *jvm);

/**
 * @brief Verifica se a call stack está vazia.
 * 
 * @param jvm O estado da JVM
 * @return 1 se vazia, 0 caso contrário
 */
int jvm_is_call_stack_empty(JVMState *jvm);

/* ============================================================================
 * FUNÇÕES DE DEPURAÇÃO E DIAGNÓSTICO
 * ============================================================================ */

/**
 * @brief Imprime o estado de um frame (para depuração).
 * 
 * Mostra o método em execução, PC, variáveis locais e pilha de operandos.
 * 
 * @param frame O frame a ser impresso
 */
void frame_print_state(Frame *frame);

/**
 * @brief Imprime toda a call stack (para depuração).
 * 
 * Mostra todos os frames da pilha, do topo até a base.
 * 
 * @param jvm O estado da JVM
 */
void jvm_print_call_stack(JVMState *jvm);

/**
 * @brief Obtém informações sobre os limites de um frame.
 * 
 * Útil para validação e depuração.
 * 
 * @param frame O frame
 * @param max_locals Ponteiro para armazenar max_locals (pode ser NULL)
 * @param max_stack Ponteiro para armazenar max_stack (pode ser NULL)
 */
void frame_get_limits(Frame *frame, u2 *max_locals, u2 *max_stack);

#ifdef __cplusplus
}
#endif

#endif // STACK_H
