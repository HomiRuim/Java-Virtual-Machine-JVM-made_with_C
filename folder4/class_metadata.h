// class_metadata.h
#ifndef CLASS_METADATA_H
#define CLASS_METADATA_H

#include <stdint.h>
#include <stddef.h>
// Assumindo que este arquivo define StackValue e tipos base (Pessoa 1)
#include "jvm_types.h" 

// -----------------------------------------------------------------------------
// Estruturas Auxiliares
// -----------------------------------------------------------------------------

// Tipo de entrada de exceção (Usado dentro de MethodInfo - CRUCIAL para Pessoa 3)
typedef struct {
    uint16_t start_pc;
    uint16_t end_pc;
    uint16_t handler_pc;
    uint16_t catch_type_index; // Índice na CP para a classe de exceção
} ExceptionTableEntry;

// Entrada genérica da Constant Pool (Simplificação)
typedef struct {
    uint8_t tag;
    void *data; // Ponteiro para a estrutura de dado específico (Utf8, FieldRef, etc.)
} ConstantPoolEntry;


// -----------------------------------------------------------------------------
// 1. Field Info (Metadados do Campo)
// -----------------------------------------------------------------------------
typedef struct s_field_info {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    
    // CRUCIAL para Pessoa 4 (Heap) e Pessoa 2 (getfield/putfield)
    uint32_t offset;           // Deslocamento dentro do array fields do objeto
    uint8_t size;              // Tamanho em StackValues (1 ou 2)
} FieldInfo;


// -----------------------------------------------------------------------------
// 2. Method Info (Metadados do Método)
// -----------------------------------------------------------------------------
typedef struct s_method_info {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    
    // Informações do Atributo Code (CRUCIAIS para Pessoas 1, 2 e 3)
    uint16_t max_stack;        // Limite da Pilha de Operandos (Pessoa 1)
    uint16_t max_locals;       // Limite das Variáveis Locais (Pessoa 1)
    uint8_t *bytecode;         // Código de execução (Pessoa 2)
    uint32_t code_length;
    
    // Tabela de Exceções (CRUCIAL para Pessoa 3 - athrow)
    ExceptionTableEntry *exception_table; 
    uint16_t exception_table_count;

} MethodInfo;


// -----------------------------------------------------------------------------
// 3. Class Metadata (Estrutura Principal - CRUCIAL)
// -----------------------------------------------------------------------------
typedef struct s_class_metadata {
    // Informações de versão e CP
    uint32_t magic;
    uint16_t major_version;
    uint16_t minor_version;
    
    uint16_t this_class_index;
    char *this_class_name; 

    // Link para Herança (CRUCIAL para Polimorfismo - Pessoa 3)
    struct s_class_metadata *super_class; 
    
    ConstantPoolEntry *constant_pool;
    uint16_t cp_count;

    // Lista de Campos e Métodos
    FieldInfo *fields;
    uint16_t fields_count;
    
    MethodInfo *methods;
    uint16_t methods_count;

    // CRUCIAL para Pessoa 4 (Heap): Tamanho total dos campos de instância
    size_t instance_size_in_stackvalues; 

} ClassMetadata;

// Garante que o tipo seja acessível em qualquer lugar
typedef struct s_class_metadata ClassMetadata; 

#endif // CLASS_METADATA_H