// test_pessoa4.c
#include <stdio.h>
#include <stdlib.h>
#include "jvm_types.h"
#include "class_metadata.h"
#include "heap_manager.h"
#include "class_loader_mock.h"

void run_test_pessoa4() {
    printf("--- TESTE PESSOA 4: GERENTE DE HEAP (Alocacao, Acesso e Desalocacao) ---\n");
    
    // --- SETUP: Carrega Metadados MOCK ---
    // Simula uma classe com 5 campos de instancia (instance_size = 5 StackValues)
    const size_t NUM_FIELDS = 5;
    ClassMetadata *test_class = mock_class_loader_load_class("MinhaClasseDeTeste", NUM_FIELDS);
    
    if (!test_class) {
        printf("[FAIL] Falha ao carregar metadados MOCK.\n");
        return;
    }
    printf("[OK] Metadados MOCK carregados (Classe: %s, Campos: %zu)\n", 
           test_class->this_class_name, test_class->instance_size_in_stackvalues);


    // --- TESTE 1: jvm_heap_new_object (Instrucao 'new') ---
    ObjectRef obj = jvm_heap_new_object(test_class, test_class->instance_size_in_stackvalues);
    
    if (obj) {
        printf("[OK] 1. Objeto alocado (new). Referencia: %p\n", (void*)obj);
        
        // Verifica se a referencia de classe foi configurada
        if (obj->class_info == test_class) {
            printf("[OK] 1.1. Referencia de classe correta.\n");
        } else {
            printf("[FAIL] 1.1. Referencia de classe incorreta.\n");
        }
    } else {
        printf("[FAIL] 1. Falha na alocacao do objeto.\n");
        mock_class_loader_unload_class(test_class);
        return;
    }


    // --- TESTE 2: jvm_heap_putfield / jvm_heap_getfield (Acesso) ---
    const uint32_t FIELD_INDEX = 3; // Quarto campo (indice 3)
    const StackValue TEST_VALUE = 0xDEADBEEF; 

    // putfield
    jvm_heap_putfield(obj, NULL, FIELD_INDEX, TEST_VALUE); 
    printf("[OK] 2.1. putfield executado no offset %u.\n", FIELD_INDEX);
    
    // getfield
    StackValue retrieved_value = jvm_heap_getfield(obj, NULL, FIELD_INDEX);
    
    if (retrieved_value == TEST_VALUE) {
        printf("[OK] 2.2. getfield/putfield funcional. Valor lido: 0x%llX\n", retrieved_value);
    } else {
        printf("[FAIL] 2.2. Falha no acesso a campo. Valor lido: 0x%llX\n", retrieved_value);
    }
    
    
    // --- TESTE 3: jvm_heap_new_array (Instrucao 'newarray') ---
    const uint32_t ARRAY_LENGTH = 10;
    ObjectRef arr_ref = jvm_heap_new_array(T_INT, ARRAY_LENGTH);
    Array *test_array = (Array*)arr_ref;

    if (test_array && test_array->length == ARRAY_LENGTH && test_array->component_type == T_INT) {
         printf("[OK] 3. Array alocado (newarray). Tamanho: %u\n", test_array->length);
    } else {
        printf("[FAIL] 3. Falha na alocacao do array.\n");
    }
    
    
    // --- TESTE 4: jvm_heap_free_object (Desalocacao Manual) ---
    // Desalocacao do Array
    if (arr_ref) {
        jvm_heap_free_object(arr_ref);
        printf("[OK] 4.1. Array desalocado manualmente (free) com sucesso.\n");
    }

    // Desalocacao do Objeto
    jvm_heap_free_object(obj);
    printf("[OK] 4.2. Objeto desalocado manualmente (free) com sucesso.\n");

    
    // --- CLEANUP: Descarrega Metadados MOCK ---
    mock_class_loader_unload_class(test_class);
    printf("[OK] 5. Metadados MOCK limpos.\n");

    printf("------------------------------------------------------------------\n");
}

int main() {
    run_test_pessoa4();
    return 0;
}