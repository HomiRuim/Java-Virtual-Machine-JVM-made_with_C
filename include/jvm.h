#ifndef JVM_H
#define JVM_H

#include "classfile.h"
#include "io.h" // Para u1, u2, u4

/**
 * @brief Tipo genérico para representar um valor na Pilha de Operandos ou Variáveis Locais.
 *
 * O Frame terá um tamanho fixo, então usamos um tipo que pode acomodar
 * qualquer valor de 4 ou 8 bytes (u4 ou u8).
 *
 * Para simplificar, usaremos u4 para representar um slot de 32 bits.
 * Valores de 64 bits (long, double) ocuparão 2 slots.
 */
typedef u4 Slot;

/**
 * @brief Estrutura que representa um Frame de Execução.
 *
 * O Frame contém:
 * - Um ponteiro para o ClassFile da classe onde o método está sendo executado.
 * - Um ponteiro para o MethodInfo do método em execução.
 * - Um vetor de Slots para as Variáveis Locais.
 * - Um vetor de Slots para a Pilha de Operandos.
 * - Um ponteiro para o topo da Pilha de Operandos (para gerenciar o push/pop).
 * - Um ponteiro para o próximo Frame na pilha de Frames (Call Stack).
 *
 * O Frame terá um tamanho único e fixo (vetor de Slots), e os ponteiros
 * (local_vars, operand_stack) serão usados para gerenciar o acesso dentro
 * desse vetor.
 */
typedef struct frame {
    ClassFile *class_file;      // Ponteiro para a estrutura ClassFile
    MethodInfo *method_info;    // Ponteiro para a estrutura MethodInfo
    u1 *pc;                     // Program Counter: ponteiro para o próximo bytecode a ser executado

    Slot *local_vars;           // Ponteiro para o início do vetor de Variáveis Locais
    Slot *operand_stack;        // Ponteiro para o início do vetor da Pilha de Operandos
    Slot *stack_top;            // Ponteiro para o topo da Pilha de Operandos (próximo slot livre)

    struct frame *next;         // Ponteiro para o próximo Frame na pilha (Call Stack)

    // O vetor de Slots que armazena Local Vars e Operand Stack.
    // O tamanho total é determinado pela soma de max_locals e max_stack do Code Attribute.
    Slot slots_data[];
} Frame;

/**
 * @brief Estrutura que representa o estado da JVM (máquina virtual).
 *
 * Contém a pilha de Frames (Call Stack) e a área de classes carregadas.
 * A "Área de Referências" foi removida conforme solicitado.
 */
typedef struct jvm_state {
    Frame *call_stack;          // Ponteiro para o Frame atual (topo da pilha)
    // TODO: Adicionar a área de classes carregadas (mapa de ClassFile*)
} JVMState;

/**
 * @brief Inicializa um novo Frame de Execução.
 *
 * @param class_file O ClassFile da classe do método.
 * @param method_info O MethodInfo do método a ser executado.
 * @param max_locals O número máximo de variáveis locais (do Code Attribute).
 * @param max_stack O número máximo de slots na pilha de operandos (do Code Attribute).
 * @return Um ponteiro para o novo Frame alocado.
 */
Frame *frame_new(ClassFile *class_file, MethodInfo *method_info, u2 max_locals, u2 max_stack);

/**
 * @brief Libera a memória de um Frame.
 *
 * @param frame O Frame a ser liberado.
 */
void frame_free(Frame *frame);

/**
 * @brief Inicializa o estado da JVM.
 *
 * @return Um ponteiro para a nova estrutura JVMState alocada.
 */
JVMState *jvm_new();

/**
 * @brief Libera a memória do estado da JVM.
 *
 * @param jvm O estado da JVM a ser liberado.
 */
void jvm_free(JVMState *jvm);

#endif // JVM_H
