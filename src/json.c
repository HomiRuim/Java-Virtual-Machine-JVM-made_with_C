/* --- src/json.c --- */
/*
 * Responsabilidade: Pessoa E
 * Implementa a saida JSON do ClassFile.
 */

#include "json.h"       // Nosso contrato (inclui base.h, classfile.h, cli.h)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// APIs das outras equipes
#include "resolve.h"    // Pessoa D (para resolver nomes)
#include "attributes.h" // Pessoa C (para parsear Code)
#include "disasm.h"     // Pessoa D (para disassembly)

/* --- Protótipos Estáticos (Forward Declarations) --- */

static void json_print_string(const char *str);

static const AttributeInfo* find_raw_attribute_by_name(
    const ClassFile *cf, 
    const AttributeInfo *attributes, 
    u2 attributes_count, 
    const char *name
);

static void json_print_cp(ClassFile *cf); // Corrigindo erro
static void json_print_fields(ClassFile *cf, const CliOptions *options); // Corrigindo erro

static void json_print_code_attribute(
    ClassFile *cf, 
    const MethodInfo *method, 
    const CliOptions *options
);

static void json_print_methods(ClassFile *cf, const CliOptions *options);


/* --- Implementações das Funções --- */

// Helper para escapar strings para JSON
static void json_print_string(const char *str) {
    if (!str) {
        printf("\"\"");
        return;
    }
    printf("\"");
    while (*str) {
        switch (*str) {
            case '\"': printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:
                if (*str >= 0 && *str < 32) {
                    printf("\\u00%02x", (unsigned char)*str);
                } else if (*str == 127) {
                     printf("\\u007f");
                }
                else {
                    putchar(*str);
                }
                break;
        }
        str++;
    }
    printf("\"");
}

/**
 * @brief Helper para encontrar um atributo CRU (raw) pelo nome.
 */
static const AttributeInfo* find_raw_attribute_by_name(
    const ClassFile *cf, 
    const AttributeInfo *attributes, 
    u2 attributes_count, 
    const char *name
) {
    for (u2 i = 0; i < attributes_count; i++) {
        const AttributeInfo *attr = &attributes[i];
        
        const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count, 
                                        attr->attribute_name_index);
        
        if (attr_name && strcmp(attr_name, name) == 0) {
            return attr;
        }
    }
    return NULL;
}

/**
 * @brief (IMPLEMENTAÇÃO FALTANTE) Helper para imprimir o Constant Pool
 */
static void json_print_cp(ClassFile *cf) {
    printf("  \"constant_pool\": [\n");
    printf("    null"); // CP[0] é sempre nulo

    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        printf(",\n    { \"index\": %u, ", i);
        CpInfo *cp = &cf->constant_pool[i];
        
        switch (cp->tag) {
            case CONSTANT_Utf8:
                printf("\"tag\": \"Utf8\", \"value\": ");
                json_print_string(cp->Utf8.bytes);
                break;
            case CONSTANT_Class:
                printf("\"tag\": \"Class\", \"name_index\": %u", cp->Class.name_index);
                break;
            case CONSTANT_String:
                printf("\"tag\": \"String\", \"string_index\": %u", cp->String.string_index);
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                printf("\"tag\": \"%s\", \"class_index\": %u, \"name_and_type_index\": %u",
                       cp->tag == CONSTANT_Fieldref ? "Fieldref" : (cp->tag == CONSTANT_Methodref ? "Methodref" : "InterfaceMethodref"),
                       cp->Ref.class_index, cp->Ref.name_and_type_index);
                break;
            case CONSTANT_NameAndType:
                printf("\"tag\": \"NameAndType\", \"name_index\": %u, \"descriptor_index\": %u",
                       cp->NameAndType.name_index, cp->NameAndType.descriptor_index);
                break;
            case CONSTANT_Integer:
                printf("\"tag\": \"Integer\", \"value\": %d", (int32_t)cp->Num.bytes);
                break;
            case CONSTANT_None:
                printf("\"tag\": \"None (Slot Vazio)\"");
                break;
            default:
                printf("\"tag\": \"TAG_DESCONHECIDA (%u)\"", cp->tag);
        }
        printf(" }");
        
        if (cp->tag == CONSTANT_Long || cp->tag == CONSTANT_Double) {
            i++; // Pula o proximo slot
        }
    }
    printf("\n  ]"); // Fim do array constant_pool
}

/**
 * @brief (IMPLEMENTAÇÃO FALTANTE) Helper para imprimir Fields
 */
static void json_print_fields(ClassFile *cf, const CliOptions *options) {
    printf("  \"fields\": [\n");
    for (u2 i = 0; i < cf->fields_count; i++) {
        FieldInfo *field = &cf->fields[i];
        
        char *name = resolve_literal_to_string(cf, field->name_index);
        char *desc = resolve_literal_to_string(cf, field->descriptor_index);

        printf("    { \"name\": ");
        json_print_string(name);
        printf(", \"descriptor\": ");
        json_print_string(desc);
        printf(", \"access_flags\": %u }", field->access_flags);

        free(name);
        free(desc);
        
        if (i < cf->fields_count - 1) printf(",\n");
    }
    printf("\n  ]"); // Fim do array fields
}


/**
 * @brief Imprime o Atributo Code e o Disassembly em JSON.
 */
static void json_print_code_attribute(ClassFile *cf, const MethodInfo *method, const CliOptions *options) {
    
    if (!options->disassemble_code) {
        printf(",\n      \"code_attribute\": { \"status\": \"omitido via --no-code\" }");
        return;
    }

    const AttributeInfo *code_attr_raw = find_raw_attribute_by_name(
        cf, 
        method->attributes, 
        method->attributes_count,
        "Code"
    );

    if (!code_attr_raw) {
        printf(",\n      \"code_attribute\": { \"status\": \"ausente (abstrato/nativo)\" }");
        return;
    }

    CodeAttribute parsed_code;
    memset(&parsed_code, 0, sizeof(CodeAttribute)); 
    
    Status parse_status = parse_code_attribute(cf, code_attr_raw, &parsed_code);
    if (parse_status != OK) {
        printf(",\n      \"code_attribute\": { \"status\": \"erro no parse (Pessoa C)\", \"codigo\": %d }", parse_status);
        return;
    }

    printf(",\n      \"code_attribute\": {\n");
    printf("        \"status\": \"parsed\",\n");
    printf("        \"max_stack\": %u,\n", parsed_code.max_stack);
    printf("        \"max_locals\": %u,\n", parsed_code.max_locals);
    printf("        \"code_length\": %u", parsed_code.code_length); // Virgula removida

    DisasmOutput disasm_out;
    memset(&disasm_out, 0, sizeof(DisasmOutput));

    bool disasm_status = disassemble_method(cf, &parsed_code, &disasm_out);

    if (disasm_status && disasm_out.count > 0) {
        printf(",\n        \"disassembly\": [\n"); // Virgula adicionada
        
        for (u4 i = 0; i < disasm_out.count; i++) {
            DisasmInstruction *inst = &disasm_out.instructions[i];
            
            printf("          { \"pc\": %u, \"mnemonic\": ", (unsigned int)inst->pc);
            json_print_string(inst->mnemonic);
            printf(", \"args\": ");
            json_print_string(inst->args_str);
            
            if (inst->resolved_info && inst->resolved_info[0] != '\0') {
                printf(", \"resolved\": ");
                json_print_string(inst->resolved_info);
            }
            
            printf(" }"); // Fim do objeto instrução
            if (i < disasm_out.count - 1) printf(",\n");
        }
        
        printf("\n        ]\n"); // Fim do array de instruções
    } else if (disasm_status) {
         printf(",\n        \"disassembly\": []\n"); // Array vazio
    } else {
        printf(",\n        \"disassembly\": { \"status\": \"erro no disassembly (Pessoa D)\" }\n"); // Virgula adicionada
    }
    
    printf("      }"); // Fim do objeto code_attribute

    free_disasm_output(&disasm_out);
    free_code_attribute(&parsed_code);
}


/**
 * @brief Imprime Methods, agora com disassembly.
 */
static void json_print_methods(ClassFile *cf, const CliOptions *options) {
    printf("  \"methods\": [\n");
    for (u2 i = 0; i < cf->methods_count; i++) {
        MethodInfo *method = &cf->methods[i];
        
        char *name = resolve_literal_to_string(cf, method->name_index);
        char *desc = resolve_literal_to_string(cf, method->descriptor_index);

        printf("    { \"name\": ");
        json_print_string(name);
        printf(", \"descriptor\": ");
        json_print_string(desc);
        printf(", \"access_flags\": %u", method->access_flags);
        
        // Chama o helper para o disassembly
        json_print_code_attribute(cf, method, options);

        printf("\n    }"); // Fim do objeto método

        free(name);
        free(desc);
        
        if (i < cf->methods_count - 1) printf(",\n");
    }
    printf("\n  ]"); // Fim do array methods
}


/* --- Funcao Publica (contrato de json.h) --- */
Status json_classfile(ClassFile *cf, const CliOptions *options) {
    
    char *this_class = NULL;
    char *super_class = NULL;

    printf("{\n"); // Inicio do objeto JSON principal

    // 1. Header e Info da Classe
    this_class = resolve_class_name_to_string(cf, cf->this_class);
    super_class = resolve_class_name_to_string(cf, cf->super_class);

    printf("  \"header\": {\n");
    printf("    \"magic\": \"0x%X\",\n", cf->magic);
    printf("    \"major_version\": %u,\n", cf->major_version);
    printf("    \"minor_version\": %u\n", cf->minor_version);
    printf("  },\n");

    printf("  \"class_info\": {\n");
    printf("    \"this_class\": ");
    json_print_string(this_class);
    printf(",\n    \"super_class\": ");
    json_print_string(super_class);
    printf(",\n    \"access_flags\": %u\n", cf->access_flags);
    printf("  },\n");
    
    free(this_class);
    free(super_class);

    // 2. Constant Pool
    if (options->print_constant_pool) {
        json_print_cp(cf);
        printf(",\n");
    }

    // 3. Fields
    if (options->print_fields) {
        json_print_fields(cf, options);
        printf(",\n");
    }

    // 4. Methods
    if (options->print_methods) {
        json_print_methods(cf, options);
    } else {
        printf("  \"methods\": []");
    }
    
    printf("\n}\n"); // Fim do objeto JSON principal
    
    return OK;
}