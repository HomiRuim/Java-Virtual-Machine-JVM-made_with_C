#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "attributes.h"
#include "resolve.h"
#include "heap_manager.h"

// Declaração da função do classfile.c
extern const char *cp_utf8(const CpInfo *cp, u2 cp_count, u2 idx);

/*
 * ====================================================================
 * HELPER: Busca de Método por Nome e Descritor
 * ====================================================================
 */

/**
 * @brief Busca um método no ClassFile pelo nome e descritor.
 */
static MethodInfo* find_method(ClassFile *class_file, const char *name, const char *descriptor) {
    for (u2 i = 0; i < class_file->methods_count; i++) {
        MethodInfo *method = &class_file->methods[i];
        
        // Obtém nome e descritor do método
        const char *method_name = cp_utf8(class_file->constant_pool, 
                                         class_file->constant_pool_count, 
                                         method->name_index);
        const char *method_desc = cp_utf8(class_file->constant_pool, 
                                         class_file->constant_pool_count, 
                                         method->descriptor_index);
        
        if (method_name && method_desc && 
            strcmp(method_name, name) == 0 && 
            strcmp(method_desc, descriptor) == 0) {
            return method;
        }
    }
    return NULL;
}

/*
 * ====================================================================
 * IMPLEMENTAÇÃO DOS MANIPULADORES DE OPCODE (DISPATCH TABLE)
 * ====================================================================
 * Cada manipulador:
 * - Lê operandos do bytecode
 * - Executa a operação
 * - Atualiza PC
 * - Retorna: 0 (sucesso), 1 (return), negativo (erro)
 */

// Manipulador padrão para opcodes não implementados
static int handle_unimplemented(Frame *frame, const CliOptions *options) {
    u1 opcode = *frame->pc;
    fprintf(stderr, "Erro: Opcode 0x%02X não implementado.\n", opcode);
    return -1;
}

// 0x00: NOP - Não faz nada
static int handle_nop(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] NOP\n");
    }
    frame->pc += 1;
    return 0;
}

// 0x01: ACONST_NULL - Empilha null
static int handle_aconst_null(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] ACONST_NULL\n");
    }
    *frame->stack_top = 0;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x02-0x08: ICONST_<i> - Empilha constante inteira
static int handle_iconst_m1(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_M1\n");
    *frame->stack_top = (Slot)(-1);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_0(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_0\n");
    *frame->stack_top = 0;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_1(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_1\n");
    *frame->stack_top = 1;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_2(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_2\n");
    *frame->stack_top = 2;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_3(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_3\n");
    *frame->stack_top = 3;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_4(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_4\n");
    *frame->stack_top = 4;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iconst_5(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ICONST_5\n");
    *frame->stack_top = 5;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x10: BIPUSH - Empilha byte com sinal
static int handle_bipush(Frame *frame, const CliOptions *options) {
    int8_t value = (int8_t)(*(frame->pc + 1));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] BIPUSH %d\n", value);
    }
    *frame->stack_top = (Slot)value;
    frame->stack_top++;
    frame->pc += 2;
    return 0;
}

// 0x11: SIPUSH - Empilha short com sinal
static int handle_sipush(Frame *frame, const CliOptions *options) {
    int16_t value = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] SIPUSH %d\n", value);
    }
    *frame->stack_top = (Slot)value;
    frame->stack_top++;
    frame->pc += 3;
    return 0;
}

// 0x12: LDC - Empilha constante do pool (simplificado)
static int handle_ldc(Frame *frame, const CliOptions *options) {
    u1 index = *(frame->pc + 1);
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] LDC #%d (stub - empilha 0)\n", index);
    }
    // Simplificação: empilha 0
    *frame->stack_top = 0;
    frame->stack_top++;
    frame->pc += 2;
    return 0;
}

// 0x15: ILOAD - Carrega int de variável local (com índice)
static int handle_iload(Frame *frame, const CliOptions *options) {
    u1 index = *(frame->pc + 1);
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] ILOAD %d\n", index);
    }
    *frame->stack_top = frame->local_vars[index];
    frame->stack_top++;
    frame->pc += 2;
    return 0;
}

// 0x1A-0x1D: ILOAD_<n> - Carrega int de variável local
static int handle_iload_0(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ILOAD_0\n");
    *frame->stack_top = frame->local_vars[0];
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iload_1(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ILOAD_1\n");
    *frame->stack_top = frame->local_vars[1];
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iload_2(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ILOAD_2\n");
    *frame->stack_top = frame->local_vars[2];
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

static int handle_iload_3(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ILOAD_3\n");
    *frame->stack_top = frame->local_vars[3];
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x36: ISTORE - Armazena int em variável local (com índice)
static int handle_istore(Frame *frame, const CliOptions *options) {
    u1 index = *(frame->pc + 1);
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] ISTORE %d\n", index);
    }
    frame->stack_top--;
    frame->local_vars[index] = *frame->stack_top;
    frame->pc += 2;
    return 0;
}

// 0x3B-0x3E: ISTORE_<n> - Armazena int em variável local
static int handle_istore_0(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ISTORE_0\n");
    frame->stack_top--;
    frame->local_vars[0] = *frame->stack_top;
    frame->pc += 1;
    return 0;
}

static int handle_istore_1(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ISTORE_1\n");
    frame->stack_top--;
    frame->local_vars[1] = *frame->stack_top;
    frame->pc += 1;
    return 0;
}

static int handle_istore_2(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ISTORE_2\n");
    frame->stack_top--;
    frame->local_vars[2] = *frame->stack_top;
    frame->pc += 1;
    return 0;
}

static int handle_istore_3(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ISTORE_3\n");
    frame->stack_top--;
    frame->local_vars[3] = *frame->stack_top;
    frame->pc += 1;
    return 0;
}

// 0x57: POP - Remove topo da pilha
static int handle_pop(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] POP\n");
    frame->stack_top--;
    frame->pc += 1;
    return 0;
}

// 0x59: DUP - Duplica topo da pilha
static int handle_dup(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] DUP\n");
    Slot value = *(frame->stack_top - 1);
    *frame->stack_top = value;
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x60: IADD - Soma dois ints
static int handle_iadd(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] IADD\n");
    frame->stack_top--;
    Slot value2 = *frame->stack_top;
    frame->stack_top--;
    Slot value1 = *frame->stack_top;
    *frame->stack_top = (Slot)((int32_t)value1 + (int32_t)value2);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x64: ISUB - Subtrai dois ints
static int handle_isub(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] ISUB\n");
    frame->stack_top--;
    Slot value2 = *frame->stack_top;
    frame->stack_top--;
    Slot value1 = *frame->stack_top;
    *frame->stack_top = (Slot)((int32_t)value1 - (int32_t)value2);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x68: IMUL - Multiplica dois ints
static int handle_imul(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] IMUL\n");
    frame->stack_top--;
    Slot value2 = *frame->stack_top;
    frame->stack_top--;
    Slot value1 = *frame->stack_top;
    *frame->stack_top = (Slot)((int32_t)value1 * (int32_t)value2);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x6C: IDIV - Divide dois ints
static int handle_idiv(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] IDIV\n");
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    
    if (value2 == 0) {
        fprintf(stderr, "Erro: Divisão por zero!\n");
        return -1;
    }
    
    *frame->stack_top = (Slot)(value1 / value2);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x70: IREM - Resto da divisão
static int handle_irem(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] IREM\n");
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    
    if (value2 == 0) {
        fprintf(stderr, "Erro: Divisão por zero!\n");
        return -1;
    }
    
    *frame->stack_top = (Slot)(value1 % value2);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x74: INEG - Negação
static int handle_ineg(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) printf("[DEBUG] INEG\n");
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    *frame->stack_top = (Slot)(-value);
    frame->stack_top++;
    frame->pc += 1;
    return 0;
}

// 0x84: IINC - Incrementa variável local
static int handle_iinc(Frame *frame, const CliOptions *options) {
    u1 index = *(frame->pc + 1);
    int8_t const_val = (int8_t)(*(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IINC %d %d\n", index, const_val);
    }
    frame->local_vars[index] = (Slot)((int32_t)frame->local_vars[index] + const_val);
    frame->pc += 3;
    return 0;
}

// 0x99-0x9E: Comparações com zero (IF<cond>)
static int handle_ifeq(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFEQ (value=%d, offset=%d)\n", value, offset);
    }
    if (value == 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_ifne(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFNE (value=%d, offset=%d)\n", value, offset);
    }
    if (value != 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_iflt(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFLT (value=%d, offset=%d)\n", value, offset);
    }
    if (value < 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_ifge(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFGE (value=%d, offset=%d)\n", value, offset);
    }
    if (value >= 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_ifgt(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFGT (value=%d, offset=%d)\n", value, offset);
    }
    if (value > 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_ifle(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IFLE (value=%d, offset=%d)\n", value, offset);
    }
    if (value <= 0) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

// 0x9F-0xA4: Comparações entre dois valores (IF_ICMP<cond>)
static int handle_if_icmpeq(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPEQ (%d == %d)\n", value1, value2);
    }
    if (value1 == value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_if_icmpne(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPNE (%d != %d)\n", value1, value2);
    }
    if (value1 != value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_if_icmplt(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPLT (%d < %d)\n", value1, value2);
    }
    if (value1 < value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_if_icmpge(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPGE (%d >= %d)\n", value1, value2);
    }
    if (value1 >= value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_if_icmpgt(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPGT (%d > %d)\n", value1, value2);
    }
    if (value1 > value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

static int handle_if_icmple(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    frame->stack_top--;
    int32_t value2 = (int32_t)*frame->stack_top;
    frame->stack_top--;
    int32_t value1 = (int32_t)*frame->stack_top;
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] IF_ICMPLE (%d <= %d)\n", value1, value2);
    }
    if (value1 <= value2) {
        frame->pc += offset;
    } else {
        frame->pc += 3;
    }
    return 0;
}

// 0xA7: GOTO - Salto incondicional
static int handle_goto(Frame *frame, const CliOptions *options) {
    int16_t offset = (int16_t)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] GOTO offset=%d\n", offset);
    }
    frame->pc += offset;
    return 0;
}

// 0xAC: IRETURN - Retorna int
static int handle_ireturn(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) {
        frame->stack_top--;
        int32_t return_value = (int32_t)*frame->stack_top;
        printf("[DEBUG] IRETURN %d\n", return_value);
    }
    return 1; // Sinaliza return
}

// 0xB1: RETURN - Retorna void
static int handle_return(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] RETURN\n");
    }
    return 1; // Sinaliza return
}

// 0xB2: GETSTATIC - Obtém campo estático (simplificado)
static int handle_getstatic(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] GETSTATIC #%d (stub)\n", index);
    }
    // Simplificação: empilha 0
    *frame->stack_top = 0;
    frame->stack_top++;
    frame->pc += 3;
    return 0;
}

// 0xB3: PUTSTATIC - Define campo estático (simplificado)
static int handle_putstatic(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] PUTSTATIC #%d (stub)\n", index);
    }
    // Simplificação: remove topo da pilha
    frame->stack_top--;
    frame->pc += 3;
    return 0;
}

// 0xB6: INVOKEVIRTUAL - Invoca método de instância (simplificado)
static int handle_invokevirtual(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] INVOKEVIRTUAL #%d (stub - método não executado)\n", index);
    }
    // Simplificação: remove referência do objeto
    frame->stack_top--;
    frame->pc += 3;
    return 0;
}

// 0xB7: INVOKESPECIAL - Invoca método de inicialização (simplificado)
static int handle_invokespecial(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] INVOKESPECIAL #%d (stub - construtor/método não executado)\n", index);
    }
    // Simplificação: remove referência do objeto
    frame->stack_top--;
    frame->pc += 3;
    return 0;
}

// 0xB8: INVOKESTATIC - Invoca método estático (simplificado)
static int handle_invokestatic(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] INVOKESTATIC #%d (stub - método não executado)\n", index);
    }
    // Simplificação: não executa o método
    frame->pc += 3;
    return 0;
}

// 0xAA: TABLESWITCH - Switch com tabela de saltos
static int handle_tableswitch(Frame *frame, const CliOptions *options) {
    u1 *start_pc = frame->pc;
    frame->pc++; // Pula o opcode
    
    // Alinhamento: padding para múltiplo de 4
    while (((frame->pc - start_pc) % 4) != 0) {
        frame->pc++;
    }
    
    // Lê default offset (4 bytes)
    int32_t default_offset = (int32_t)(
        (*(frame->pc) << 24) | 
        (*(frame->pc + 1) << 16) | 
        (*(frame->pc + 2) << 8) | 
        *(frame->pc + 3)
    );
    frame->pc += 4;
    
    // Lê low (4 bytes)
    int32_t low = (int32_t)(
        (*(frame->pc) << 24) | 
        (*(frame->pc + 1) << 16) | 
        (*(frame->pc + 2) << 8) | 
        *(frame->pc + 3)
    );
    frame->pc += 4;
    
    // Lê high (4 bytes)
    int32_t high = (int32_t)(
        (*(frame->pc) << 24) | 
        (*(frame->pc + 1) << 16) | 
        (*(frame->pc + 2) << 8) | 
        *(frame->pc + 3)
    );
    frame->pc += 4;
    
    // Desempilha o índice
    frame->stack_top--;
    int32_t index = (int32_t)*frame->stack_top;
    
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] TABLESWITCH index=%d, low=%d, high=%d, default=%d\n", 
               index, low, high, default_offset);
    }
    
    // Verifica se o índice está no range
    if (index < low || index > high) {
        // Usa o offset default
        frame->pc = start_pc + default_offset;
    } else {
        // Calcula a posição na tabela
        int32_t table_index = index - low;
        u1 *offset_ptr = frame->pc + (table_index * 4);
        
        // Lê o offset da tabela
        int32_t offset = (int32_t)(
            (*offset_ptr << 24) | 
            (*(offset_ptr + 1) << 16) | 
            (*(offset_ptr + 2) << 8) | 
            *(offset_ptr + 3)
        );
        
        // Salta para o offset
        frame->pc = start_pc + offset;
    }
    
    return 0;
}

// 0xBB: NEW - Cria novo objeto
static int handle_new(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] NEW #%d\n", index);
    }
    
    // Simplificação: cria objeto com 10 campos (ajustar conforme necessário)
    ObjectRef obj = jvm_heap_new_object(frame->class_file, 10);
    
    // Empilha a referência do objeto
    *frame->stack_top = (StackValue)(uintptr_t)obj;
    frame->stack_top++;
    
    frame->pc += 3;
    return 0;
}

// 0xBC: NEWARRAY - Cria novo array primitivo
static int handle_newarray(Frame *frame, const CliOptions *options) {
    u1 atype = *(frame->pc + 1);
    
    // Desempilha o tamanho
    frame->stack_top--;
    int32_t count = (int32_t)*frame->stack_top;
    
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] NEWARRAY type=%d, count=%d\n", atype, count);
    }
    
    if (count < 0) {
        fprintf(stderr, "Erro: Tamanho de array negativo\n");
        return -1;
    }
    
    // Cria o array
    ObjectRef array = jvm_heap_new_array(atype, (u4)count);
    
    // Empilha a referência do array
    *frame->stack_top = (StackValue)(uintptr_t)array;
    frame->stack_top++;
    
    frame->pc += 2;
    return 0;
}

// 0xB4: GETFIELD - Obtém campo de objeto
static int handle_getfield(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    
    // Desempilha a referência do objeto
    frame->stack_top--;
    ObjectRef obj = (ObjectRef)(uintptr_t)*frame->stack_top;
    
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] GETFIELD #%d\n", index);
    }
    
    if (!obj) {
        fprintf(stderr, "Erro: NullPointerException em GETFIELD\n");
        return -1;
    }
    
    // Simplificação: usa index % 10 como offset
    u4 offset = index % 10;
    StackValue value = jvm_heap_getfield(obj, offset);
    
    // Empilha o valor
    *frame->stack_top = value;
    frame->stack_top++;
    
    frame->pc += 3;
    return 0;
}

// 0xB5: PUTFIELD - Define campo de objeto
static int handle_putfield(Frame *frame, const CliOptions *options) {
    u2 index = (u2)((*(frame->pc + 1) << 8) | *(frame->pc + 2));
    
    // Desempilha o valor
    frame->stack_top--;
    StackValue value = *frame->stack_top;
    
    // Desempilha a referência do objeto
    frame->stack_top--;
    ObjectRef obj = (ObjectRef)(uintptr_t)*frame->stack_top;
    
    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] PUTFIELD #%d\n", index);
    }
    
    if (!obj) {
        fprintf(stderr, "Erro: NullPointerException em PUTFIELD\n");
        return -1;
    }
    
    // Simplificação: usa index % 10 como offset
    u4 offset = index % 10;
    jvm_heap_putfield(obj, offset, value);
    
    frame->pc += 3;
    return 0;
}

// 0xB0: ARETURN - Retorna referência de objeto
static int handle_areturn(Frame *frame, const CliOptions *options) {
    if (options->execution_mode == MODE_DEBUG) {
        frame->stack_top--;
        ObjectRef return_value = (ObjectRef)(uintptr_t)*frame->stack_top;
        printf("[DEBUG] ARETURN (object reference: %p)\n", (void*)return_value);
    }
    return 1; // Sinaliza return
}

/*
 * ====================================================================
 * DISPATCH TABLE - Tabela de Ponteiros de Função (256 opcodes)
 * ====================================================================
 * Esta abordagem substitui grandes estruturas switch para melhor
 * performance e modularidade.
 */

OpcodeHandler opcode_handlers[256];

/**
 * @brief Inicializa a Dispatch Table com todos os manipuladores.
 * 
 * Mapeia cada opcode (0x00 a 0xFF) para sua função correspondente.
 */
void init_opcode_handlers() {
    // 1. Inicializa todos com o manipulador não implementado
    for (int i = 0; i < 256; i++) {
        opcode_handlers[i] = handle_unimplemented;
    }

    // 2. Atribui manipuladores implementados
    opcode_handlers[0x00] = handle_nop;
    opcode_handlers[0x01] = handle_aconst_null;
    opcode_handlers[0x02] = handle_iconst_m1;
    opcode_handlers[0x03] = handle_iconst_0;
    opcode_handlers[0x04] = handle_iconst_1;
    opcode_handlers[0x05] = handle_iconst_2;
    opcode_handlers[0x06] = handle_iconst_3;
    opcode_handlers[0x07] = handle_iconst_4;
    opcode_handlers[0x08] = handle_iconst_5;
    opcode_handlers[0x10] = handle_bipush;
    opcode_handlers[0x11] = handle_sipush;
    opcode_handlers[0x12] = handle_ldc;
    opcode_handlers[0x15] = handle_iload;
    opcode_handlers[0x1A] = handle_iload_0;
    opcode_handlers[0x1B] = handle_iload_1;
    opcode_handlers[0x1C] = handle_iload_2;
    opcode_handlers[0x1D] = handle_iload_3;
    opcode_handlers[0x36] = handle_istore;
    opcode_handlers[0x3B] = handle_istore_0;
    opcode_handlers[0x3C] = handle_istore_1;
    opcode_handlers[0x3D] = handle_istore_2;
    opcode_handlers[0x3E] = handle_istore_3;
    opcode_handlers[0x57] = handle_pop;
    opcode_handlers[0x59] = handle_dup;
    opcode_handlers[0x60] = handle_iadd;
    opcode_handlers[0x64] = handle_isub;
    opcode_handlers[0x68] = handle_imul;
    opcode_handlers[0x6C] = handle_idiv;
    opcode_handlers[0x70] = handle_irem;
    opcode_handlers[0x74] = handle_ineg;
    opcode_handlers[0x84] = handle_iinc;
    opcode_handlers[0x99] = handle_ifeq;
    opcode_handlers[0x9A] = handle_ifne;
    opcode_handlers[0x9B] = handle_iflt;
    opcode_handlers[0x9C] = handle_ifge;
    opcode_handlers[0x9D] = handle_ifgt;
    opcode_handlers[0x9E] = handle_ifle;
    opcode_handlers[0x9F] = handle_if_icmpeq;
    opcode_handlers[0xA0] = handle_if_icmpne;
    opcode_handlers[0xA1] = handle_if_icmplt;
    opcode_handlers[0xA2] = handle_if_icmpge;
    opcode_handlers[0xA3] = handle_if_icmpgt;
    opcode_handlers[0xA4] = handle_if_icmple;
    opcode_handlers[0xA7] = handle_goto;
    opcode_handlers[0xAA] = handle_tableswitch;
    opcode_handlers[0xAC] = handle_ireturn;
    opcode_handlers[0xB0] = handle_areturn;
    opcode_handlers[0xB1] = handle_return;
    opcode_handlers[0xB2] = handle_getstatic;
    opcode_handlers[0xB3] = handle_putstatic;
    opcode_handlers[0xB4] = handle_getfield;
    opcode_handlers[0xB5] = handle_putfield;
    opcode_handlers[0xB6] = handle_invokevirtual;
    opcode_handlers[0xB7] = handle_invokespecial;
    opcode_handlers[0xB8] = handle_invokestatic;
    opcode_handlers[0xBB] = handle_new;
    opcode_handlers[0xBC] = handle_newarray;
}

/*
 * ====================================================================
 * FUNÇÃO PRINCIPAL DE EXECUÇÃO
 * ====================================================================
 */

/**
 * @brief Executa o método main da classe carregada.
 * 
 * Esta função implementa o interpretador principal da JVM:
 * 1. Busca o método main
 * 2. Obtém o Code Attribute
 * 3. Cria o Frame de execução
 * 4. Loop de execução usando a Dispatch Table
 * 5. Limpeza de memória
 */
int execute_main_method(ClassFile *class_file, const CliOptions *options) {
    if (!class_file || !options) {
        fprintf(stderr, "Erro: Parâmetros inválidos para execute_main_method.\n");
        return 1;
    }

    if (options->execution_mode == MODE_DEBUG) {
        printf("\n[DEBUG] ========== INICIANDO EXECUÇÃO ==========\n");
    }

    // 1. Buscar o método main (public static void main(String[]))
    MethodInfo *main_method = find_method(class_file, "main", "([Ljava/lang/String;)V");
    
    if (!main_method) {
        fprintf(stderr, "Erro: Método 'main' não encontrado.\n");
        fprintf(stderr, "Esperado: public static void main(String[])\n");
        return 1;
    }

    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] Método 'main' encontrado.\n");
    }

    // 2. Obter o Code Attribute do método
    const CodeAttribute *code_attr = find_code_attribute(class_file, main_method);
    if (!code_attr) {
        fprintf(stderr, "Erro: Code Attribute não encontrado para o método main.\n");
        return 1;
    }

    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] Code Attribute: max_stack=%d, max_locals=%d, code_length=%d\n",
               code_attr->max_stack, code_attr->max_locals, code_attr->code_length);
    }

    // 3. Criar o Frame de Execução
    Frame *frame = frame_new(class_file, main_method, 
                            code_attr->max_locals, code_attr->max_stack);
    if (!frame) {
        fprintf(stderr, "Erro: Falha ao criar o Frame de Execução.\n");
        free_code_attribute((CodeAttribute*)code_attr);
        return 1;
    }

    // 4. Inicializar o Program Counter
    frame->pc = code_attr->code;
    u1 *code_end = code_attr->code + code_attr->code_length;

    // 5. Inicializar a Dispatch Table
    init_opcode_handlers();

    if (options->execution_mode == MODE_DEBUG) {
        printf("[DEBUG] Dispatch Table inicializada com %d opcodes.\n", 256);
        printf("[DEBUG] Iniciando loop de execução...\n\n");
    }

    // 6. Loop de Execução Principal
    int status = 0;
    int instruction_count = 0;
    
    while (status == 0 && frame->pc < code_end) {
        // Lê o opcode atual
        u1 opcode = *frame->pc;
        
        if (options->execution_mode == MODE_DEBUG) {
            printf("[DEBUG] [PC=%ld] Opcode: 0x%02X | Stack depth: %ld\n",
                   (long)(frame->pc - code_attr->code),
                   opcode,
                   (long)(frame->stack_top - frame->operand_stack));
        }

        // Obtém o handler da Dispatch Table
        OpcodeHandler handler = opcode_handlers[opcode];
        
        // Executa o handler
        status = handler(frame, options);
        
        instruction_count++;
        
        // Proteção contra loops infinitos em modo debug
        if (options->execution_mode == MODE_DEBUG && instruction_count > 100000) {
            fprintf(stderr, "\n[DEBUG] AVISO: Executadas mais de 100.000 instruções. Possível loop infinito.\n");
            break;
        }
    }

    // 7. Verificação do resultado
    if (status < 0) {
        fprintf(stderr, "\nErro: Execução falhou com código %d.\n", status);
    } else if (options->execution_mode == MODE_DEBUG) {
        printf("\n[DEBUG] ========== EXECUÇÃO CONCLUÍDA ==========\n");
        printf("[DEBUG] Total de instruções executadas: %d\n", instruction_count);
        printf("[DEBUG] Status final: %s\n", status == 1 ? "RETURN" : "FIM DO CÓDIGO");
    } else if (options->execution_mode == MODE_EXECUTE) {
        printf("\nExecução concluída com sucesso.\n");
    }

    // 8. Limpeza
    frame_free(frame);
    free_code_attribute((CodeAttribute*)code_attr);

    return (status >= 0) ? 0 : 1;
}
