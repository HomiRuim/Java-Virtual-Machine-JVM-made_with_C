#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include "classfile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} ExceptionTableEntry;

typedef struct {
    u2 start_pc;
    u2 line_number;
} LineNumberTableEntry;

typedef struct {
    u2 start_pc;
    u2 length;
    u2 name_index;
    u2 descriptor_index;
    u2 index;
} LocalVariableTableEntry;

typedef struct {
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1 *code;
    u2 exception_table_length;
    ExceptionTableEntry *exception_table;
    u2 attributes_count;
    AttributeInfo *attributes;
    
    u2 line_number_table_length;
    LineNumberTableEntry *line_number_table;
    
    u2 local_variable_table_length;
    LocalVariableTableEntry *local_variable_table;
} CodeAttribute;

Status parse_code_attribute(const ClassFile *cf, const AttributeInfo *attr, CodeAttribute *out);

void free_code_attribute(CodeAttribute *code_attr);

const CodeAttribute* find_code_attribute(const ClassFile *cf, const MethodInfo *method);

#ifdef __cplusplus
}
#endif

#endif
