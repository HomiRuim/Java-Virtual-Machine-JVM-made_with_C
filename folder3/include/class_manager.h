#ifndef CLASS_MANAGER_H
#define CLASS_MANAGER_H

#include "jvm_types.h"

// ONDE OS ERROS FORAM CORRIGIDOS: Definição completa das structs
struct s_class_metadata {
    char *this_class_name;
    // ... (campos omitidos para foco)
};
typedef struct s_class_metadata ClassMetadata; 

// Estrutura de Exceção Simplificada (parte do Atributo Code)
typedef struct {
    uint16_t start_pc;
    uint16_t end_pc;
    uint16_t handler_pc;
    uint16_t catch_type_index; 
} ExceptionTableEntry;

// Estrutura de Informação do Método
struct s_method_info {
    uint16_t max_locals;
    uint16_t max_stack;
    uint8_t *bytecode; 
    
    ExceptionTableEntry *exception_table; 
    uint16_t exception_table_count;
};
typedef struct s_method_info MethodInfo;


// Estrutura de Frame Simplificada 
struct s_stack_frame {
    StackValue local_vars[256];
    StackValue operand_stack[256];
    size_t stack_ptr;
    uint32_t pc;
    MethodInfo *method;
    struct s_stack_frame *previous_frame;
};
typedef struct s_stack_frame StackFrame;


// Referência de Objeto na Heap
struct s_object_ref {
    void *address; 
};
typedef struct s_object_ref ObjectRef;


// Funções de Resolução:
MethodInfo* class_manager_find_static_method(const char *class_name, const char *method_signature);
MethodInfo* class_manager_find_virtual_method_in_hierarchy(ClassMetadata *real_class, const char *method_signature);
bool class_manager_is_exception_compatible(uint16_t thrown_exception_cp_index, uint16_t catch_type_index);
size_t get_argument_count(const char *signature); 

#endif // CLASS_MANAGER_H <-- FIX: Adicionado o '#endif' ausente