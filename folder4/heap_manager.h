// heap_manager.h
#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "class_metadata.h"

// Assumindo que jvm_types.h define StackValue
typedef uint64_t StackValue; 

// --- Estruturas Mock para Dependências (Pessoa 4 depende de Pessoa 1 e 2) ---
// Estes structs serão definidos completamente pela Pessoa 4 (Class Loader)
//typedef struct s_class_metadata ClassMetadata; 
//typedef struct s_field_info FieldInfo;


// --- Estruturas da Heap (Responsabilidade da Pessoa 4) ---

// Struct Array: Implementa suporte a vetores (int[], Object[])
typedef struct Array {
    uint8_t component_type; // Tipo dos elementos (Ex: T_INT, T_OBJECT)
    uint32_t length;        // Número de elementos
    StackValue *data;       // Ponteiro para o bloco de dados
} Array;


// Struct Object: Define como um objeto é guardado na Heap
typedef struct s_object_ref {
    ClassMetadata *class_info; // Quem sou eu? (Tipo real)
    StackValue *fields;        // Dados dos campos de instância
} Object;


// Tipo para Referência de Objeto (Endereço na Heap)
typedef Object* ObjectRef;


// --- Funções de Alocação (new, newarray) ---

// Aloca memória para um novo objeto na Heap
ObjectRef jvm_heap_new_object(ClassMetadata *class_info, size_t field_count);

// Aloca memória para um novo array (newarray)
ObjectRef jvm_heap_new_array(uint8_t type, uint32_t length);


// --- Funções de Acesso (getfield, putfield) ---

// Lê o valor de um campo de instância (getfield)
StackValue jvm_heap_getfield(ObjectRef obj_ref, FieldInfo *field_info, uint32_t offset);

// Grava um valor em um campo de instância (putfield)
void jvm_heap_putfield(ObjectRef obj_ref, FieldInfo *field_info, uint32_t offset, StackValue value);


void jvm_heap_free_object(ObjectRef obj_ref);


#endif // HEAP_MANAGER_H