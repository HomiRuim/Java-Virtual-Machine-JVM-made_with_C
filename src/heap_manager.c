// heap_manager.c - Implementação do Gerenciamento de Heap (Pessoa 4)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "heap_manager.h"

/**
 * @brief Aloca memória para um novo objeto na Heap
 */
ObjectRef jvm_heap_new_object(ClassFile *class_info, size_t field_count) {
    if (!class_info) {
        fprintf(stderr, "Erro: class_info NULL em jvm_heap_new_object\n");
        return NULL;
    }

    // 1. Calcular o tamanho total para a estrutura Object + campos
    size_t total_field_bytes = field_count * sizeof(StackValue);
    
    // 2. Alocar a memória
    ObjectRef new_obj = (ObjectRef)malloc(sizeof(Object) + total_field_bytes);
    if (!new_obj) {
        fprintf(stderr, "Erro: Falha na alocação de memória para objeto\n");
        exit(1);
    }
    
    // 3. Inicializar metadados e campos
    new_obj->class_info = class_info;
    new_obj->fields = (StackValue*)((u1*)new_obj + sizeof(Object));
    
    // Zera os campos
    memset(new_obj->fields, 0, total_field_bytes);

    return new_obj;
}

/**
 * @brief Aloca memória para um novo array (newarray)
 */
ObjectRef jvm_heap_new_array(u1 type, u4 length) {
    // 1. Calcular o tamanho total
    size_t data_bytes = length * sizeof(StackValue);
    
    // 2. Alocar a memória
    Array *new_array = (Array*)malloc(sizeof(Array) + data_bytes);
    if (!new_array) {
        fprintf(stderr, "Erro: Falha na alocação de memória para array\n");
        exit(1);
    }
    
    // 3. Inicializar campos
    new_array->component_type = type;
    new_array->length = length;
    new_array->data = (StackValue*)((u1*)new_array + sizeof(Array));

    // Zera os dados
    memset(new_array->data, 0, data_bytes);
    
    // Array é um tipo especial de ObjectRef
    return (ObjectRef)(void*)new_array;
}

/**
 * @brief Lê o valor de um campo de instância (getfield)
 */
StackValue jvm_heap_getfield(ObjectRef obj_ref, u4 offset) {
    if (!obj_ref) {
        fprintf(stderr, "Erro: NullPointerException em getfield\n");
        return 0;
    }
    
    // Acessa o array de campos do objeto usando o offset
    return obj_ref->fields[offset];
}

/**
 * @brief Grava um valor em um campo de instância (putfield)
 */
void jvm_heap_putfield(ObjectRef obj_ref, u4 offset, StackValue value) {
    if (!obj_ref) {
        fprintf(stderr, "Erro: NullPointerException em putfield\n");
        return;
    }

    // Grava o valor no offset
    obj_ref->fields[offset] = value;
}

/**
 * @brief Libera a memória de um objeto
 */
void jvm_heap_free_object(ObjectRef obj_ref) {
    if (obj_ref == NULL) return;
    
    // Na nossa JVM simplificada sem GC, free() é suficiente
    free(obj_ref);
}

/**
 * @brief Obtém a classe de um objeto
 */
ClassFile* jvm_heap_get_object_class(ObjectRef obj_ref) {
    if (!obj_ref) {
        fprintf(stderr, "Erro: NullPointerException em get_object_class\n");
        return NULL;
    }
    
    return obj_ref->class_info;
}
