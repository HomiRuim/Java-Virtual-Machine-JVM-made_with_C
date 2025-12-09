// memory_manager_mock.c
#include "../include/jvm_types.h"
#include "../include/memory_manager.h"
#include "../include/class_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Variáveis Mock para rastreamento e retorno
StackFrame mock_frames[5];
int mock_frame_count = 0;
ClassMetadata mock_real_class; 

// Implementação Mock de push_frame
StackFrame* jvm_stack_push_frame(size_t max_locals, size_t max_stack) {
    if (mock_frame_count >= 5) {
        printf("[MOCK ERROR] Stack Overflow simulado.\n");
        return NULL;
    }
    
    // Configura o Frame (Simula alocação)
    StackFrame *new_frame = &mock_frames[mock_frame_count++];
    memset(new_frame->local_vars, 0, sizeof(StackValue) * 256);
    memset(new_frame->operand_stack, 0, sizeof(StackValue) * 256);
    new_frame->pc = 0;
    
    // Conecta ao Frame anterior
    if (mock_frame_count > 1) {
        new_frame->previous_frame = &mock_frames[mock_frame_count - 2];
    } else {
        new_frame->previous_frame = NULL;
    }

    return new_frame;
}

// Implementação Mock de pop_frame
StackFrame* jvm_stack_pop_frame(StackValue *return_value) {
    if (mock_frame_count <= 0) return NULL;
    
    // Simula a limpeza do Frame do topo
    mock_frame_count--;
    
    // Retorna o Frame anterior
    if (mock_frame_count > 0) {
        return &mock_frames[mock_frame_count - 1];
    }
    return NULL;
}

// Implementação Mock de heap_get_object_class (Polimorfismo)
ClassMetadata* heap_get_object_class(ObjectRef obj_ref) {
    // Retorna sempre um ponteiro para a classe mock pré-configurada
    mock_real_class.this_class_name = "MockRealClass";
    return &mock_real_class;
}