#ifndef CLASSFILE_H
#define CLASSFILE_H

/* Tipos básicos e I/O: ajuste se no seu projeto esses tipos morarem em outro header */
#include "io.h"   /* u1, u2, u4, Buffer, read_u1/u2/u4/read_bytes */
#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------
 * Tags do Constant Pool (inclui sentinela p/ slot vazio)
 * ----------------------------------------------------------- */
enum {
    CONSTANT_None               = 0,  /* slot vazio após Long/Double */
    CONSTANT_Utf8               = 1,
    CONSTANT_Integer            = 3,
    CONSTANT_Float              = 4,
    CONSTANT_Long               = 5,
    CONSTANT_Double             = 6,
    CONSTANT_Class              = 7,
    CONSTANT_String             = 8,
    CONSTANT_Fieldref           = 9,
    CONSTANT_Methodref          = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_NameAndType        = 12,
    CONSTANT_MethodHandle       = 15,
    CONSTANT_MethodType         = 16,
    CONSTANT_Dynamic            = 17,
    CONSTANT_InvokeDynamic      = 18,
    CONSTANT_Module             = 19,
    CONSTANT_Package            = 20
};

/* -----------------------------------------------------------
 * Estruturas
 * ----------------------------------------------------------- */

/* Entrada genérica do Constant Pool */
typedef struct {
    u1 tag;
    union {
        struct { u2 length; char *bytes; } Utf8;                 /* bytes[length] == '\0' */
        struct { u2 name_index; } Class;
        struct { u2 string_index; } String;
        struct { u2 class_index, name_and_type_index; } Ref;     /* Field/Method/InterfaceMethod */
        struct { u2 name_index, descriptor_index; } NameAndType;
        struct { u4 bytes; } Num;                                 /* Integer/Float bruto */
        struct { u4 high_bytes, low_bytes; } LongDouble;          /* Long/Double, 2 slots */
        struct { u1 reference_kind; u2 reference_index; } MethodHandle;
        struct { u2 descriptor_index; } MethodType;
        struct { u2 bootstrap_method_attr_index, name_and_type_index; } InvokeDynamic;
        /* Dynamic/Module/Package podem ser adicionados se necessário */
    };
} CpInfo;

/* Atributo cru (payload bruto) */
typedef struct {
    u2 attribute_name_index;
    u4 attribute_length;
    u1 *info;   /* Copiado do arquivo; outro módulo interpreta */
} AttributeInfo;

/* Cabeçalho comum de Field/Method */
typedef struct {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    AttributeInfo *attributes;
} FieldInfo;

typedef FieldInfo MethodInfo;

/* Estrutura principal representando um .class na memória */
typedef struct class_file {
    u4 magic;
    u2 minor_version, major_version;

    u2 constant_pool_count;
    CpInfo *constant_pool; /* índices válidos: [1 .. constant_pool_count-1]; 0 é inválido */

    u2 access_flags, this_class, super_class;

    u2 interfaces_count;
    u2 *interfaces;        /* cada entrada referencia CONSTANT_Class */

    u2 fields_count;
    FieldInfo *fields;

    u2 methods_count;
    MethodInfo *methods;

    u2 attributes_count;
    AttributeInfo *attributes; /* atributos de nível de classe (crus) */
} ClassFile;

/* -----------------------------------------------------------
 * API pública do parser
 * ----------------------------------------------------------- */
typedef enum {
    CF_STATUS_OK = 0,
    CF_STATUS_ERR_BAD_MAGIC,
    CF_STATUS_ERR_OOB,               /* out-of-bounds: faltaram bytes */
    CF_STATUS_ERR_CP_TAG,            /* tag desconhecida/ilegal */
    CF_STATUS_ERR_CP_REF,            /* referência de CP inválida */
    CF_STATUS_ERR_UNSUPPORTED_VER,   /* versão de class não suportada pelo trabalho */
    CF_STATUS_ERR_ALLOC              /* falha de alocação */
} ClassFileStatus;

/* Lê o Buffer (bytes do .class) e preenche *out. Retorna ClassFileStatus. */
ClassFileStatus parse_classfile(ClassFile *out, Buffer *in);

/* Libera toda a memória alocada dentro de *cf (idempotente). */
void free_classfile(ClassFile *cf);

/* -----------------------------------------------------------
 * Helpers opcionais de consulta (facilitam impressão/resolve)
 * ----------------------------------------------------------- */
const char *cp_utf8(const CpInfo *cp, u2 cp_count, u2 idx);       /* "" se inválido */
const char *cp_nome_classe(const CpInfo *cp, u2 cp_count, u2 idx); /* Class->Utf8 ou "" */
void        cp_referencia_metodo(const CpInfo *cp, u2 cp_count, u2 idx,
                          const char **out_class,
                          const char **out_name,
                          const char **out_desc);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* CLASSFILE_H */
