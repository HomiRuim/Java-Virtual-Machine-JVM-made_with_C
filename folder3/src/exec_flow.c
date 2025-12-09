// vm_core/exec_flow.c

#include <stdio.h>
#include <stdlib.h>
#include "../include/memory_manager.h"
#include "../include/jvm_types.h"
#include "../include/class_manager.h"
#include "../include/exec_flow.h"

// Variáveis globais (acessadas pela Pessoa 2)
// extern StackFrame *current_frame; 
StackFrame *current_frame = NULL;

// -----------------------------------------------------------------------------
// 1. INVOKESTATIC: Chamada Simples (Métodos de Classe)
// -----------------------------------------------------------------------------
void exec_invokestatic(uint16_t cp_index) {
    // 1. Resolver o método (Assume que Pessoa 4 fornece as strings)
    const char *class_name = "TargetClass";
    const char *method_sig = "targetMethod()V";
    
    MethodInfo *target_method = class_manager_find_static_method(class_name, method_sig);
    if (!target_method) return; // Lançar Exceção (NoSuchMethodError)

    // 2. Preparar e Criar o Novo Frame
    size_t arg_count = get_argument_count(method_sig);
    StackFrame *new_frame = jvm_stack_push_frame(target_method->max_locals, target_method->max_stack);
    
    // 3. Transferência de Argumentos
    // Copia os argumentos da Pilha de Operandos do Frame Antigo para Variáveis Locais do Novo Frame.
    for (size_t i = 0; i < arg_count; i++) {
        // Índice da variável local 'i' (começa em 0)
        size_t src_index = current_frame->stack_ptr - arg_count + i;
        new_frame->local_vars[i] = current_frame->operand_stack[src_index];
    }
    // Ajusta o stack_ptr do frame antigo (argumentos foram consumidos)
    current_frame->stack_ptr -= arg_count;

    // 4. Iniciar Execução no Novo Frame
    new_frame->method = target_method;
    current_frame = new_frame;
}

// -----------------------------------------------------------------------------
// 2. INVOKEVIRTUAL: Polimorfismo e Hierarquia
// -----------------------------------------------------------------------------
void exec_invokevirtual(uint16_t cp_index) {
    // 1. Obter a referência do objeto ('this') no topo da Pilha de Operandos
    if (current_frame->stack_ptr == 0) return; // Lançar NullPointerException
    
    ObjectRef obj_ref;
    // Desempilha a referência do objeto
    obj_ref.address = (void*)current_frame->operand_stack[--current_frame->stack_ptr]; 
    
    if (obj_ref.address == NULL) return; // Lançar NullPointerException

    // 2. Descobrir a Classe Real (usando a interface da Pessoa 1)
    ClassMetadata *real_class = heap_get_object_class(obj_ref);

    // 3. Resolver o Método na Hierarquia (Pessoa 4 - Polimorfismo)
    const char *method_sig = "targetMethod()V"; 
    MethodInfo *target_method = class_manager_find_virtual_method_in_hierarchy(real_class, method_sig);
    
    if (!target_method) return; // Lançar AbstractMethodError/NoSuchMethodError

    // 4. Criação do Frame e Transferência
    // invokevirtual inclui o 'this', então arg_count é +1
    size_t arg_count = get_argument_count(method_sig) + 1; 
    StackFrame *new_frame = jvm_stack_push_frame(target_method->max_locals, target_method->max_stack);

    // O 'this' deve ser o primeiro argumento (posição 0)
    // O 'this' deve ser o primeiro argumento (posição 0)
    new_frame->local_vars[0] = (StackValue)(uintptr_t)obj_ref.address; 
    
    // Transferir os argumentos restantes (1 até arg_count - 1)
    // A lógica é similar a invokestatic, mas começando do índice 1 em local_vars
    for (size_t i = 1; i < arg_count; i++) {
        size_t src_index = current_frame->stack_ptr - (arg_count - 1) + (i - 1);
        new_frame->local_vars[i] = current_frame->operand_stack[src_index];
    }
    current_frame->stack_ptr -= (arg_count - 1); // Ajustar o stack_ptr

    // 5. Iniciar Execução
    new_frame->method = target_method;
    current_frame = new_frame;
}

// -----------------------------------------------------------------------------
// 3. RETORNO: Limpeza e Continuidade
// -----------------------------------------------------------------------------
void exec_return_family(uint8_t opcode) {
    StackValue return_value = 0;
    bool has_return_value = (opcode != 0xB1); // 0xB1 é 'return'

    // 1. Capturar o valor de retorno (se houver)
    if (has_return_value) {
        return_value = current_frame->operand_stack[--current_frame->stack_ptr];
    }
    
    // 2. Desalocar o Frame Atual e Obter o Frame Anterior (Pessoa 1)
    StackFrame *calling_frame = jvm_stack_pop_frame(has_return_value ? &return_value : NULL); 
    
    if (!calling_frame) exit(0); // Pilha vazia: programa terminou.
    
    // 3. Colocar o valor de retorno na Pilha de Operandos do Frame Anterior
    if (has_return_value) {
        // O valor é colocado no topo da pilha do chamador.
        calling_frame->operand_stack[calling_frame->stack_ptr++] = return_value;
    }
    
    // 4. Continuar no Frame Anterior
    current_frame = calling_frame;
}

// -----------------------------------------------------------------------------
// 4. EXCEÇÕES: athrow e Busca na Tabela
// -----------------------------------------------------------------------------
void exec_athrow(uint16_t thrown_exception_cp_index) {
    // Referência do objeto de exceção
    StackValue exception_object_ref = current_frame->operand_stack[--current_frame->stack_ptr];
    MethodInfo *method = current_frame->method;
    
    // 1. Buscar um Handler na Tabela de Exceções do método atual
    for (size_t i = 0; i < method->exception_table_count; i++) {
        ExceptionTableEntry *entry = &method->exception_table[i];

        // a) O PC atual está na região TRY (start_pc <= PC < end_pc)
        bool pc_in_range = (current_frame->pc >= entry->start_pc && current_frame->pc < entry->end_pc);
        
        // b) O tipo da exceção lançada é compatível
        bool type_match = (entry->catch_type_index == 0) || // 0 captura qualquer coisa (finally)
                          class_manager_is_exception_compatible(thrown_exception_cp_index, entry->catch_type_index);

        if (pc_in_range && type_match) {
            
            // Handler Encontrado!
            
            // 2. Limpar Pilha de Operandos
            current_frame->stack_ptr = 0; 
            
            // 3. Empilhar a Referência da Exceção
            current_frame->operand_stack[current_frame->stack_ptr++] = exception_object_ref;
            
            // 4. Salto para o Handler (catch)
            current_frame->pc = entry->handler_pc;
            return;
        }
    }
    
    // 5. Unwinding: Nenhum Handler encontrado neste Frame
    // Desalocar o Frame Atual (sem valor de retorno)
    StackFrame *caller = jvm_stack_pop_frame(NULL);
    
    if (caller) {
        // Tentar buscar o handler no Frame anterior (re-lançamento)
        current_frame = caller;
        // O objeto de exceção deve ser repassado, neste caso, a função é chamada recursivamente
        exec_athrow(thrown_exception_cp_index); 
    } else {
        // Nenhuma exceção tratada em toda a pilha. JVM deve encerrar.
        exit(1); 
    }
}