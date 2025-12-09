// Arquivo: src/test_d.c - Versão Corrigida

#include <stdio.h>    // Para printf
#include <stdint.h>   // Para tipos padronizados
#include <stdlib.h>   // Para free

// Headers do projeto (assumindo que definem os tipos u1, u2, u4, ClassFile, CpInfo, CodeAttribute, DisasmOutput, e as constantes)
#include "base.h"       
#include "io.h"
#include "classfile.h"  // Define ClassFile, CpInfo
#include "attributes.h" // Define CodeAttribute
#include "disasm.h"     // Define DisasmOutput, disassemble_method, free_disasm_output

// Se estas macros não estiverem nos headers, use-as. Se causarem redefinição, remova-as.
#define CONSTANT_Utf8 1
#define CONSTANT_Integer 3
#define CONSTANT_Class 7
#define CONSTANT_Methodref 10
#define CONSTANT_NameAndType 12 // NameAndType é tag 12

// --- FUNÇÃO MAIN DE TESTE ---
int main(int argc, char *argv[]) {
    // ----------------------------------------------------
    // 1. MOCK: CRIAÇÃO DO CP PARA TESTE (ClassFile)
    // ----------------------------------------------------
    
    // CORREÇÕES: 
    // - Removido `.info` (assumindo união anônima).
    // - Usado `.Num.bytes` para CONSTANT_Integer.
    // - Corrigida a inicialização de CONSTANT_Utf8 (cast e cálculo de length).
    
    CpInfo mock_cp[] = {
        {0}, // Index 0 (não usado)
        // Index 1: Utf8 "java/lang/Object"
        {.tag = CONSTANT_Utf8, .Utf8.length = (u2)(sizeof("java/lang/Object") - 1), .Utf8.bytes = (u1*)"java/lang/Object"}, 
        // Index 2: Utf8 "init"
        {.tag = CONSTANT_Utf8, .Utf8.length = (u2)(sizeof("init") - 1), .Utf8.bytes = (u1*)"init"}, 
        // Index 3: Utf8 "()V"
        {.tag = CONSTANT_Utf8, .Utf8.length = (u2)(sizeof("()V") - 1), .Utf8.bytes = (u1*)"()V"}, 

        // Index 4: Class que aponta para #1
        {.tag = CONSTANT_Class, .Class.name_index = 1}, 

        // Index 5: NameAndType (Tag 12) - nome #2, desc #3
        {.tag = CONSTANT_NameAndType, .NameAndType.name_index = 2, .NameAndType.descriptor_index = 3}, 

        // Index 6: Methodref (Invokespecial) - classe #4, name_type #5
        // OBS: Usando .Ref. Se o campo for Methodref, mude para .Methodref.class_index, etc.
        {.tag = CONSTANT_Methodref, .Ref.class_index = 4, .Ref.name_and_type_index = 5}, 

        // Index 7: Integer - Usando .Num.bytes como solicitado
        {.tag = CONSTANT_Integer, .Num.bytes = 12345}, 
    };

    ClassFile mock_cf = {
        .constant_pool_count = 8,
        .constant_pool = mock_cp
    };
    
    // ----------------------------------------------------
    // 2. MOCK: CRIAÇÃO DO BYTECODE PARA TESTE (CodeAttribute)
    // ----------------------------------------------------
    u1 mock_code[] = {
        0x12, 0x07,       // ldc #7
        0xB7, 0x00, 0x06, // invokespecial #6
        0x99, 0x00, 0x07, // ifne +7 bytes
        0xA7, 0x00, 0x0A, // goto +10 bytes
        0xB1                // return
    };

    // CORREÇÃO: Usando o campo `.code` (u1*) para armazenar o bytecode.
    CodeAttribute mock_code_attr = {
        .max_stack = 1,     // Exemplo de inicialização
        .max_locals = 1,    // Exemplo de inicialização
        .code_length = sizeof(mock_code),
        .code = mock_code
    };
    
    // ----------------------------------------------------
    // 3. EXECUÇÃO DO DISASSEMBLER
    // ----------------------------------------------------
    DisasmOutput output = {0};

    printf("--- Iniciando Disassembler ---\n");
    // O warning 'incompatible pointer types' deve ser resolvido agora que
    // as definições de CodeAttribute e ClassFile vêm dos headers.
    if (disassemble_method(&mock_cf, &mock_code_attr, &output)) {
        printf("Disassembleamento concluido. Total de %u instrucoes.\n", output.count);
        
        // 4. IMPRESSÃO DOS RESULTADOS
        for (u4 i = 0; i < output.count; i++) {
            DisasmInstruction *inst = &output.instructions[i];
            printf("%04x: %s %s // %s\n", 
                   inst->pc, 
                   inst->mnemonic, 
                   inst->args_str, 
                   inst->resolved_info);
        }

        // 5. LIMPEZA
        free_disasm_output(&output); 
        printf("Memoria liberada.\n");
    } else {
        printf("Erro durante o desassembleamento.\n");
    }

    return 0;
}