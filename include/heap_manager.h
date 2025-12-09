// heap_manager.h - Gerenciamento de Heap (Pessoa 4)
#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "classfile.h"
#include "io.h"

// Tipo para valores na pilha
typedef u4 StackValue;

// Definições de tipos de componentes de array
#define T_INT 5
#define T_OBJECT 9

// --- Estruturas da Heap ---

// Struct Array: Implementa suporte a vetores (int[], Object[])
typedef struct {
    u1 component_type;  // Tipo dos elementos (Ex: T_INT, T_OBJECT)
    u4 length;          // Número de elementos
    StackValue *data;   // Ponteiro para o bloco de dados
} Array;

// Struct Object: Define como um objeto é guardado na Heap
typedef struct {
    ClassFile *class_info;  // Quem sou eu? (Tipo real)
    StackValue *fields;     // Dados dos campos de instância
} Object;

// Tipo para Referência de Objeto (Endereço na Heap)
typedef Object* ObjectRef;

// --- Funções de Alocação (new, newarray) ---

/**
 * @brief Aloca memória para um novo objeto na Heap
 * @param class_info Metadados da classe
 * @param field_count Número de campos da instância
 * @return Referência para o objeto alocado
 */
ObjectRef jvm_heap_new_object(ClassFile *class_info, size_t field_count);

/**
 * @brief Aloca memória para um novo array (newarray)
 * @param type Tipo dos elementos do array
 * @param length Tamanho do array
 * @return Referência para o array alocado
 */
ObjectRef jvm_heap_new_array(u1 type, u4 length);

// --- Funções de Acesso (getfield, putfield) ---

/**
 * @brief Lê o valor de um campo de instância (getfield)
 * @param obj_ref Referência do objeto
 * @param offset Offset do campo no objeto
 * @return Valor do campo
 */
StackValue jvm_heap_getfield(ObjectRef obj_ref, u4 offset);

/**
 * @brief Grava um valor em um campo de instância (putfield)
 * @param obj_ref Referência do objeto
 * @param offset Offset do campo no objeto
 * @param value Valor a ser gravado
 */
void jvm_heap_putfield(ObjectRef obj_ref, u4 offset, StackValue value);

/**
 * @brief Libera a memória de um objeto
 * @param obj_ref Referência do objeto a ser liberado
 */
void jvm_heap_free_object(ObjectRef obj_ref);

/**
 * @brief Obtém a classe de um objeto
 * @param obj_ref Referência do objeto
 * @return Ponteiro para o ClassFile da classe do objeto
 */
ClassFile* jvm_heap_get_object_class(ObjectRef obj_ref);

#endif // HEAP_MANAGER_H
