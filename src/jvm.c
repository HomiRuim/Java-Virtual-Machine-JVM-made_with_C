#include <stdlib.h>
#include <string.h>
#include "jvm.h"
#include "attributes.h" // Para acessar o Code Attribute

/**
 * @brief Inicializa um novo Frame de Execução.
 */
Frame *frame_new(ClassFile *class_file, MethodInfo *method_info, u2 max_locals, u2 max_stack) {
    // O tamanho total do Frame será o tamanho da estrutura base + o espaço para os slots.
    size_t total_slots = max_locals + max_stack;
    size_t total_size = sizeof(Frame) + (total_slots * sizeof(Slot));

    Frame *frame = (Frame *)calloc(1, total_size);
    if (!frame) {
        return NULL; // Falha na alocação
    }

    frame->class_file = class_file;
    frame->method_info = method_info;
    frame->pc = NULL; // Será inicializado com o início do bytecode

    // O vetor de slots_data é contíguo.
    // local_vars aponta para o início dos slots.
    frame->local_vars = frame->slots_data;

    // operand_stack aponta para o fim das variáveis locais.
    frame->operand_stack = frame->slots_data + max_locals;

    // stack_top aponta para o início da pilha de operandos (vazia).
    frame->stack_top = frame->operand_stack;

    frame->next = NULL;

    // TODO: Inicializar local_vars com os argumentos do método, se houver.

    return frame;
}

/**
 * @brief Libera a memória de um Frame.
 */
void frame_free(Frame *frame) {
    // A memória do Frame foi alocada em um único bloco com calloc,
    // então um único free é suficiente.
    if (frame) {
        free(frame);
    }
}

/**
 * @brief Inicializa o estado da JVM.
 */
JVMState *jvm_new() {
    JVMState *jvm = (JVMState *)calloc(1, sizeof(JVMState));
    if (!jvm) {
        return NULL;
    }
    jvm->call_stack = NULL;
    // TODO: Inicializar a área de classes carregadas.
    return jvm;
}

/**
 * @brief Libera a memória do estado da JVM.
 */
void jvm_free(JVMState *jvm) {
    if (jvm) {
        // Libera todos os Frames na Call Stack
        Frame *current = jvm->call_stack;
        while (current) {
            Frame *next = current->next;
            frame_free(current);
            current = next;
        }
        // TODO: Liberar a área de classes carregadas.
        free(jvm);
    }
}
