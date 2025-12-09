// exec_flow.h

#ifndef EXEC_FLOW_H
#define EXEC_FLOW_H

#include "jvm_types.h"

// Protótipos das funções do Gerente de Fluxo (Pessoa 3)
void exec_invokestatic(uint16_t cp_index);
void exec_invokevirtual(uint16_t cp_index);
void exec_return_family(uint8_t opcode);
void exec_athrow(uint16_t thrown_exception_cp_index);

// Declaração externa para a variável global do Frame atual
extern StackFrame *current_frame;

#endif // EXEC_FLOW_H