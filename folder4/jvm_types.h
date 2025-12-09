#ifndef JVM_TYPES_H
#define JVM_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define T_INT 5     // <-- ESTA LINHA É ESSENCIAL
#define T_OBJECT 9

typedef uint64_t StackValue; 

// Declarações antecipadas
typedef struct s_stack_frame StackFrame;
typedef struct s_method_info MethodInfo;
typedef struct s_class_metadata ClassMetadata;
//typedef struct s_object_ref ObjectRef; // Referência a objeto na Heap
struct s_object_ref; // Declaração antecipada da struct que representa um objeto
typedef struct s_object_ref Object;

#endif // JVM_TYPES_H