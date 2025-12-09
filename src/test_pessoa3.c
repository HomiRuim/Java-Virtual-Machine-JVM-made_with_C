
#include <stdio.h>
#include <stdlib.h>
#include "../include/jvm_types.h"
#include "../include/exec_flow.h"
#include "../include/memory_manager.h"
#include "../include/class_manager.h"
extern int mock_frame_count;
extern MethodInfo mock_method_exception;

// Funções de teste
void run_test_invokestatic() {
    printf("--- Teste 1: invokestatic ---\n");
    // 1. Setup: Criar Frame Antigo com 2 argumentos (10 e 20)
    StackFrame *old_frame = jvm_stack_push_frame(10, 10);
    old_frame->operand_stack[old_frame->stack_ptr++] = 10;
    old_frame->operand_stack[old_frame->stack_ptr++] = 20;
    current_frame = old_frame;

    // 2. Execução
    exec_invokestatic(1); // CP index 1

    // 3. Verificação
    StackFrame *new_frame = current_frame; // O novo Frame é o current_frame
    
    // Novo Frame deve ter 10 e 20 nas Variáveis Locais 0 e 1
    if (new_frame->local_vars[0] == 10 && new_frame->local_vars[1] == 20) {
        printf("[OK] Argumentos transferidos corretamente.\n");
    } else {
        printf("[FAIL] Falha na transferência de argumentos.\n");
    }

    // Frame Antigo deve ter o stack_ptr ajustado (2 argumentos removidos)
    if (old_frame->stack_ptr == 0) {
        printf("[OK] Stack do frame antigo limpa.\n");
    } else {
        printf("[FAIL] Stack do frame antigo NÃO foi limpa.\n");
    }
    
    // Limpar o ambiente de Mock
    mock_frame_count = 0; 
    printf("------------------------------\n");
}
void run_test_invokevirtual() {
    printf("--- Teste 2: invokevirtual (Polimorfismo) ---\n");
    // 1. Setup: Frame Antigo, argumentos (30, 40) e a referência 'this' (0xDEADBEEF)
    StackFrame *old_frame = jvm_stack_push_frame(10, 10);
    
    // Argumentos
    old_frame->operand_stack[old_frame->stack_ptr++] = 30; 
    old_frame->operand_stack[old_frame->stack_ptr++] = 40; 
    
    // Referência do objeto (o 'this')
    StackValue this_ref = 0xDEADBEEF;
    old_frame->operand_stack[old_frame->stack_ptr++] = this_ref; 
    current_frame = old_frame;

    // 2. Execução
    exec_invokevirtual(2); // CP index 2

    // 3. Verificação
    StackFrame *new_frame = current_frame; 
    
    // Variável Local 0 deve ser o 'this'
    if (new_frame->local_vars[0] == this_ref) {
        printf("[OK] Referência 'this' transferida para local[0].\n");
    } else {
        printf("[FAIL] Falha na transferência de 'this'. Valor: %llu\n", new_frame->local_vars[0]);
    }
    
    // Argumentos devem estar em local[1] e local[2]
    if (new_frame->local_vars[1] == 30 && new_frame->local_vars[2] == 40) {
        printf("[OK] Argumentos restantes transferidos corretamente.\n");
    } else {
        printf("[FAIL] Falha na transferência de argumentos restantes.\n");
    }
    
    // Limpar
    mock_frame_count = 0; 
    printf("------------------------------\n");
}
void run_test_return() {
    printf("--- Teste 3: ireturn (Retorno) ---\n");
    // 1. Setup: Frame A (chamador) e Frame B (executando, topo da pilha)
    StackFrame *frame_a = jvm_stack_push_frame(10, 10); // Frame anterior
    frame_a->stack_ptr = 5; // Simular que tem 5 itens na pilha
    
    StackFrame *frame_b = jvm_stack_push_frame(10, 10); // Frame atual
    frame_b->operand_stack[frame_b->stack_ptr++] = 99; // Valor de Retorno
    current_frame = frame_b;

    // 2. Execução (ireturn = 0xAC)
    exec_return_family(0xAC); 

    // 3. Verificação
    StackFrame *calling_frame = current_frame; 
    
    // O current_frame deve ser Frame A
    if (calling_frame == frame_a) {
        printf("[OK] Retorno para o Frame Anterior (A).\n");
    } else {
        printf("[FAIL] Falha no retorno do Frame.\n");
    }

    // O valor de retorno (99) deve estar no topo da pilha do Frame A
    if (calling_frame->operand_stack[5] == 99 && calling_frame->stack_ptr == 6) {
        printf("[OK] Valor de retorno (99) empilhado no Frame A.\n");
    } else {
        printf("[FAIL] Falha ao empilhar valor de retorno.\n");
    }
    
    // Limpar
    mock_frame_count = 0; 
    printf("------------------------------\n");
}
void run_test_athrow() {
    printf("--- Teste 4: athrow (Exceções) ---\n");
    
    // 1. Setup: Frame com Tabela de Exceções e PC dentro do 'try'
    StackFrame *frame = jvm_stack_push_frame(10, 10);
    current_frame = frame;
    
    // Configurar tabela mock no MethodInfo
    ExceptionTableEntry mock_exception_table[] = {
        { .start_pc = 5, .end_pc = 15, .handler_pc = 50, .catch_type_index = 1 }
    };
    mock_method_exception.exception_table = mock_exception_table;
    mock_method_exception.exception_table_count = 1;
    frame->method = &mock_method_exception;
    
    // Simular estado: PC está em 10 (dentro do try), Pilha tem lixo e exceção
    frame->pc = 10;
    frame->operand_stack[frame->stack_ptr++] = 12345; // Lixo
    frame->operand_stack[frame->stack_ptr++] = 0xCAFEL; // Referência da Exceção
    
    // 2. Execução
    exec_athrow(1); // Lança a exceção do tipo 1

    // 3. Verificação
    // O PC deve ter saltado para o handler_pc
    if (current_frame->pc == 50) {
        printf("[OK] PC saltou para o handler_pc (50).\n");
    } else {
        printf("[FAIL] Falha no salto do PC. PC atual: %u\n", current_frame->pc);
    }
    
    // Pilha de Operandos deve ter sido limpa, sobrando apenas a referência da exceção
    if (current_frame->stack_ptr == 1 && current_frame->operand_stack[0] == 0xCAFEL) {
        printf("[OK] Pilha limpa, apenas referência de exceção permanece.\n");
    } else {
        printf("[FAIL] Pilha não foi limpa corretamente. Stack Ptr: %zu\n", current_frame->stack_ptr);
    }
    
    // Limpar
    mock_frame_count = 0; 
    printf("------------------------------\n");
}


// VARIÁVEL GLOBAL ESSENCIAL (Mock)
//StackFrame *current_frame; 

int main() {
    run_test_invokestatic();
    run_test_invokevirtual();
    run_test_return();
    run_test_athrow();
    return 0;
}