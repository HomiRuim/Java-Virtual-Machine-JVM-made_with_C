/* --- src/print.c --- */
/*
 * Responsabilidade: Pessoa E
 * Implementa a saida "pretty" (impressao humana) do ClassFile.
 * Utiliza as APIs das Pessoas B, C, e D para resolver nomes e
 * fazer o disassembly.
 */

#include "print.h" // Nosso contrato (ja inclui base.h, classfile.h, cli.h)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// APIs das outras equipes que precisamos
#include "attributes.h" // Pessoa C (parse_code_attribute, free_code_attribute)
#include "resolve.h"    // Pessoa D (resolve_*, funcoes de consulta)
#include "disasm.h"     // Pessoa D (disassemble_method, free_disasm_output)


/* -----------------------------------------------------------
 * Helpers de versão (Java e compilador C)
 * ----------------------------------------------------------- */
static const char* java_version_str(int major) {
    switch (major) {
        case 45: return "Java 1.1";
        case 46: return "Java 1.2";
        case 47: return "Java 1.3";
        case 48: return "Java 1.4";
        case 49: return "Java 5";
        case 50: return "Java 6";
        case 51: return "Java 7";
        case 52: return "Java 8";
        case 53: return "Java 9";
        case 54: return "Java 10";
        case 55: return "Java 11";
        case 56: return "Java 12";
        case 57: return "Java 13";
        case 58: return "Java 14";
        case 59: return "Java 15";
        case 60: return "Java 16";
        case 61: return "Java 17";
        case 62: return "Java 18";
        case 63: return "Java 19";
        case 64: return "Java 20";
        case 65: return "Java 21";
        case 66: return "Java 22";
        default: return "Versão desconhecida";
    }
}

static const char* compiler_version_str(void) {
#if defined(__clang__)
    static char buf[64];
    sprintf(buf, "Clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
    return buf;
#elif defined(__GNUC__)
    static char buf[64];
    sprintf(buf, "GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    return buf;
#elif defined(_MSC_VER)
    static char buf[64];
    sprintf(buf, "MSVC %d", _MSC_VER);
    return buf;
#else
    return "Compilador desconhecido";
#endif
}



/* --- Funcoes Auxiliares (Helpers) --- */

/**
 * @brief Helper para encontrar um atributo CRU (raw) pelo nome.
 * (Ex: "Code", "SourceFile")
 */
static const AttributeInfo* find_raw_attribute_by_name(
    const ClassFile *cf, 
    const AttributeInfo *attributes, 
    u2 attributes_count, 
    const char *name
) {
    for (u2 i = 0; i < attributes_count; i++) {
        const AttributeInfo *attr = &attributes[i];
        
        // Usa a API da Pessoa B para ler o nome do atributo do Constant Pool
        const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count, 
                                        attr->attribute_name_index);
        
        if (attr_name && strcmp(attr_name, name) == 0) {
            return attr;
        }
    }
    return NULL;
}


/**
 * @brief Imprime o corpo de um metodo, incluindo o disassembly.
 * Esta funcao chama as Pessoas C e D.
 */
static void print_method_body(const ClassFile *cf, const MethodInfo *method, const CliOptions *options) {
    
    // Respeita a flag --no-code
    if (!options->disassemble_code) {
        printf("    [Disassembly desativado via --no-code]\n");
        return;
    }

    // 1. Encontra o atributo "Code"
    const AttributeInfo *code_attr_raw = find_raw_attribute_by_name(
        cf, 
        method->attributes, 
        method->attributes_count,
        "Code"
    );

    if (!code_attr_raw) {
        printf("    (Metodo abstrato ou nativo - sem atributo Code)\n");
        return;
    }

    // 2. Chama Pessoa C para parsear o atributo
    CodeAttribute parsed_code;
    memset(&parsed_code, 0, sizeof(CodeAttribute)); 
    
    Status parse_status = parse_code_attribute(cf, code_attr_raw, &parsed_code);
    if (parse_status != OK) {
        fprintf(stderr, "    Erro: Falha ao parsear atributo Code (Pessoa C). Codigo: %d\n", parse_status);
        return;
    }

    printf("    Code (max_stack=%u, max_locals=%u, code_length=%u):\n",
           parsed_code.max_stack, parsed_code.max_locals, parsed_code.code_length);

    // 3. Chama Pessoa D para o disassembly
    DisasmOutput disasm_out;
    memset(&disasm_out, 0, sizeof(DisasmOutput));

    // A API da Pessoa D (disasm.h) pode não ser 'const-correct', 
    // então fazemos um cast se necessário.
    bool disasm_status = disassemble_method((ClassFile*)cf, &parsed_code, &disasm_out);

    if (disasm_status) {
        // Itera sobre as instrucoes desmontadas
        for (u4 i = 0; i < disasm_out.count; i++) {
            DisasmInstruction *inst = &disasm_out.instructions[i];
            
            // Imprime: PC: MNEMONIC ARGS // RESOLVIDO
            printf("      %04u: %-15s %-10s", 
                   (unsigned int)inst->pc, 
                   inst->mnemonic ? inst->mnemonic : "<?>",
                   inst->args_str ? inst->args_str : "");
                   
            if (inst->resolved_info && inst->resolved_info[0] != '\0') {
                printf("// %s", inst->resolved_info);
            }
            printf("\n");
        }
    } else {
        fprintf(stderr, "    Erro: Falha ao fazer disassembly (Pessoa D).\n");
    }

    // 4. Limpeza (Libera memoria alocada pelas Pessoas D e C)
    free_disasm_output(&disasm_out);
    free_code_attribute(&parsed_code);
}


/* --- Funcao Publica (contrato de print.h) --- */

/**
 * @brief Imprime o ClassFile de forma legivel, 
 * respeitando as flags de CLI.
 */
Status print_classfile(ClassFile *cf, const CliOptions *options) {
    
    // --- 1. Cabeçalho (Header) ---
    if (options->print_header) {
        printf("--- ClassFile (Pretty Print) ---\n");
        printf("Magic: 0x%08X\n", cf->magic);
        printf("Major Version: %u (0x%04X)  ->  %s\n",
            cf->major_version, cf->major_version,
            java_version_str(cf->major_version));

        printf("Minor Version: %u (0x%04X)\n",
            cf->minor_version, cf->minor_version);


        printf("C Compiler: %s\n", compiler_version_str());

        printf("Constant Pool Count: %u\n", cf->constant_pool_count);
        printf("Access Flags: 0x%04X\n", cf->access_flags);

        // Resolve 'this_class' e 'super_class' usando a API da Pessoa D
        char *this_class = resolve_class_name_to_string(cf, cf->this_class);
        char *super_class = resolve_class_name_to_string(cf, cf->super_class);

        printf("This Class:  #%u // %s\n", cf->this_class, this_class ? this_class : "ERRO_RESOLVE");
        printf("Super Class: #%u // %s\n", cf->super_class, super_class ? super_class : "ERRO_RESOLVE");
        

        // A API de Pessoa D aloca memoria, precisamos liberar
        free(this_class);
        free(super_class);
    }

    // --- 2. Interfaces ---
    if (options->print_interfaces) {
        printf("\nInterfaces (%u):\n", cf->interfaces_count);
        for (u2 i = 0; i < cf->interfaces_count; i++) {
            u2 interface_idx = cf->interfaces[i];
            char *if_name = resolve_class_name_to_string(cf, interface_idx);
            printf("  - #%u // %s\n", interface_idx, if_name ? if_name : "ERRO_RESOLVE");
            free(if_name);
        }
    }
    if (options->print_constant_pool) {
            printf("\nConstant Pool (%u entries):\n", cf->constant_pool_count - 1);
            for (u2 i = 1; i < cf->constant_pool_count; i++) {
                CpInfo *cp = &cf->constant_pool[i];
                printf("  #%u = ", i);

                switch (cp->tag) {
                    case CONSTANT_Class:
                        printf("Class\t\t#%u // %s\n",
                            cp->Class.name_index,
                            cp_utf8(cf->constant_pool, cf->constant_pool_count, cp->Class.name_index));
                        break;

                    case CONSTANT_Fieldref:
                    case CONSTANT_Methodref:
                    case CONSTANT_InterfaceMethodref: {
                        const char *cls = cp_nome_classe(cf->constant_pool, cf->constant_pool_count, cp->Ref.class_index);
                        const CpInfo *nt = &cf->constant_pool[cp->Ref.name_and_type_index];
                        const char *nm  = cp_utf8(cf->constant_pool, cf->constant_pool_count, nt->NameAndType.name_index);
                        const char *ds  = cp_utf8(cf->constant_pool, cf->constant_pool_count, nt->NameAndType.descriptor_index);
                        const char *kind = (cp->tag == CONSTANT_Fieldref) ? "Fieldref" :
                                        (cp->tag == CONSTANT_Methodref) ? "Methodref" : "InterfaceMethodref";
                        printf("%s\t#%u.#%u // %s.%s:%s\n", kind,
                            cp->Ref.class_index, cp->Ref.name_and_type_index,
                            cls ? cls : "?", nm ? nm : "?", ds ? ds : "?");
                        break;
                    }

                    case CONSTANT_String:
                        printf("String\t\t#%u // \"%s\"\n",
                            cp->String.string_index,
                            cp_utf8(cf->constant_pool, cf->constant_pool_count, cp->String.string_index));
                        break;

                    case CONSTANT_Integer:
                        printf("Integer\t\t%d\n", (int32_t)cp->Num.bytes);
                        break;

                    case CONSTANT_Float: {
                        float f;
                        memcpy(&f, &cp->Num.bytes, 4);
                        printf("Float\t\t%g\n", f);
                        break;
                    }

                    case CONSTANT_Long: {
                        uint64_t val = ((uint64_t)cp->LongDouble.high_bytes << 32) | cp->LongDouble.low_bytes;
                        printf("Long\t\t%lld\n", (long long)val);
                        i++; // Longs ocupam 2 slots
                        break;
                    }

                    case CONSTANT_Double: {
                        uint64_t bits = ((uint64_t)cp->LongDouble.high_bytes << 32) | cp->LongDouble.low_bytes;
                        double d;
                        memcpy(&d, &bits, 8);
                        printf("Double\t\t%g\n", d);
                        i++; // também ocupa 2 slots
                        break;
                    }

                    case CONSTANT_Utf8:
                        printf("Utf8\t\t%s\n", cp->Utf8.bytes);
                        break;
                    case CONSTANT_NameAndType:
                        printf("NameAndType\t#%u:#%u // %s:%s\n",
                            cp->NameAndType.name_index,
                            cp->NameAndType.descriptor_index,
                            cp_utf8(cf->constant_pool, cf->constant_pool_count, cp->NameAndType.name_index),
                            cp_utf8(cf->constant_pool, cf->constant_pool_count, cp->NameAndType.descriptor_index));
                        break;

                    default:
                        printf("Unknown tag %u\n", cp->tag);
                }
            }
        }



    // --- 3. Fields (Campos) ---
    if (options->print_fields) {
        printf("\nFields (%u):\n", cf->fields_count);
        for (u2 i = 0; i < cf->fields_count; i++) {
            FieldInfo *field = &cf->fields[i];
            
            // Usa Pessoa D (resolve.h) para obter os nomes
            char *field_name = resolve_literal_to_string(cf, field->name_index);
            char *field_desc = resolve_literal_to_string(cf, field->descriptor_index);

            printf("  - %s (Desc: %s), Flags: 0x%04X\n", 
                   field_name ? field_name : "?", 
                   field_desc ? field_desc : "?", 
                   field->access_flags);
            
            free(field_name);
            free(field_desc);
        }
    }

    // --- 4. Methods (Métodos) ---
    if (options->print_methods) {
        printf("\nMethods (%u):\n", cf->methods_count);
        for (u2 i = 0; i < cf->methods_count; i++) {
            MethodInfo *method = &cf->methods[i];
            
            char *method_name = resolve_literal_to_string(cf, method->name_index);
            char *method_desc = resolve_literal_to_string(cf, method->descriptor_index);

            printf("  ----------------------------------\n");
            printf("  %s%s\n", 
                   method_name ? method_name : "?", 
                   method_desc ? method_desc : "?");
            printf("  Flags: 0x%04X\n", method->access_flags);
            
            free(method_name);
            free(method_desc);
            
            // Chama o helper para o disassembly
            print_method_body(cf, method, options);
        }
        printf("  ----------------------------------\n");
    }

    // --- 5. Atributos de Classe (ex: SourceFile) ---
    if (options->print_attributes) {
        printf("\nAttributes (%u):\n", cf->attributes_count);
        for (u2 i = 0; i < cf->attributes_count; i++) {
            AttributeInfo *attr = &cf->attributes[i];
            const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count, 
                                            attr->attribute_name_index);

            if (attr_name && strcmp(attr_name, "SourceFile") == 0 && attr->attribute_length == 2) {
                // atributo SourceFile tem payload de 2 bytes = índice para CONSTANT_Utf8 com o nome
                u2 idx = (attr->info[0] << 8) | attr->info[1];
                const char *filename = cp_utf8(cf->constant_pool, cf->constant_pool_count, idx);
                printf("  - SourceFile: %s\n", filename ? filename : "?");
            } else {
                // impressão genérica para qualquer outro atributo
                printf("  - %s (Length: %u)\n", attr_name ? attr_name : "?", attr->attribute_length);
            }
        }
    }

    
    printf("\n--- Fim (Pretty Print) ---\n");
    return OK; // (de base.h)
}