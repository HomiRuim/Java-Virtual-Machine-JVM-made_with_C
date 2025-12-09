#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "jvm_types.h"

// Função de Gerenciamento de Frames
// Cria e adiciona um novo Frame (Pessoa 1)
StackFrame* jvm_stack_push_frame(size_t max_locals, size_t max_stack);

// Remove o Frame do topo, limpa sua memória, e retorna o Frame anterior (Pessoa 1)
StackFrame* jvm_stack_pop_frame(StackValue *return_value); 

// Função de Heap: Obtém os metadados da classe a partir de uma referência de objeto (Pessoa 1)
ClassMetadata* heap_get_object_class(ObjectRef obj_ref);

#endif // MEMORY_MANAGER_H