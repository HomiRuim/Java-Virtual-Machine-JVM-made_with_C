// class_manager_mock.c
#include "../include/jvm_types.h"
#include "../include/class_manager.h"
#include <string.h>
#include <stdio.h>

// Estruturas Mock
MethodInfo mock_method_static = { .max_locals = 2, .max_stack = 5, .bytecode = NULL, .exception_table_count = 0 };
MethodInfo mock_method_virtual = { .max_locals = 3, .max_stack = 6, .bytecode = NULL, .exception_table_count = 0 };
MethodInfo mock_method_exception;

// Implementação Mock: Retorna um método estático fixo
MethodInfo* class_manager_find_static_method(const char *class_name, const char *method_signature) {
    printf("[MOCK] Resolvendo static: %s.%s\n", class_name, method_signature);
    return &mock_method_static;
}

// Implementação Mock: Retorna um método virtual (simulando override)
MethodInfo* class_manager_find_virtual_method_in_hierarchy(ClassMetadata *real_class, const char *method_signature) {
    printf("[MOCK] Resolvendo virtual para classe real: %s\n", real_class->this_class_name);
    // Para simplificar, sempre retorna o método virtual mock
    return &mock_method_virtual;
}

// Implementação Mock: Compatibilidade de Exceção
bool class_manager_is_exception_compatible(uint16_t thrown_exception_cp_index, uint16_t catch_type_index) {
    // Simula a compatibilidade: aceita se o índice for 1 ou se o catch_type for 0 (catch all)
    return (catch_type_index == 0 || catch_type_index == 1);
}

// Implementação Mock: Obtém a contagem de argumentos
size_t get_argument_count(const char *signature) {
    // Simplesmente retorna 2 argumentos para testes
    return 2; 
}