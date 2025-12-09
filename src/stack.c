#include "stack.h"
#include "attributes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * FUNÇÕES AUXILIARES INTERNAS
 * ============================================================================ */

/**
 * @brief Calcula o tamanho máximo da pilha de operandos a partir do frame.
 * 
 * Como o frame armazena local_vars e operand_stack de forma contígua,
 * precisamos calcular max_stack baseado na diferença de ponteiros.
 */
static u2 get_max_stack(Frame *frame) {
    if (!frame) {
        return 0;
    }
    
    // Se não há method_info, calcula baseado na diferença de ponteiros
    // (útil para testes)
    if (!frame->method_info || !frame->class_file) {
        // Assume que o espaço após operand_stack até o fim é o max_stack
        // Para frames de teste, retorna um valor grande
        return 100;
    }
    
    // Encontra o atributo Code do método
    const CodeAttribute *code = find_code_attribute(frame->class_file, frame->method_info);
    if (!code) {
        return 100; // Fallback para testes
    }
    
    return code->max_stack;
}

/**
 * @brief Calcula o número máximo de variáveis locais do frame.
 */
static u2 get_max_locals(Frame *frame) {
    if (!frame) {
        return 0;
    }
    
    // Se não há method_info, calcula baseado na diferença de ponteiros
    if (!frame->method_info || !frame->class_file) {
        // Calcula quantos slots existem entre local_vars e operand_stack
        return (u2)(frame->operand_stack - frame->local_vars);
    }
    
    const CodeAttribute *code = find_code_attribute(frame->class_file, frame->method_info);
    if (!code) {
        return (u2)(frame->operand_stack - frame->local_vars);
    }
    
    return code->max_locals;
}

/* ============================================================================
 * OPERAÇÕES DA PILHA DE OPERANDOS
 * ============================================================================ */

int frame_push(Frame *frame, Slot value) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_push: frame is NULL\n");
        return -1;
    }
    
    u2 max_stack = get_max_stack(frame);
    u2 current_size = frame->stack_top - frame->operand_stack;
    
    if (current_size >= max_stack) {
        fprintf(stderr, "[STACK ERROR] Stack overflow: current=%u, max=%u\n", 
                current_size, max_stack);
        return -1;
    }
    
    *frame->stack_top = value;
    frame->stack_top++;
    
    return 0;
}

int frame_pop(Frame *frame, Slot *out) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_pop: frame is NULL\n");
        return -1;
    }
    
    if (frame->stack_top <= frame->operand_stack) {
        fprintf(stderr, "[STACK ERROR] Stack underflow\n");
        return -1;
    }
    
    frame->stack_top--;
    if (out) {
        *out = *frame->stack_top;
    }
    
    return 0;
}

int frame_peek(Frame *frame, Slot *out) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_peek: frame is NULL\n");
        return -1;
    }
    
    if (frame->stack_top <= frame->operand_stack) {
        fprintf(stderr, "[STACK ERROR] Cannot peek: stack is empty\n");
        return -1;
    }
    
    if (out) {
        *out = *(frame->stack_top - 1);
    }
    
    return 0;
}

int frame_is_stack_empty(Frame *frame) {
    if (!frame) {
        return 1;
    }
    return frame->stack_top == frame->operand_stack;
}

int frame_stack_size(Frame *frame) {
    if (!frame) {
        return 0;
    }
    return (int)(frame->stack_top - frame->operand_stack);
}

int frame_push_long(Frame *frame, Slot high_word, Slot low_word) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_push_long: frame is NULL\n");
        return -1;
    }
    
    u2 max_stack = get_max_stack(frame);
    u2 current_size = frame->stack_top - frame->operand_stack;
    
    // Precisa de 2 slots livres
    if (current_size + 2 > max_stack) {
        fprintf(stderr, "[STACK ERROR] Stack overflow on push_long: current=%u, max=%u\n", 
                current_size, max_stack);
        return -1;
    }
    
    // Empilha high word primeiro (convenção JVM)
    *frame->stack_top = high_word;
    frame->stack_top++;
    *frame->stack_top = low_word;
    frame->stack_top++;
    
    return 0;
}

int frame_pop_long(Frame *frame, Slot *high_word, Slot *low_word) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_pop_long: frame is NULL\n");
        return -1;
    }
    
    u2 current_size = frame->stack_top - frame->operand_stack;
    
    // Precisa de pelo menos 2 slots
    if (current_size < 2) {
        fprintf(stderr, "[STACK ERROR] Stack underflow on pop_long: current=%u\n", 
                current_size);
        return -1;
    }
    
    // Desempilha low word primeiro (ordem inversa do push)
    frame->stack_top--;
    if (low_word) {
        *low_word = *frame->stack_top;
    }
    
    frame->stack_top--;
    if (high_word) {
        *high_word = *frame->stack_top;
    }
    
    return 0;
}

/* ============================================================================
 * OPERAÇÕES DE VARIÁVEIS LOCAIS
 * ============================================================================ */

int frame_set_local(Frame *frame, u2 index, Slot value) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_set_local: frame is NULL\n");
        return -1;
    }
    
    u2 max_locals = get_max_locals(frame);
    
    if (index >= max_locals) {
        fprintf(stderr, "[STACK ERROR] Invalid local variable index: %u (max=%u)\n", 
                index, max_locals);
        return -1;
    }
    
    frame->local_vars[index] = value;
    return 0;
}

int frame_get_local(Frame *frame, u2 index, Slot *out) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_get_local: frame is NULL\n");
        return -1;
    }
    
    u2 max_locals = get_max_locals(frame);
    
    if (index >= max_locals) {
        fprintf(stderr, "[STACK ERROR] Invalid local variable index: %u (max=%u)\n", 
                index, max_locals);
        return -1;
    }
    
    if (out) {
        *out = frame->local_vars[index];
    }
    
    return 0;
}

int frame_set_local_long(Frame *frame, u2 index, Slot high_word, Slot low_word) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_set_local_long: frame is NULL\n");
        return -1;
    }
    
    u2 max_locals = get_max_locals(frame);
    
    // Precisa de 2 slots consecutivos
    if (index + 1 >= max_locals) {
        fprintf(stderr, "[STACK ERROR] Invalid local variable index for long: %u (max=%u)\n", 
                index, max_locals);
        return -1;
    }
    
    frame->local_vars[index] = high_word;
    frame->local_vars[index + 1] = low_word;
    
    return 0;
}

int frame_get_local_long(Frame *frame, u2 index, Slot *high_word, Slot *low_word) {
    if (!frame) {
        fprintf(stderr, "[STACK ERROR] frame_get_local_long: frame is NULL\n");
        return -1;
    }
    
    u2 max_locals = get_max_locals(frame);
    
    if (index + 1 >= max_locals) {
        fprintf(stderr, "[STACK ERROR] Invalid local variable index for long: %u (max=%u)\n", 
                index, max_locals);
        return -1;
    }
    
    if (high_word) {
        *high_word = frame->local_vars[index];
    }
    if (low_word) {
        *low_word = frame->local_vars[index + 1];
    }
    
    return 0;
}

/* ============================================================================
 * OPERAÇÕES DA CALL STACK
 * ============================================================================ */

int jvm_push_frame(JVMState *jvm, ClassFile *class_file, MethodInfo *method_info,
                   u2 max_locals, u2 max_stack) {
    if (!jvm) {
        fprintf(stderr, "[STACK ERROR] jvm_push_frame: jvm is NULL\n");
        return -1;
    }
    
    if (!class_file || !method_info) {
        fprintf(stderr, "[STACK ERROR] jvm_push_frame: invalid arguments\n");
        return -1;
    }
    
    // Cria o novo frame
    Frame *new_frame = frame_new(class_file, method_info, max_locals, max_stack);
    if (!new_frame) {
        fprintf(stderr, "[STACK ERROR] Failed to allocate new frame\n");
        return -1;
    }
    
    // Empilha o frame
    new_frame->next = jvm->call_stack;
    jvm->call_stack = new_frame;
    
    return 0;
}

void jvm_pop_frame(JVMState *jvm) {
    if (!jvm || !jvm->call_stack) {
        fprintf(stderr, "[STACK WARNING] jvm_pop_frame: call stack is empty\n");
        return;
    }
    
    Frame *old_frame = jvm->call_stack;
    jvm->call_stack = old_frame->next;
    
    frame_free(old_frame);
}

Frame* jvm_current_frame(JVMState *jvm) {
    if (!jvm) {
        return NULL;
    }
    return jvm->call_stack;
}

int jvm_frame_count(JVMState *jvm) {
    if (!jvm) {
        return 0;
    }
    
    int count = 0;
    Frame *current = jvm->call_stack;
    while (current) {
        count++;
        current = current->next;
    }
    
    return count;
}

int jvm_is_call_stack_empty(JVMState *jvm) {
    if (!jvm) {
        return 1;
    }
    return jvm->call_stack == NULL;
}

/* ============================================================================
 * FUNÇÕES DE DEPURAÇÃO
 * ============================================================================ */

void frame_get_limits(Frame *frame, u2 *max_locals, u2 *max_stack) {
    if (!frame) {
        if (max_locals) *max_locals = 0;
        if (max_stack) *max_stack = 0;
        return;
    }
    
    if (max_locals) {
        *max_locals = get_max_locals(frame);
    }
    
    if (max_stack) {
        *max_stack = get_max_stack(frame);
    }
}

void frame_print_state(Frame *frame) {
    if (!frame) {
        printf("[FRAME] NULL frame\n");
        return;
    }
    
    // Obtém o nome do método
    const char *method_name = "";
    if (frame->method_info && frame->class_file) {
        method_name = cp_utf8(frame->class_file->constant_pool,
                             frame->class_file->constant_pool_count,
                             frame->method_info->name_index);
    }
    
    printf("=== FRAME STATE ===\n");
    printf("Method: %s\n", method_name);
    
    // Imprime PC
    const CodeAttribute *code = find_code_attribute(frame->class_file, frame->method_info);
    if (code && frame->pc) {
        u4 pc_offset = (u4)(frame->pc - code->code);
        printf("PC: %u (0x%04X)\n", pc_offset, pc_offset);
    } else {
        printf("PC: (not initialized)\n");
    }
    
    // Imprime variáveis locais
    u2 max_locals = get_max_locals(frame);
    printf("\nLocal Variables (%u slots):\n", max_locals);
    for (u2 i = 0; i < max_locals && i < 10; i++) {
        printf("  [%u] = %u (0x%08X)\n", i, frame->local_vars[i], frame->local_vars[i]);
    }
    if (max_locals > 10) {
        printf("  ... (%u more)\n", max_locals - 10);
    }
    
    // Imprime pilha de operandos
    int stack_size = frame_stack_size(frame);
    u2 max_stack = get_max_stack(frame);
    printf("\nOperand Stack (%d/%u slots used):\n", stack_size, max_stack);
    
    if (stack_size == 0) {
        printf("  (empty)\n");
    } else {
        for (int i = 0; i < stack_size && i < 10; i++) {
            Slot value = frame->operand_stack[i];
            printf("  [%d] = %u (0x%08X)\n", i, value, value);
        }
        if (stack_size > 10) {
            printf("  ... (%d more)\n", stack_size - 10);
        }
    }
    
    printf("==================\n\n");
}

void jvm_print_call_stack(JVMState *jvm) {
    if (!jvm) {
        printf("[CALL STACK] JVM is NULL\n");
        return;
    }
    
    if (!jvm->call_stack) {
        printf("[CALL STACK] Empty\n");
        return;
    }
    
    printf("\n===== CALL STACK =====\n");
    
    int depth = 0;
    Frame *current = jvm->call_stack;
    
    while (current) {
        const char *method_name = "";
        const char *class_name = "";
        
        if (current->method_info && current->class_file) {
            method_name = cp_utf8(current->class_file->constant_pool,
                                 current->class_file->constant_pool_count,
                                 current->method_info->name_index);
            
            class_name = cp_nome_classe(current->class_file->constant_pool,
                                       current->class_file->constant_pool_count,
                                       current->class_file->this_class);
        }
        
        printf("[%d] %s.%s\n", depth, class_name, method_name);
        
        // Mostra informações básicas do frame
        u2 max_locals, max_stack;
        frame_get_limits(current, &max_locals, &max_stack);
        int stack_size = frame_stack_size(current);
        
        printf("    Locals: %u, Stack: %d/%u\n", max_locals, stack_size, max_stack);
        
        depth++;
        current = current->next;
    }
    
    printf("======================\n\n");
}
