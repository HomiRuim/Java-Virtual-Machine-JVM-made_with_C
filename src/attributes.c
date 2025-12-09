#include "attributes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Status parse_exception_table(Buffer *buf, CodeAttribute *code) {
    Status status;
    
    status = read_u2(buf, &code->exception_table_length);
    if (status != OK) return status;
    
    if (code->exception_table_length == 0) {
        code->exception_table = NULL;
        return OK;
    }
    
    code->exception_table = malloc(sizeof(ExceptionTableEntry) * code->exception_table_length);
    if (!code->exception_table) return ERR_MEMORY;
    
    for (u2 i = 0; i < code->exception_table_length; i++) {
        ExceptionTableEntry *entry = &code->exception_table[i];
        
        if ((status = read_u2(buf, &entry->start_pc)) != OK) return status;
        if ((status = read_u2(buf, &entry->end_pc)) != OK) return status;
        if ((status = read_u2(buf, &entry->handler_pc)) != OK) return status;
        if ((status = read_u2(buf, &entry->catch_type)) != OK) return status;
    }
    
    return OK;
}

static Status parse_line_number_table(Buffer *buf, CodeAttribute *code) {
    Status status;
    
    status = read_u2(buf, &code->line_number_table_length);
    if (status != OK) return status;
    
    if (code->line_number_table_length == 0) {
        code->line_number_table = NULL;
        return OK;
    }
    
    code->line_number_table = malloc(sizeof(LineNumberTableEntry) * code->line_number_table_length);
    if (!code->line_number_table) return ERR_MEMORY;
    
    for (u2 i = 0; i < code->line_number_table_length; i++) {
        LineNumberTableEntry *entry = &code->line_number_table[i];
        
        if ((status = read_u2(buf, &entry->start_pc)) != OK) return status;
        if ((status = read_u2(buf, &entry->line_number)) != OK) return status;
    }
    
    return OK;
}

static Status parse_local_variable_table(Buffer *buf, CodeAttribute *code) {
    Status status;
    
    status = read_u2(buf, &code->local_variable_table_length);
    if (status != OK) return status;
    
    if (code->local_variable_table_length == 0) {
        code->local_variable_table = NULL;
        return OK;
    }
    
    code->local_variable_table = malloc(sizeof(LocalVariableTableEntry) * code->local_variable_table_length);
    if (!code->local_variable_table) return ERR_MEMORY;
    
    for (u2 i = 0; i < code->local_variable_table_length; i++) {
        LocalVariableTableEntry *entry = &code->local_variable_table[i];
        
        if ((status = read_u2(buf, &entry->start_pc)) != OK) return status;
        if ((status = read_u2(buf, &entry->length)) != OK) return status;
        if ((status = read_u2(buf, &entry->name_index)) != OK) return status;
        if ((status = read_u2(buf, &entry->descriptor_index)) != OK) return status;
        if ((status = read_u2(buf, &entry->index)) != OK) return status;
    }
    
    return OK;
}

static Status parse_code_attributes(const ClassFile *cf, Buffer *buf, CodeAttribute *code) {
    Status status;
    
    status = read_u2(buf, &code->attributes_count);
    if (status != OK) return status;
    
    if (code->attributes_count == 0) {
        code->attributes = NULL;
        return OK;
    }
    
    code->attributes = malloc(sizeof(AttributeInfo) * code->attributes_count);
    if (!code->attributes) return ERR_MEMORY;
    
    for (u2 i = 0; i < code->attributes_count; i++) {
        AttributeInfo *attr = &code->attributes[i];
        
        if ((status = read_u2(buf, &attr->attribute_name_index)) != OK) return status;
        if ((status = read_u4(buf, &attr->attribute_length)) != OK) return status;
        
        if (attr->attribute_length > 0) {
            if (buf->offset + attr->attribute_length > buf->size) {
                return ERR_BOUNDS;
            }
            
            attr->info = malloc(attr->attribute_length);
            if (!attr->info) return ERR_MEMORY;
            
            const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count, attr->attribute_name_index);
            
            if (strcmp(attr_name, "LineNumberTable") == 0) {
                Buffer attr_buf = {attr->info, attr->attribute_length, 0};
                status = read_bytes(buf, attr->info, attr->attribute_length);
                if (status != OK) return status;
                
                attr_buf.offset = 0;
                status = parse_line_number_table(&attr_buf, code);
                if (status != OK) {
                    fprintf(stderr, "Warning: Failed to parse LineNumberTable\n");
                }
            }
            else if (strcmp(attr_name, "LocalVariableTable") == 0) {
                Buffer attr_buf = {attr->info, attr->attribute_length, 0};
                status = read_bytes(buf, attr->info, attr->attribute_length);
                if (status != OK) return status;
                
                attr_buf.offset = 0;
                status = parse_local_variable_table(&attr_buf, code);
                if (status != OK) {
                    fprintf(stderr, "Warning: Failed to parse LocalVariableTable\n");
                }
            }
            else {
                status = read_bytes(buf, attr->info, attr->attribute_length);
                if (status != OK) return status;
                
                fprintf(stderr, "Warning: Skipping unsupported Code sub-attribute: %s\n", attr_name);
            }
        } else {
            attr->info = NULL;
        }
    }
    
    return OK;
}

Status parse_code_attribute(const ClassFile *cf, const AttributeInfo *attr, CodeAttribute *out) {
    if (!cf || !attr || !out) return ERR_BOUNDS;
    
    memset(out, 0, sizeof(CodeAttribute));
    
    if (attr->attribute_length < 12) {
        return ERR_BOUNDS;
    }
    
    Buffer buf = {attr->info, attr->attribute_length, 0};
    Status status;
    
    status = read_u2(&buf, &out->max_stack);
    if (status != OK) return status;
    
    status = read_u2(&buf, &out->max_locals);
    if (status != OK) return status;
    
    status = read_u4(&buf, &out->code_length);
    if (status != OK) return status;
    
    if (out->code_length == 0 || out->code_length > 65535) {
        return ERR_BOUNDS;
    }
    
    if (buf.offset + out->code_length > buf.size) {
        return ERR_BOUNDS;
    }
    
    out->code = malloc(out->code_length);
    if (!out->code) return ERR_MEMORY;
    
    status = read_bytes(&buf, out->code, out->code_length);
    if (status != OK) {
        free(out->code);
        return status;
    }
    
    status = parse_exception_table(&buf, out);
    if (status != OK) {
        free(out->code);
        return status;
    }
    
    status = parse_code_attributes(cf, &buf, out);
    if (status != OK) {
        free(out->code);
        if (out->exception_table) free(out->exception_table);
        return status;
    }
    
    return OK;
}

void free_code_attribute(CodeAttribute *code_attr) {
    if (!code_attr) return;
    
    if (code_attr->code) {
        free(code_attr->code);
        code_attr->code = NULL;
    }
    
    if (code_attr->exception_table) {
        free(code_attr->exception_table);
        code_attr->exception_table = NULL;
    }
    
    if (code_attr->line_number_table) {
        free(code_attr->line_number_table);
        code_attr->line_number_table = NULL;
    }
    
    if (code_attr->local_variable_table) {
        free(code_attr->local_variable_table);
        code_attr->local_variable_table = NULL;
    }
    
    if (code_attr->attributes) {
        for (u2 i = 0; i < code_attr->attributes_count; i++) {
            if (code_attr->attributes[i].info) {
                free(code_attr->attributes[i].info);
            }
        }
        free(code_attr->attributes);
        code_attr->attributes = NULL;
    }
}

const CodeAttribute* find_code_attribute(const ClassFile *cf, const MethodInfo *method) {
    if (!cf || !method) return NULL;
    
    for (u2 i = 0; i < method->attributes_count; i++) {
        const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count, 
                                         method->attributes[i].attribute_name_index);
        
        if (strcmp(attr_name, "Code") == 0) {
            static CodeAttribute code_attr;
            memset(&code_attr, 0, sizeof(CodeAttribute));
            
            Status status = parse_code_attribute(cf, &method->attributes[i], &code_attr);
            if (status == OK) {
                return &code_attr;
            } else {
                free_code_attribute(&code_attr);
                return NULL;
            }
        }
    }
    
    return NULL;
}
