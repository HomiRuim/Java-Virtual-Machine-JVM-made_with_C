// heap_manager.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "heap_manager.h"
#include "class_metadata.h"



// Aloca memória para um novo objeto na Heap
ObjectRef jvm_heap_new_object(ClassMetadata *class_info, size_t field_count) {
    if (!class_info) return NULL; 

    // 1. Calcular o tamanho total para a estrutura Object + campos
    size_t total_field_bytes = field_count * sizeof(StackValue);
    
    // 2. Alocar a memória (Simulando a alocação de baixo nível)
    ObjectRef new_obj = (ObjectRef)malloc(sizeof(Object) + total_field_bytes);
    if (!new_obj) {
        fprintf(stderr, "Erro de alocação de Heap para objeto.\n");
        exit(1);
    }
    
    // 3. Inicializar metadados e campos
    new_obj->class_info = class_info;
    new_obj->fields = (StackValue*)((uint8_t*)new_obj + sizeof(Object));
    
    // Zera os campos
    memset(new_obj->fields, 0, total_field_bytes);

    return new_obj;
}

// Aloca memória para um novo array (newarray)
ObjectRef jvm_heap_new_array(uint8_t type, uint32_t length) {
    // 1. Calcular o tamanho total
    size_t data_bytes = length * sizeof(StackValue); 
    
    // 2. Alocar a memória
    Array *new_array = (Array*)malloc(sizeof(Array) + data_bytes);
    if (!new_array) {
        fprintf(stderr, "Erro de alocação de Heap para array.\n");
        exit(1);
    }
    
    // 3. Inicializar campos
    new_array->component_type = type;
    new_array->length = length;
    new_array->data = (StackValue*)((uint8_t*)new_array + sizeof(Array));

    // Zera os dados
    memset(new_array->data, 0, data_bytes);
    
    // Array é um tipo especial de ObjectRef
    // FIX CRÍTICO: Conversão intermediária para (void*) para satisfazer o compilador.
    return (ObjectRef)(void*)new_array; 
}


// Lê o valor de um campo de instância (getfield)
StackValue jvm_heap_getfield(ObjectRef obj_ref, FieldInfo *field_info, uint32_t offset) {
    if (!obj_ref) {
        // Lançar NullPointerException (Pessoa 3)
        return 0; 
    }
    
    // 1. Acessa o array de campos do objeto
    // 2. Usa o offset (índice na nossa simplificação) para obter o valor
    return obj_ref->fields[offset]; 
}

// Grava um valor em um campo de instância (putfield)
void jvm_heap_putfield(ObjectRef obj_ref, FieldInfo *field_info, uint32_t offset, StackValue value) {
    if (!obj_ref) {
        // Lançar NullPointerException (Pessoa 3)
        return;
    }

    // 1. Acessa o array de campos do objeto
    // 2. Grava o valor no offset (índice)
    obj_ref->fields[offset] = value;
}

void jvm_heap_free_object(ObjectRef obj_ref) {
    if (obj_ref == NULL) return;
    
    // Na nossa JVM simplificada sem GC, free() é suficiente.
    // Lembre-se: Objetos e Arrays são alocados com malloc.
    free(obj_ref);
    // Em uma JVM real, você precisaria de lógica mais complexa 
    // para limpar campos, vetores internos, etc.
}