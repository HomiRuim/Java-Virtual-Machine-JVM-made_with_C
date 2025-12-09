// src/disasm.c

#include "disasm.h"
#include "resolve.h"
#include "classfile.h" // Para acesso a ClassFile e CodeAttribute
#include "attributes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Para tratamento de floats/doubles (se necessário)


// --- STUBS DE IO  ---
static inline uint16_t io_read_u2_from_array(const uint8_t *code, uint32_t offset) {
    return (uint16_t)(((uint16_t)code[offset] << 8) | (uint16_t)code[offset + 1]);
}

static inline uint32_t io_read_u4_from_array(const uint8_t *code, uint32_t offset) {
    return ((uint32_t)code[offset] << 24) |
           ((uint32_t)code[offset + 1] << 16) |
           ((uint32_t)code[offset + 2] << 8)  |
            (uint32_t)code[offset + 3];
}


// --- TABELA DE INSTRUÇÕES (Opcode Map) ---

// Tipos de Argumento para determinar como ler os bytes seguintes
typedef enum {
    NO_ARGS = 0,    // 0 bytes de argumento (Ex: return, iadd)
    U1_ARG,         // 1 byte de argumento (Ex: ldc, iload)
    U2_ARG,         // 2 bytes de argumento (Ex: invokestatic, ldc_w)
    WIDE_ARG,       // Argumento especial (opcode wide)
    OFFSET_U2,      // Salto relativo de 2 bytes (Ex: ifeq, goto)
    OFFSET_U4,      // Salto relativo de 4 bytes (Ex: goto_w)
    TABLE_SWITCH,   // tableswitch (lógica de padding e tabela)
    LOOKUP_SWITCH,  // lookupswitch (lógica de padding e tabela)
} OpcodeArgType;

typedef struct {
    const char *mnemonic;
    OpcodeArgType arg_type;
    uint8_t fixed_length; // Comprimento total da instrucao (1 + argumentos), ou 0 se variavel.
} OpcodeInfo;

// Tabela de Opcodes (Inclui Sprints 1, 2 e 3)
// Tabela completa de opcodes JVM (bytecodes “clássicos")
// Observação: para 0xB9, 0xBA e 0xC5 ver nota acima.

static const OpcodeInfo opcode_table[256] = {
    // 0x00..0x0F  - const/nop
    [0x00] = {"nop", NO_ARGS, 1},
    [0x01] = {"aconst_null", NO_ARGS, 1},
    [0x02] = {"iconst_m1", NO_ARGS, 1},
    [0x03] = {"iconst_0", NO_ARGS, 1},
    [0x04] = {"iconst_1", NO_ARGS, 1},
    [0x05] = {"iconst_2", NO_ARGS, 1},
    [0x06] = {"iconst_3", NO_ARGS, 1},
    [0x07] = {"iconst_4", NO_ARGS, 1},
    [0x08] = {"iconst_5", NO_ARGS, 1},
    [0x09] = {"lconst_0", NO_ARGS, 1},
    [0x0A] = {"lconst_1", NO_ARGS, 1},
    [0x0B] = {"fconst_0", NO_ARGS, 1},
    [0x0C] = {"fconst_1", NO_ARGS, 1},
    [0x0D] = {"fconst_2", NO_ARGS, 1},
    [0x0E] = {"dconst_0", NO_ARGS, 1},
    [0x0F] = {"dconst_1", NO_ARGS, 1},

    // 0x10..0x19 - push e loads indexados
    [0x10] = {"bipush", U1_ARG, 2},
    [0x11] = {"sipush", U2_ARG, 3},
    [0x12] = {"ldc",    U1_ARG, 2},
    [0x13] = {"ldc_w",  U2_ARG, 3},
    [0x14] = {"ldc2_w", U2_ARG, 3},
    [0x15] = {"iload",  U1_ARG, 2},
    [0x16] = {"lload",  U1_ARG, 2},
    [0x17] = {"fload",  U1_ARG, 2},
    [0x18] = {"dload",  U1_ARG, 2},
    [0x19] = {"aload",  U1_ARG, 2},

    // 0x1A..0x29 - loads _0.._3
    [0x1A] = {"iload_0", NO_ARGS, 1},
    [0x1B] = {"iload_1", NO_ARGS, 1},
    [0x1C] = {"iload_2", NO_ARGS, 1},
    [0x1D] = {"iload_3", NO_ARGS, 1},
    [0x1E] = {"lload_0", NO_ARGS, 1},
    [0x1F] = {"lload_1", NO_ARGS, 1},
    [0x20] = {"lload_2", NO_ARGS, 1},
    [0x21] = {"lload_3", NO_ARGS, 1},
    [0x22] = {"fload_0", NO_ARGS, 1},
    [0x23] = {"fload_1", NO_ARGS, 1},
    [0x24] = {"fload_2", NO_ARGS, 1},
    [0x25] = {"fload_3", NO_ARGS, 1},
    [0x26] = {"dload_0", NO_ARGS, 1},
    [0x27] = {"dload_1", NO_ARGS, 1},
    [0x28] = {"dload_2", NO_ARGS, 1},
    [0x29] = {"dload_3", NO_ARGS, 1},

    // 0x2A..0x35 - aload_* e array loads
    [0x2A] = {"aload_0", NO_ARGS, 1},
    [0x2B] = {"aload_1", NO_ARGS, 1},
    [0x2C] = {"aload_2", NO_ARGS, 1},
    [0x2D] = {"aload_3", NO_ARGS, 1},
    [0x2E] = {"iaload",  NO_ARGS, 1},
    [0x2F] = {"laload",  NO_ARGS, 1},
    [0x30] = {"faload",  NO_ARGS, 1},
    [0x31] = {"daload",  NO_ARGS, 1},
    [0x32] = {"aaload",  NO_ARGS, 1},
    [0x33] = {"baload",  NO_ARGS, 1},
    [0x34] = {"caload",  NO_ARGS, 1},
    [0x35] = {"saload",  NO_ARGS, 1},

    // 0x36..0x3E - stores indexados e istore_*
    [0x36] = {"istore", U1_ARG, 2},
    [0x37] = {"lstore", U1_ARG, 2},
    [0x38] = {"fstore", U1_ARG, 2},
    [0x39] = {"dstore", U1_ARG, 2},
    [0x3A] = {"astore", U1_ARG, 2},
    [0x3B] = {"istore_0", NO_ARGS, 1},
    [0x3C] = {"istore_1", NO_ARGS, 1},
    [0x3D] = {"istore_2", NO_ARGS, 1},
    [0x3E] = {"istore_3", NO_ARGS, 1},

    // 0x3F..0x4E - stores _0.._3 restantes
    [0x3F] = {"lstore_0", NO_ARGS, 1},
    [0x40] = {"lstore_1", NO_ARGS, 1},
    [0x41] = {"lstore_2", NO_ARGS, 1},
    [0x42] = {"lstore_3", NO_ARGS, 1},
    [0x43] = {"fstore_0", NO_ARGS, 1},
    [0x44] = {"fstore_1", NO_ARGS, 1},
    [0x45] = {"fstore_2", NO_ARGS, 1},
    [0x46] = {"fstore_3", NO_ARGS, 1},
    [0x47] = {"dstore_0", NO_ARGS, 1},
    [0x48] = {"dstore_1", NO_ARGS, 1},
    [0x49] = {"dstore_2", NO_ARGS, 1},
    [0x4A] = {"dstore_3", NO_ARGS, 1},
    [0x4B] = {"astore_0", NO_ARGS, 1},
    [0x4C] = {"astore_1", NO_ARGS, 1},
    [0x4D] = {"astore_2", NO_ARGS, 1},
    [0x4E] = {"astore_3", NO_ARGS, 1},

    // 0x4F..0x56 - array stores
    [0x4F] = {"iastore", NO_ARGS, 1},
    [0x50] = {"lastore", NO_ARGS, 1},
    [0x51] = {"fastore", NO_ARGS, 1},
    [0x52] = {"dastore", NO_ARGS, 1},
    [0x53] = {"aastore", NO_ARGS, 1},
    [0x54] = {"bastore", NO_ARGS, 1},
    [0x55] = {"castore", NO_ARGS, 1},
    [0x56] = {"sastore", NO_ARGS, 1},

    // 0x57..0x5F - pilha
    [0x57] = {"pop", NO_ARGS, 1},
    [0x58] = {"pop2", NO_ARGS, 1},
    [0x59] = {"dup", NO_ARGS, 1},
    [0x5A] = {"dup_x1", NO_ARGS, 1},
    [0x5B] = {"dup_x2", NO_ARGS, 1},
    [0x5C] = {"dup2", NO_ARGS, 1},
    [0x5D] = {"dup2_x1", NO_ARGS, 1},
    [0x5E] = {"dup2_x2", NO_ARGS, 1},
    [0x5F] = {"swap", NO_ARGS, 1},

    // 0x60..0x83 - aritmética/bitwise
    [0x60] = {"iadd", NO_ARGS, 1},
    [0x61] = {"ladd", NO_ARGS, 1},
    [0x62] = {"fadd", NO_ARGS, 1},
    [0x63] = {"dadd", NO_ARGS, 1},
    [0x64] = {"isub", NO_ARGS, 1},
    [0x65] = {"lsub", NO_ARGS, 1},
    [0x66] = {"fsub", NO_ARGS, 1},
    [0x67] = {"dsub", NO_ARGS, 1},
    [0x68] = {"imul", NO_ARGS, 1},
    [0x69] = {"lmul", NO_ARGS, 1},
    [0x6A] = {"fmul", NO_ARGS, 1},
    [0x6B] = {"dmul", NO_ARGS, 1},
    [0x6C] = {"idiv", NO_ARGS, 1},
    [0x6D] = {"ldiv", NO_ARGS, 1},
    [0x6E] = {"fdiv", NO_ARGS, 1},
    [0x6F] = {"ddiv", NO_ARGS, 1},
    [0x70] = {"irem", NO_ARGS, 1},
    [0x71] = {"lrem", NO_ARGS, 1},
    [0x72] = {"frem", NO_ARGS, 1},
    [0x73] = {"drem", NO_ARGS, 1},
    [0x74] = {"ineg", NO_ARGS, 1},
    [0x75] = {"lneg", NO_ARGS, 1},
    [0x76] = {"fneg", NO_ARGS, 1},
    [0x77] = {"dneg", NO_ARGS, 1},
    [0x78] = {"ishl", NO_ARGS, 1},
    [0x79] = {"lshl", NO_ARGS, 1},
    [0x7A] = {"ishr", NO_ARGS, 1},
    [0x7B] = {"lshr", NO_ARGS, 1},
    [0x7C] = {"iushr", NO_ARGS, 1},
    [0x7D] = {"lushr", NO_ARGS, 1},
    [0x7E] = {"iand", NO_ARGS, 1},
    [0x7F] = {"land", NO_ARGS, 1},
    [0x80] = {"ior", NO_ARGS, 1},
    [0x81] = {"lor", NO_ARGS, 1},
    [0x82] = {"ixor", NO_ARGS, 1},
    [0x83] = {"lxor", NO_ARGS, 1},

    // 0x84..0x93 - iinc e conversões
    [0x84] = {"iinc", WIDE_ARG, 3}, // (u1 index, s1 const) — wide altera o formato
    [0x85] = {"i2l", NO_ARGS, 1},
    [0x86] = {"i2f", NO_ARGS, 1},
    [0x87] = {"i2d", NO_ARGS, 1},
    [0x88] = {"l2i", NO_ARGS, 1},
    [0x89] = {"l2f", NO_ARGS, 1},
    [0x8A] = {"l2d", NO_ARGS, 1},
    [0x8B] = {"f2i", NO_ARGS, 1},
    [0x8C] = {"f2l", NO_ARGS, 1},
    [0x8D] = {"f2d", NO_ARGS, 1},
    [0x8E] = {"d2i", NO_ARGS, 1},
    [0x8F] = {"d2l", NO_ARGS, 1},
    [0x90] = {"d2f", NO_ARGS, 1},
    [0x91] = {"i2b", NO_ARGS, 1},
    [0x92] = {"i2c", NO_ARGS, 1},
    [0x93] = {"i2s", NO_ARGS, 1},

    // 0x94..0x98 - comparações
    [0x94] = {"lcmp", NO_ARGS, 1},
    [0x95] = {"fcmpl", NO_ARGS, 1},
    [0x96] = {"fcmpg", NO_ARGS, 1},
    [0x97] = {"dcmpl", NO_ARGS, 1},
    [0x98] = {"dcmpg", NO_ARGS, 1},

    // 0x99..0xA7 - branches (u2 offset)
    [0x99] = {"ifeq", OFFSET_U2, 3},
    [0x9A] = {"ifne", OFFSET_U2, 3},
    [0x9B] = {"iflt", OFFSET_U2, 3},
    [0x9C] = {"ifge", OFFSET_U2, 3},
    [0x9D] = {"ifgt", OFFSET_U2, 3},
    [0x9E] = {"ifle", OFFSET_U2, 3},
    [0x9F] = {"if_icmpeq", OFFSET_U2, 3},
    [0xA0] = {"if_icmpne", OFFSET_U2, 3},
    [0xA1] = {"if_icmplt", OFFSET_U2, 3},
    [0xA2] = {"if_icmpge", OFFSET_U2, 3},
    [0xA3] = {"if_icmpgt", OFFSET_U2, 3},
    [0xA4] = {"if_icmple", OFFSET_U2, 3},
    [0xA5] = {"if_acmpeq", OFFSET_U2, 3},
    [0xA6] = {"if_acmpne", OFFSET_U2, 3},
    [0xA7] = {"goto", OFFSET_U2, 3},

    // 0xA8..0xAF - jsr/ret e returns tipados
    [0xA8] = {"jsr", OFFSET_U2, 3},
    [0xA9] = {"ret", U1_ARG, 2}, // wide altera tamanho
    [0xAA] = {"tableswitch", TABLE_SWITCH, 0},
    [0xAB] = {"lookupswitch", LOOKUP_SWITCH, 0},
    [0xAC] = {"ireturn", NO_ARGS, 1},
    [0xAD] = {"lreturn", NO_ARGS, 1},
    [0xAE] = {"freturn", NO_ARGS, 1},
    [0xAF] = {"dreturn", NO_ARGS, 1},

    // 0xB0..0xB8 - areturn e invocações/campos
    [0xB0] = {"areturn", NO_ARGS, 1},
    [0xB1] = {"return",  NO_ARGS, 1},
    [0xB2] = {"getstatic", U2_ARG, 3},
    [0xB3] = {"putstatic", U2_ARG, 3},
    [0xB4] = {"getfield",  U2_ARG, 3},
    [0xB5] = {"putfield",  U2_ARG, 3},
    [0xB6] = {"invokevirtual",  U2_ARG, 3},
    [0xB7] = {"invokespecial", U2_ARG, 3},
    [0xB8] = {"invokestatic",  U2_ARG, 3},

    // 0xB9..0xBF - invokeinterface/dynamic, objetos/arrays
    [0xB9] = {"invokeinterface", U2_ARG, 5}, // ver nota (u2 index + u1 count + u1 0)
    [0xBA] = {"invokedynamic",  U2_ARG, 5},  // ver nota (Java 7+)
    [0xBB] = {"new", U2_ARG, 3},
    [0xBC] = {"newarray", U1_ARG, 2},        // atype
    [0xBD] = {"anewarray", U2_ARG, 3},
    [0xBE] = {"arraylength", NO_ARGS, 1},
    [0xBF] = {"athrow", NO_ARGS, 1},

    // 0xC0..0xC9 - type ops, monitores, wide/jumps
    [0xC0] = {"checkcast", U2_ARG, 3},
    [0xC1] = {"instanceof", U2_ARG, 3},
    [0xC2] = {"monitorenter", NO_ARGS, 1},
    [0xC3] = {"monitorexit",  NO_ARGS, 1},
    [0xC4] = {"wide", WIDE_ARG, 0},          // muda formato de várias instruções
    [0xC5] = {"multianewarray", U2_ARG, 4},  // ver nota (u2 index + u1 dims)
    [0xC6] = {"ifnull", OFFSET_U2, 3},
    [0xC7] = {"ifnonnull", OFFSET_U2, 3},
    [0xC8] = {"goto_w", OFFSET_U4, 5},
    [0xC9] = {"jsr_w",  OFFSET_U4, 5},

    // 0xCA..0xFF - breakpoint, reservados e impdep
    [0xCA] = {"breakpoint", NO_ARGS, 1},
    [0xCB] = {"reserved", NO_ARGS, 1},
    [0xCC] = {"reserved", NO_ARGS, 1},
    [0xCD] = {"reserved", NO_ARGS, 1},
    [0xCE] = {"reserved", NO_ARGS, 1},
    [0xCF] = {"reserved", NO_ARGS, 1},
    [0xD0] = {"reserved", NO_ARGS, 1},
    [0xD1] = {"reserved", NO_ARGS, 1},
    [0xD2] = {"reserved", NO_ARGS, 1},
    [0xD3] = {"reserved", NO_ARGS, 1},
    [0xD4] = {"reserved", NO_ARGS, 1},
    [0xD5] = {"reserved", NO_ARGS, 1},
    [0xD6] = {"reserved", NO_ARGS, 1},
    [0xD7] = {"reserved", NO_ARGS, 1},
    [0xD8] = {"reserved", NO_ARGS, 1},
    [0xD9] = {"reserved", NO_ARGS, 1},
    [0xDA] = {"reserved", NO_ARGS, 1},
    [0xDB] = {"reserved", NO_ARGS, 1},
    [0xDC] = {"reserved", NO_ARGS, 1},
    [0xDD] = {"reserved", NO_ARGS, 1},
    [0xDE] = {"reserved", NO_ARGS, 1},
    [0xDF] = {"reserved", NO_ARGS, 1},
    [0xE0] = {"reserved", NO_ARGS, 1},
    [0xE1] = {"reserved", NO_ARGS, 1},
    [0xE2] = {"reserved", NO_ARGS, 1},
    [0xE3] = {"reserved", NO_ARGS, 1},
    [0xE4] = {"reserved", NO_ARGS, 1},
    [0xE5] = {"reserved", NO_ARGS, 1},
    [0xE6] = {"reserved", NO_ARGS, 1},
    [0xE7] = {"reserved", NO_ARGS, 1},
    [0xE8] = {"reserved", NO_ARGS, 1},
    [0xE9] = {"reserved", NO_ARGS, 1},
    [0xEA] = {"reserved", NO_ARGS, 1},
    [0xEB] = {"reserved", NO_ARGS, 1},
    [0xEC] = {"reserved", NO_ARGS, 1},
    [0xED] = {"reserved", NO_ARGS, 1},
    [0xEE] = {"reserved", NO_ARGS, 1},
    [0xEF] = {"reserved", NO_ARGS, 1},
    [0xF0] = {"reserved", NO_ARGS, 1},
    [0xF1] = {"reserved", NO_ARGS, 1},
    [0xF2] = {"reserved", NO_ARGS, 1},
    [0xF3] = {"reserved", NO_ARGS, 1},
    [0xF4] = {"reserved", NO_ARGS, 1},
    [0xF5] = {"reserved", NO_ARGS, 1},
    [0xF6] = {"reserved", NO_ARGS, 1},
    [0xF7] = {"reserved", NO_ARGS, 1},
    [0xF8] = {"reserved", NO_ARGS, 1},
    [0xF9] = {"reserved", NO_ARGS, 1},
    [0xFA] = {"reserved", NO_ARGS, 1},
    [0xFB] = {"reserved", NO_ARGS, 1},
    [0xFC] = {"reserved", NO_ARGS, 1},
    [0xFD] = {"reserved", NO_ARGS, 1},
    [0xFE] = {"impdep1", NO_ARGS, 1},
    [0xFF] = {"impdep2", NO_ARGS, 1},
};



/**
 * @brief Desmonta o Bytecode de um metodo.
 */
bool disassemble_method(ClassFile *cf, CodeAttribute *code_attr, DisasmOutput *output) {
    if (!code_attr || code_attr->code_length == 0) {
        output->instructions = NULL;
        output->count = 0;
        return true;
    }

    uint8_t *code = (uint8_t *)code_attr->code; // Array de bytes do Bytecode
    uint32_t code_len = code_attr->code_length;
    uint32_t pc = 0;
    uint32_t max_instructions = code_len; // Estimativa maxima
    
    // Aloca a lista de instrucoes
    output->instructions = (DisasmInstruction *)calloc(max_instructions, sizeof(DisasmInstruction));
    if (!output->instructions) return false;

    uint32_t inst_count = 0;

    // Loop de Desassembleamento
    while (pc < code_len && inst_count < max_instructions) {
        DisasmInstruction *current_inst = &output->instructions[inst_count];
        current_inst->pc = pc;
        uint8_t opcode = code[pc];
        
        const OpcodeInfo *info = &opcode_table[opcode];
        current_inst->mnemonic = strdup(info->mnemonic ? info->mnemonic : "unknown_opcode");
        
        uint32_t next_pc = pc + 1; // Proximo PC padrao
        char args_buffer[128] = {0};
        char *resolved_info = NULL;
        uint16_t arg_u2 = 0;
        uint8_t arg_u1 = 0;
        
        // --- LÓGICA DE TRATAMENTO DE ARGUMENTOS ---
        
        // Determina o comprimento e a leitura do argumento
        if (info->arg_type == NO_ARGS) {
            current_inst->length = 1;
        } else if (info->fixed_length > 0) {
            current_inst->length = info->fixed_length;
        } else {
            // Se o tamanho for variavel (SWITCHES, WIDE), ele sera ajustado abaixo
            current_inst->length = 1; 
        }

        switch (info->arg_type) {
            case NO_ARGS:
                break;
            
            case U1_ARG:
                arg_u1 = code[pc + 1];
                sprintf(args_buffer, "#%d", arg_u1);
                next_pc = pc + 2;
                current_inst->length = 2;
                // Opcodes ldc (0x12) usam U1_ARG
                if (opcode == 0x12) {
                    resolved_info = resolve_literal_to_string(cf, arg_u1);
                }
                break;
                
            case U2_ARG: {
                arg_u2 = io_read_u2_from_array(code, pc + 1);
                sprintf(args_buffer, "#%d", arg_u2);
                next_pc = pc + 3;
                current_inst->length = 3;

                if (opcode == 0x13) {               // ldc_w
                    resolved_info = resolve_literal_to_string(cf, arg_u2);
                } else if (opcode == 0x14) {        // ldc2_w (Long/Double)
                    resolved_info = resolve_literal_to_string(cf, arg_u2);
                } else if (opcode == 0xBB ||        // new
                        opcode == 0xBD ||        // anewarray
                        opcode == 0xC0 ||        // checkcast
                        opcode == 0xC1 ||        // instanceof
                        opcode == 0xC5) {        // multianewarray (u2 index + u1 dims)
                    // estes usam CONSTANT_Class
                    char *cls = resolve_class_name_to_string(cf, arg_u2);
                    resolved_info = cls ? cls : strdup("");
                } else if (opcode >= 0xB2 && opcode <= 0xB8) {
                    // get/putfield/static, invoke*
                    resolved_info = resolve_ref_to_string(cf, arg_u2);
                } else {
                    resolved_info = strdup(""); // neutro
                }
                break;
            }



            case OFFSET_U2: {
                int16_t offset = (int16_t)io_read_u2_from_array(code, pc + 1);
                int32_t target_pc = (int32_t)pc + 3 + offset;  // relativo à próxima instrução
                sprintf(args_buffer, "%d (to %d)", offset, target_pc);   // ← agora tudo decimal
                next_pc = pc + 3;
                current_inst->length = 3;
                break;
            }

            case OFFSET_U4: {
                int32_t offset = (int32_t)io_read_u4_from_array(code, pc + 1);
                int64_t target_pc = (int64_t)pc + 5 + offset;
                sprintf(args_buffer, "%d (to %d)", offset, (int32_t)target_pc);   // ← decimal
                next_pc = pc + 5;
                current_inst->length = 5;
                break;
            }


            
            case WIDE_ARG: { // iinc e o opcode wide em si
                if (opcode == 0x84) { // iinc <local_index> <const_val>
                    uint8_t index = code[pc + 1];
                    int8_t constant = (int8_t)code[pc + 2];
                    sprintf(args_buffer, "%d, %d", index, constant);
                    next_pc = pc + 3;
                    current_inst->length = 3;
                } else if (opcode == 0xC4) { // wide (muda o proximo opcode)
                    // Lógica para 'wide' é complexa, aqui tratamos apenas como um salto simples
                    sprintf(args_buffer, " (prefix)");
                    next_pc = pc + 1;
                    current_inst->length = 1;
                }
                break;
            }

            case TABLE_SWITCH: 
            case LOOKUP_SWITCH: { // Lógica complexa de Switch (Sprint 3)
                // 1. Calcular o PADDING para alinhar com 4 bytes
                uint32_t pad = (4 - ((pc + 1) % 4)) % 4; 
                uint32_t start_offset = pc + 1 + pad; // Onde default_offset comeca

                // 2. Leitura do default_offset (4 bytes)
                int32_t default_offset = (int32_t)io_read_u4_from_array(code, start_offset);
                uint32_t current_offset = start_offset + 4;
                
                // 3. Leitura dos parametros e calculo do tamanho total
                if (info->arg_type == TABLE_SWITCH) {
                    int32_t low = (int32_t)io_read_u4_from_array(code, current_offset);
                    int32_t high = (int32_t)io_read_u4_from_array(code, current_offset + 4);
                    uint32_t size = (high - low) + 1;
                    current_offset += 8 + (size * 4); // low + high + (size * offset)
                    
                    sprintf(args_buffer, " [default: %d, range: %d to %d]", default_offset, low, high);
                } else { // LOOKUP_SWITCH
                    int32_t npairs = (int32_t)io_read_u4_from_array(code, current_offset);
                    current_offset += 4 + (npairs * 8); // npairs + (count * 8 bytes)

                    sprintf(args_buffer, " [default: %d, npairs: %d]", default_offset, npairs);
                }
                
                current_inst->length = current_offset - pc; // Comprimento total
                next_pc = current_offset;
                break;
            }
            
            // ... (default e tratamento de opcodes desconhecidos) ...
        }

        // --- FINALIZAÇÃO E PRÓXIMA INSTRUÇÃO ---
        
        current_inst->args_str = strdup(args_buffer);
        current_inst->resolved_info = resolved_info ? resolved_info : strdup("");
        
        pc = next_pc; // Avanca para a proxima instrucao
        inst_count++;
    }

    output->count = inst_count;
    return true;
}
// Bloco 3: Funções de Limpeza (Se necessário)

/**
 * @brief Libera a memória alocada pela estrutura DisasmOutput.
 */
void free_disasm_output(DisasmOutput *output) {
    if (!output || !output->instructions) return;

    for (uint32_t i = 0; i < output->count; ++i) {
        free(output->instructions[i].mnemonic);
        free(output->instructions[i].args_str);
        free(output->instructions[i].resolved_info);
    }
    free(output->instructions);
    output->instructions = NULL;
    output->count = 0;
}
