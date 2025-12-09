
#include "classfile.h"
#include <stdlib.h>
#include <string.h>
#include "io.h"

/* Definições de Tipo assumidas para tradução */
// Mantendo PascalCase para tipos
#define Classe ClassFile
#define INFO_CP CpInfo
#define InfoCampo FieldInfo
#define InfoAtributo AttributeInfo
#define Status ClassFileStatus


/* --- helpers internos (Funções em snake_case) --- */
static Status ler_cabecalho(Classe *classe, Buffer *in);
static Status ler_constantes(Classe *classe, Buffer *in);
static Status ler_interfaces(Classe *classe, Buffer *in);
static Status ler_atributos(InfoAtributo **out, u2 *count, Buffer *in);
static Status ler_membros(InfoCampo **out, u2 *count, Buffer *in);

/* Macro simples pra checar “faltou byte” nas leituras do io.h */
// Mantendo SCREAMING_SNAKE_CASE para macros e constantes
#define CHECAR(cond) do { if ((cond)) return ERRO_LEITURA; } while (0)
#define CHECAR_MEMORIA(ptr) do { if (!(ptr)) return ERRO_ALOCACAO; } while (0)

#define ERRO_ALOCACAO CF_STATUS_ERR_ALLOC
#define ERRO_CLASSE CF_STATUS_ERR_BAD_MAGIC
#define ERRO_TAG_CP CF_STATUS_ERR_CP_TAG
#define OK CF_STATUS_OK
#define ERRO_LEITURA CF_STATUS_ERR_OOB 
#define ler_u1 read_u1
#define ler_u2 read_u2
#define ler_u4 read_u4
#define ler_bytes read_bytes

/* ============================================================
 * API pública
 * ============================================================ */
Status ler_classe(Classe *classe, Buffer *in) {
    Status res = ler_cabecalho(classe, in);
    if (res != OK) return res;

    res = ler_constantes(classe, in);
    if (res != OK) return res;

    // Campos da struct Classe/ClassFile
    CHECAR(ler_u2(in, &classe->access_flags));
    CHECAR(ler_u2(in, &classe->this_class));
    CHECAR(ler_u2(in, &classe->super_class));

    res = ler_interfaces(classe, in);
    if (res != OK) return res;

    res = ler_membros(&classe->fields, &classe->fields_count, in);
    if (res != OK) return res;

    res = ler_membros(&classe->methods, &classe->methods_count, in);
    if (res != OK) return res;

    res = ler_atributos(&classe->attributes, &classe->attributes_count, in);
    if (res != OK) return res;

    return OK;
}


void liberar_classe(Classe *classe) {
    if (!classe) return;

    /* CP: libera Utf8, ignora slots vazios (CONSTANT_None) */
    if (classe->constant_pool) {
        for (u2 i = 1; i < classe->constant_pool_count; ++i) {
            if (classe->constant_pool[i].tag == CONSTANT_Utf8) {
                free(classe->constant_pool[i].Utf8.bytes);
            }
        }
        free(classe->constant_pool);
    }

    /* interfaces */
    free(classe->interfaces);

    /* fields/methods + seus atributos */
    if (classe->fields) {
        for (u2 i = 0; i < classe->fields_count; ++i) {
            for (u2 j = 0; j < classe->fields[i].attributes_count; ++j) {
                free(classe->fields[i].attributes[j].info);
            }
            free(classe->fields[i].attributes);
        }
        free(classe->fields);
    }
    if (classe->methods) {
        for (u2 i = 0; i < classe->methods_count; ++i) {
            for (u2 j = 0; j < classe->methods[i].attributes_count; ++j) {
                free(classe->methods[i].attributes[j].info);
            }
            free(classe->methods[i].attributes);
        }
        free(classe->methods);
    }

    /* atributos de classe */
    if (classe->attributes) {
        for (u2 i = 0; i < classe->attributes_count; ++i) {
            free(classe->attributes[i].info);
        }
        free(classe->attributes);
    }

    /* zera  */
    memset(classe, 0, sizeof *classe);
}

/* ============================================================
 * Helpers de consulta
 * ============================================================ */
const char *cp_utf8(const INFO_CP *cp, u2 cp_count, u2 idx) {
    if (!cp || idx == 0 || idx >= cp_count) return "";
    if (cp[idx].tag != CONSTANT_Utf8) return "";
    return cp[idx].Utf8.bytes ? cp[idx].Utf8.bytes : "";
}

const char *cp_nome_classe(const INFO_CP *cp, u2 cp_count, u2 idx) {
    if (!cp || idx == 0 || idx >= cp_count) return "";
    if (cp[idx].tag != CONSTANT_Class) return "";
    u2 ni = cp[idx].Class.name_index;
    return cp_utf8(cp, cp_count, ni);
}

void cp_referencia_metodo(const INFO_CP *cp, u2 cp_count, u2 idx,
                        const char **out_classe, const char **out_nome, const char **out_desc) {
    static const char *VAZIO = "";
    const char *cls = VAZIO, *nam = VAZIO, *des = VAZIO;

    if (cp && idx > 0 && idx < cp_count) {
        u1 tag = cp[idx].tag;
        if (tag == CONSTANT_Methodref || tag == CONSTANT_InterfaceMethodref || tag == CONSTANT_Fieldref) {
            u2 ci = cp[idx].Ref.class_index;
            u2 nt = cp[idx].Ref.name_and_type_index;
            cls = cp_nome_classe(cp, cp_count, ci);
            if (nt > 0 && nt < cp_count && cp[nt].tag == CONSTANT_NameAndType) {
                nam = cp_utf8(cp, cp_count, cp[nt].NameAndType.name_index);
                des = cp_utf8(cp, cp_count, cp[nt].NameAndType.descriptor_index);
            }
        }
    }
    if (out_classe) *out_classe = cls;
    if (out_nome) *out_nome = nam;
    if (out_desc) *out_desc = des;
}

/* ============================================================
 * Implementações internas 
 * ============================================================ */

static Status ler_cabecalho(Classe *classe, Buffer *in) {
    CHECAR(ler_u4(in, &classe->magic));
    if (classe->magic != 0xCAFEBABE) return ERRO_CLASSE;

    CHECAR(ler_u2(in, &classe->minor_version));
    CHECAR(ler_u2(in, &classe->major_version));

    CHECAR(ler_u2(in, &classe->constant_pool_count));
    classe->constant_pool = (INFO_CP*)calloc(classe->constant_pool_count, sizeof(INFO_CP));
    CHECAR_MEMORIA(classe->constant_pool);

    return OK;
}

static Status ler_constantes(Classe *classe, Buffer *in) {
    INFO_CP *cp = classe->constant_pool;
    u2 count = classe->constant_pool_count;

    for (u2 i = 1; i < count; ++i) {
        u1 tag; CHECAR(ler_u1(in, &tag));
        cp[i].tag = tag;

        switch (tag) {
        case CONSTANT_Utf8: {
            u2 len; CHECAR(ler_u2(in, &len));
            cp[i].Utf8.length = len;
            cp[i].Utf8.bytes = (char*)malloc((size_t)len + 1);
            CHECAR_MEMORIA(cp[i].Utf8.bytes);
            CHECAR(ler_bytes(in, (u1*)cp[i].Utf8.bytes, len));
            cp[i].Utf8.bytes[len] = '\0';
        } break;

        case CONSTANT_Integer:
        case CONSTANT_Float: {
            CHECAR(ler_u4(in, &cp[i].Num.bytes));
        } break;

        case CONSTANT_Long:
        case CONSTANT_Double: {
            CHECAR(ler_u4(in, &cp[i].LongDouble.high_bytes));
            CHECAR(ler_u4(in, &cp[i].LongDouble.low_bytes));
            /* ocupa 2 slots */
            if (i + 1 >= count) return ERRO_TAG_CP;
            cp[i + 1].tag = CONSTANT_None;
            ++i; /* pula o slot vazio */
        } break;

        case CONSTANT_Class: {
            CHECAR(ler_u2(in, &cp[i].Class.name_index));
        } break;

        case CONSTANT_String: {
            CHECAR(ler_u2(in, &cp[i].String.string_index));
        } break;

        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref: {
            CHECAR(ler_u2(in, &cp[i].Ref.class_index));
            CHECAR(ler_u2(in, &cp[i].Ref.name_and_type_index));
        } break;

        case CONSTANT_NameAndType: {
            CHECAR(ler_u2(in, &cp[i].NameAndType.name_index));
            CHECAR(ler_u2(in, &cp[i].NameAndType.descriptor_index));
        } break;

        case CONSTANT_MethodHandle: {
            CHECAR(ler_u1(in, &cp[i].MethodHandle.reference_kind));
            CHECAR(ler_u2(in, &cp[i].MethodHandle.reference_index));
        } break;

        case CONSTANT_MethodType: {
            CHECAR(ler_u2(in, &cp[i].MethodType.descriptor_index));
        } break;

        case CONSTANT_InvokeDynamic: {
            CHECAR(ler_u2(in, &cp[i].InvokeDynamic.bootstrap_method_attr_index));
            CHECAR(ler_u2(in, &cp[i].InvokeDynamic.name_and_type_index));
        } break;

        /* Se não quiser suportar outras tags, retorne erro: */
        case CONSTANT_Dynamic:
        case CONSTANT_Module:
        case CONSTANT_Package:
        default:
            return ERRO_TAG_CP;
        }
    }

    return OK;
}

static Status ler_interfaces(Classe *classe, Buffer *in) {
    CHECAR(ler_u2(in, &classe->interfaces_count));
    if (classe->interfaces_count == 0) return OK;

    classe->interfaces = (u2*)malloc(sizeof(u2) * classe->interfaces_count);
    CHECAR_MEMORIA(classe->interfaces);

    for (u2 i = 0; i < classe->interfaces_count; ++i) {
        CHECAR(ler_u2(in, &classe->interfaces[i])); /* cada um deve referenciar CONSTANT_Class */
    }
    return OK;
}

static Status ler_atributos(InfoAtributo **out, u2 *count, Buffer *in) {
    CHECAR(ler_u2(in, count));
    if (*count == 0) { *out = NULL; return OK; }

    InfoAtributo *attrs = (InfoAtributo*)calloc(*count, sizeof(InfoAtributo));
    CHECAR_MEMORIA(attrs);

    for (u2 i = 0; i < *count; ++i) {
        CHECAR(ler_u2(in, &attrs[i].attribute_name_index));
        CHECAR(ler_u4(in, &attrs[i].attribute_length));

        if (attrs[i].attribute_length > 0) {
            attrs[i].info = (u1*)malloc(attrs[i].attribute_length);
            if (!attrs[i].info) { free(attrs); return ERRO_ALOCACAO; }
            CHECAR(ler_bytes(in, attrs[i].info, attrs[i].attribute_length));
        }
    }

    *out = attrs;
    return OK;
}

static Status ler_membros(InfoCampo **out, u2 *count, Buffer *in) {
    CHECAR(ler_u2(in, count));
    if (*count == 0) { *out = NULL; return OK; }

    InfoCampo *arr = (InfoCampo*)calloc(*count, sizeof(InfoCampo));
    CHECAR_MEMORIA(arr);

    for (u2 i = 0; i < *count; ++i) {
        CHECAR(ler_u2(in, &arr[i].access_flags));
        CHECAR(ler_u2(in, &arr[i].name_index));
        CHECAR(ler_u2(in, &arr[i].descriptor_index));

        Status st = ler_atributos(&arr[i].attributes, &arr[i].attributes_count, in);
        if (st != OK) {
            /* rollback parcial: liberar o que já foi lido */
            for (u2 k = 0; k < i; ++k) {
                for (u2 j = 0; j < arr[k].attributes_count; ++j) free(arr[k].attributes[j].info);
                free(arr[k].attributes);
            }
            free(arr);
            return st;
        }
    }

    *out = arr;
    return OK;
}ClassFileStatus parse_classfile(ClassFile *out, Buffer *in) { return ler_classe(out, in); } void free_classfile(ClassFile *cf) { liberar_classe(cf); }
