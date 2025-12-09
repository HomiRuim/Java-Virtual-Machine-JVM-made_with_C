#include "attributes.h"
#include <string.h>

Status validate_code_bounds(const CodeAttribute *code) {
    if (!code) return ERR_BOUNDS;
    
    if (code->code_length == 0) return ERR_BOUNDS;
    
    if (!code->code) return ERR_BOUNDS;
    
    for (u2 i = 0; i < code->exception_table_length; i++) {
        ExceptionTableEntry *entry = &code->exception_table[i];
        
        if (entry->start_pc >= code->code_length) return ERR_BOUNDS;
        if (entry->end_pc > code->code_length) return ERR_BOUNDS;
        if (entry->handler_pc >= code->code_length) return ERR_BOUNDS;
        if (entry->start_pc >= entry->end_pc) return ERR_BOUNDS;
    }
    
    return OK;
}

Status find_code_attribute_by_name(const ClassFile *cf, const MethodInfo *method, 
                                   const char *target_name, AttributeInfo **out_attr) {
    if (!cf || !method || !target_name || !out_attr) return ERR_BOUNDS;
    
    for (u2 i = 0; i < method->attributes_count; i++) {
        const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count,
                                         method->attributes[i].attribute_name_index);
        
        if (attr_name && strcmp(attr_name, target_name) == 0) {
            *out_attr = &method->attributes[i];
            return OK;
        }
    }
    
    return ERR_BOUNDS;
}

int has_code_attribute(const ClassFile *cf, const MethodInfo *method) {
    if (!cf || !method) return 0;
    
    for (u2 i = 0; i < method->attributes_count; i++) {
        const char *attr_name = cp_utf8(cf->constant_pool, cf->constant_pool_count,
                                         method->attributes[i].attribute_name_index);
        
        if (attr_name && strcmp(attr_name, "Code") == 0) {
            return 1;
        }
    }
    
    return 0;
}
