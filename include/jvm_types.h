#ifndef JVM_TYPES_H
#define JVM_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t StackValue; 

// Declarações antecipadas
typedef struct s_stack_frame StackFrame;
typedef struct s_method_info MethodInfo;
typedef struct s_class_metadata ClassMetadata;
typedef struct s_object_ref ObjectRef; // Referência a objeto na Heap

#endif // JVM_TYPES_H