// src/resolve.c
#define _GNU_SOURCE
#include "resolve.h"
#include "classfile.h" // Necessario para as funcoes de consulta cp_* e tipos
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Definição de Classe para uso interno (evitar warnings)
#define Classe ClassFile
#define INFO_CP CpInfo


// =======================================================
// IMPLEMENTAÇÕES DE FUNÇÕES HELPER (Baseado em classfile.c)
// =======================================================

/**
 * Helper para lidar com Methodref, Fieldref, e InterfaceMethodref (Opcoes 9, 10, 11)
 */
char* resolve_ref_to_string(const Classe *cf, uint16_t idx) {
    const char *out_classe = NULL;
    const char *out_nome = NULL;
    const char *out_desc = NULL;
    static const char *ERRO = "ERRO_REF";

    // OBTEM O PONTEIRO PARA O CP E SEU TAMANHO
    const INFO_CP *cp = cf->constant_pool;
    u2 cp_count = cf->constant_pool_count;

    // 1. CHAMA O HELPER DA PESSOA B: Navegacao dos indices
    cp_referencia_metodo(
        cp,                      // O array do CP
        cp_count,                // Tamanho do CP
        idx,                     // O indice lido do bytecode (Ex: 26)
        &out_classe,             // Retorna o nome da Classe
        &out_nome,               // Retorna o nome do Metodo/Campo
        &out_desc                // Retorna o Descritor
    );

    // Verifica se a Pessoa B falhou (retornou a string VAZIO/""/NULL)
    if (!out_classe || out_classe[0] == '\0' || !out_nome || out_nome[0] == '\0') {
        // Se a referência falhou, retorna erro
        char *err_msg;
        asprintf(&err_msg, "%s #%d", ERRO, idx); // Usa asprintf se disponivel, ou snprintf/malloc
        if (err_msg) return err_msg;
        return strdup(ERRO);
    }
    
    // 2. MONTAGEM FINAL DA STRING
    // Formato: Classe.Nome:Descritor (Calcula tamanho maximo e aloca)
    size_t required_size = strlen(out_classe) + strlen(out_nome) + strlen(out_desc) + 3; 
    char *final_string = (char*)malloc(required_size);

    if (final_string) {
        sprintf(final_string, "%s.%s:%s", out_classe, out_nome, out_desc);
    } else {
        return strdup("ERRO_MEMORIA");
    }

    // Retorna a string final para o disasm.c
    return final_string; 
}


/**
 * Helper para lidar com CONSTANT_Class (Opcao 7)
 */
char* resolve_class_name_to_string(const Classe *cf, uint16_t idx) {
    // OBTEM O PONTEIRO PARA O CP E SEU TAMANHO
    const INFO_CP *cp = cf->constant_pool;
    u2 cp_count = cf->constant_pool_count;

    // 1. CHAMA O HELPER DA PESSOA B: Obtem o nome da Classe
    const char *class_name = cp_nome_classe(cp, cp_count, idx);

    // 2. RETORNA UMA COPIA ALOCADA
    if (class_name && class_name[0] != '\0') {
        return strdup(class_name);
    }
    
    // 3. Em caso de erro
    char *err_msg;
    asprintf(&err_msg, "CLASSE_NAO_ENCONTRADA #%d", idx);
    if (err_msg) return err_msg;
    return strdup("CLASSE_NAO_ENCONTRADA");
}


/**
 * Helper para lidar com constantes literais (Opcao 1, 8, 9)
 * Focado em String/Utf8 para o disasm
 */
char* resolve_literal_to_string(const ClassFile *cf, u2 idx) {
    if (!cf || idx == 0 || idx >= cf->constant_pool_count) return strdup("<?>");
    const CpInfo *cp = cf->constant_pool;
    u1 tag = cp[idx].tag;

    // Ignora slot vazio (após Long/Double)
    if (tag == CONSTANT_None) return strdup("<?>");

    switch (tag) {
        case CONSTANT_String: {
            const char *s = cp_utf8(cp, cf->constant_pool_count, cp[idx].String.string_index);
            if (!s) return strdup("\"?\"");
            size_t n = strlen(s) + 3;
            char *out = (char*)malloc(n);
            snprintf(out, n, "\"%s\"", s);
            return out;
        }

        case CONSTANT_Integer: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%d", (int32_t)cp[idx].Num.bytes);
            return strdup(buf);
        }

        case CONSTANT_Float: {
            char buf[64];
            float f;
            memcpy(&f, &cp[idx].Num.bytes, 4);   // IEEE-754
            snprintf(buf, sizeof(buf), "%g", f); // uso de %g
            return strdup(buf);
        }

        case CONSTANT_Long: {
            char buf[64];
            uint64_t bits = ((uint64_t)cp[idx].LongDouble.high_bytes << 32)
                          |  (uint64_t)cp[idx].LongDouble.low_bytes; // high<<32 | low
            long long v = (long long)bits; // interpretado como assinado
            snprintf(buf, sizeof(buf), "%lld", v);
            return strdup(buf);
        }

        case CONSTANT_Double: {
            char buf[64];
            uint64_t bits = ((uint64_t)cp[idx].LongDouble.high_bytes << 32)
                          |  (uint64_t)cp[idx].LongDouble.low_bytes; // high<<32 | low
            double d;
            memcpy(&d, &bits, 8); // IEEE-754
            snprintf(buf, sizeof(buf), "%f", d); // uso de %g
            return strdup(buf);
        }

        case CONSTANT_Class: {
            // Mostra nome interno da classe (ex: java/lang/String)
            const char *name = cp_utf8(cp, cf->constant_pool_count, cp[idx].Class.name_index);
            return strdup(name ? name : "<?>");
        }

        case CONSTANT_Utf8: {
            // Normalmente ldc não aponta pra Utf8 direto, mas deixamos por segurança
            const char *s = cp_utf8(cp, cf->constant_pool_count, idx);
            return strdup(s ? s : "<?>");
        }

        default:
            return strdup("<?>");
    }
}
