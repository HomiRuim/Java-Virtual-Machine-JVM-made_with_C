#include "attributes.h"
#include "classfile.h"
#include "io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.class>\n", argv[0]);
        return 1;
    }

    Buffer buf;
    Status io_status = buffer_from_file(argv[1], &buf);
    if (io_status != OK) {
        fprintf(stderr, "Erro ao abrir arquivo: %d\n", io_status);
        return 1;
    }

    ClassFile cf;
    ClassFileStatus cf_status = parse_classfile(&cf, &buf);
    if (cf_status != CF_STATUS_OK) {
        fprintf(stderr, "Erro ao parsear classfile: %d\n", cf_status);
        buffer_free(&buf);
        return 1;
    }

    printf("=== Teste do Módulo de Atributos (Pessoa C) ===\n\n");
    printf("Arquivo: %s\n", argv[1]);
    printf("Magic: 0x%08X\n", cf.magic);
    printf("Version: %d.%d\n", cf.major_version, cf.minor_version);
    printf("Métodos encontrados: %d\n\n", cf.methods_count);

    for (u2 i = 0; i < cf.methods_count; i++) {
        MethodInfo *method = &cf.methods[i];
        const char *method_name = cp_utf8(cf.constant_pool, cf.constant_pool_count, method->name_index);
        const char *method_desc = cp_utf8(cf.constant_pool, cf.constant_pool_count, method->descriptor_index);
        
        printf("--- Método #%d: %s%s ---\n", i + 1, method_name, method_desc);
        printf("  Atributos: %d\n", method->attributes_count);

        int found_code = 0;
        for (u2 j = 0; j < method->attributes_count; j++) {
            const char *attr_name = cp_utf8(cf.constant_pool, cf.constant_pool_count, 
                                           method->attributes[j].attribute_name_index);
            printf("    [%d] %s (tamanho: %u bytes)\n", j + 1, attr_name, method->attributes[j].attribute_length);

            if (strcmp(attr_name, "Code") == 0) {
                found_code = 1;
                CodeAttribute code;
                Status code_status = parse_code_attribute(&cf, &method->attributes[j], &code);
                
                if (code_status == OK) {
                    printf("\n    ✓ Atributo Code parseado com sucesso!\n");
                    printf("      max_stack       = %d\n", code.max_stack);
                    printf("      max_locals      = %d\n", code.max_locals);
                    printf("      code_length     = %u bytes\n", code.code_length);
                    printf("      exception_table = %d entradas\n", code.exception_table_length);
                    printf("      sub-atributos   = %d\n", code.attributes_count);
                    
                    if (code.line_number_table_length > 0) {
                        printf("\n      ✓ LineNumberTable encontrada: %d entradas\n", code.line_number_table_length);
                        for (u2 k = 0; k < code.line_number_table_length && k < 5; k++) {
                            printf("        PC %d → linha %d\n", 
                                   code.line_number_table[k].start_pc,
                                   code.line_number_table[k].line_number);
                        }
                        if (code.line_number_table_length > 5) {
                            printf("        ... e mais %d entradas\n", code.line_number_table_length - 5);
                        }
                    }
                    
                    if (code.local_variable_table_length > 0) {
                        printf("\n      ✓ LocalVariableTable encontrada: %d entradas\n", code.local_variable_table_length);
                    }

                    printf("\n      Primeiros bytes do bytecode:\n      ");
                    for (u4 k = 0; k < code.code_length && k < 16; k++) {
                        printf("%02X ", code.code[k]);
                    }
                    if (code.code_length > 16) {
                        printf("...");
                    }
                    printf("\n");

                    free_code_attribute(&code);
                } else {
                    printf("    ✗ Erro ao parsear Code: %d\n", code_status);
                }
            }
        }

        if (!found_code) {
            printf("  (Método abstrato ou nativo - sem bytecode)\n");
        }
        printf("\n");
    }

    printf("=== Teste Concluído ===\n");

    free_classfile(&cf);
    buffer_free(&buf);
    return 0;
}
