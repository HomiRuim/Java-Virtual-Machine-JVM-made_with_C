#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"
#include "jvm.h"
#include "classfile.h"

/**
 * Programa de teste para o modulo de gerenciamento da Stack.
 * 
 * Testa todas as funcionalidades implementadas:
 * - Operacoes da pilha de operandos (push/pop)
 * - Operacoes de variaveis locais
 * - Gerenciamento da call stack
 * - Funcoes de depuracao
 */

void test_operand_stack() {
    printf("\n--- TEST: Operand Stack Operations ---\n");
    
    // Cria um frame de teste (sem metodo real)
    Frame *frame = (Frame *)calloc(1, sizeof(Frame) + 20 * sizeof(Slot));
    assert(frame != NULL);
    
    // Simula um frame com max_locals=5, max_stack=10
    frame->local_vars = frame->slots_data;
    frame->operand_stack = frame->slots_data + 5;
    frame->stack_top = frame->operand_stack;
    
    // Testa se a pilha esta vazia
    assert(frame_is_stack_empty(frame) == 1);
    assert(frame_stack_size(frame) == 0);
    printf("V Stack inicialmente vazia\n");
    
    // Testa push
    assert(frame_push(frame, 42) == 0);
    assert(frame_push(frame, 100) == 0);
    assert(frame_push(frame, 255) == 0);
    assert(frame_stack_size(frame) == 3);
    printf("V Push de 3 valores: 42, 100, 255\n");
    
    // Testa peek
    Slot value;
    assert(frame_peek(frame, &value) == 0);
    assert(value == 255);
    assert(frame_stack_size(frame) == 3); // Peek nao remove
    printf("V Peek retornou 255 (topo nao foi removido)\n");
    
    // Testa pop
    assert(frame_pop(frame, &value) == 0);
    assert(value == 255);
    assert(frame_stack_size(frame) == 2);
    
    assert(frame_pop(frame, &value) == 0);
    assert(value == 100);
    
    assert(frame_pop(frame, &value) == 0);
    assert(value == 42);
    
    assert(frame_is_stack_empty(frame) == 1);
    printf("V Pop de 3 valores na ordem correta: 255, 100, 42\n");
    
    // Testa underflow
    assert(frame_pop(frame, &value) == -1);
    printf("V Stack underflow detectado corretamente\n");
    
    // Testa push/pop de long (64 bits)
    assert(frame_push_long(frame, 0x12345678, 0xABCDEF00) == 0);
    assert(frame_stack_size(frame) == 2);
    printf("V Push de long (2 slots)\n");
    
    Slot high, low;
    assert(frame_pop_long(frame, &high, &low) == 0);
    assert(high == 0x12345678);
    assert(low == 0xABCDEF00);
    assert(frame_is_stack_empty(frame) == 1);
    printf("V Pop de long correto\n");
    
    free(frame);
    printf("V Todos os testes da pilha de operandos passaram!\n");
}

void test_local_variables() {
    printf("\n--- TEST: Local Variables Operations ---\n");
    
    // Cria um frame de teste
    Frame *frame = (Frame *)calloc(1, sizeof(Frame) + 20 * sizeof(Slot));
    assert(frame != NULL);
    
    frame->local_vars = frame->slots_data;
    frame->operand_stack = frame->slots_data + 10;
    frame->stack_top = frame->operand_stack;
    
    // Testa set/get de variaveis locais
    assert(frame_set_local(frame, 0, 111) == 0);
    assert(frame_set_local(frame, 1, 222) == 0);
    assert(frame_set_local(frame, 5, 999) == 0);
    printf("V Set de variaveis locais [0]=111, [1]=222, [5]=999\n");
    
    Slot value;
    assert(frame_get_local(frame, 0, &value) == 0);
    assert(value == 111);
    
    assert(frame_get_local(frame, 1, &value) == 0);
    assert(value == 222);
    
    assert(frame_get_local(frame, 5, &value) == 0);
    assert(value == 999);
    printf("V Get de variaveis locais correto\n");
    
    // Testa set/get de long
    assert(frame_set_local_long(frame, 2, 0xDEADBEEF, 0xCAFEBABE) == 0);
    printf("V Set de long nas posicoes [2] e [3]\n");
    
    Slot high, low;
    assert(frame_get_local_long(frame, 2, &high, &low) == 0);
    assert(high == 0xDEADBEEF);
    assert(low == 0xCAFEBABE);
    printf("V Get de long correto\n");
    
    free(frame);
    printf("V Todos os testes de variaveis locais passaram!\n");
}

void test_call_stack() {
    printf("\n--- TEST: Call Stack Operations ---\n");
    
    // Cria a JVM
    JVMState *jvm = jvm_new();
    assert(jvm != NULL);
    assert(jvm_is_call_stack_empty(jvm) == 1);
    assert(jvm_frame_count(jvm) == 0);
    printf("V JVM criada com call stack vazia\n");
    
    // Simula empilhamento de frames (sem ClassFile/MethodInfo reais)
    // Nota: Em uso real, seria usado jvm_push_frame com argumentos validos
    
    // Cria frames manualmente para teste
    Frame *frame1 = (Frame *)calloc(1, sizeof(Frame) + 20 * sizeof(Slot));
    frame1->local_vars = frame1->slots_data;
    frame1->operand_stack = frame1->slots_data + 5;
    frame1->stack_top = frame1->operand_stack;
    frame1->next = NULL;
    
    Frame *frame2 = (Frame *)calloc(1, sizeof(Frame) + 20 * sizeof(Slot));
    frame2->local_vars = frame2->slots_data;
    frame2->operand_stack = frame2->slots_data + 5;
    frame2->stack_top = frame2->operand_stack;
    frame2->next = NULL;
    
    // Empilha manualmente
    jvm->call_stack = frame1;
    assert(jvm_frame_count(jvm) == 1);
    assert(jvm_current_frame(jvm) == frame1);
    printf("V Frame 1 empilhado\n");
    
    frame2->next = jvm->call_stack;
    jvm->call_stack = frame2;
    assert(jvm_frame_count(jvm) == 2);
    assert(jvm_current_frame(jvm) == frame2);
    printf("V Frame 2 empilhado\n");
    
    // Desempilha
    jvm_pop_frame(jvm);
    assert(jvm_frame_count(jvm) == 1);
    assert(jvm_current_frame(jvm) == frame1);
    printf("V Frame 2 desempilhado\n");
    
    jvm_pop_frame(jvm);
    assert(jvm_frame_count(jvm) == 0);
    assert(jvm_is_call_stack_empty(jvm) == 1);
    printf("V Frame 1 desempilhado\n");
    
    jvm_free(jvm);
    printf("V Todos os testes da call stack passaram!\n");
}

void test_integration() {
    printf("\n--- TEST: Integration Test ---\n");
    
    // Simula um cenario de execucao mais realista
    JVMState *jvm = jvm_new();
    
    // Cria um frame simulando metodo main
    Frame *main_frame = (Frame *)calloc(1, sizeof(Frame) + 30 * sizeof(Slot));
    main_frame->local_vars = main_frame->slots_data;
    main_frame->operand_stack = main_frame->slots_data + 10;
    main_frame->stack_top = main_frame->operand_stack;
    main_frame->next = NULL;
    
    jvm->call_stack = main_frame;
    
    printf("Simulando execucao de bytecode:\n");
    
    // Simula: iconst_5 (push 5)
    frame_push(main_frame, 5);
    printf("  iconst_5 -> stack = [5]\n");
    
    // Simula: istore_0 (pop e armazena em local[0])
    Slot value;
    frame_pop(main_frame, &value);
    frame_set_local(main_frame, 0, value);
    printf("  istore_0 -> local[0] = 5\n");
    
    // Simula: iconst_3 (push 3)
    frame_push(main_frame, 3);
    printf("  iconst_3 -> stack = [3]\n");
    
    // Simula: iload_0 (push local[0])
    frame_get_local(main_frame, 0, &value);
    frame_push(main_frame, value);
    printf("  iload_0 -> stack = [3, 5]\n");
    
    // Simula: iadd (pop 2, soma, push resultado)
    Slot b, a;
    frame_pop(main_frame, &b);
    frame_pop(main_frame, &a);
    frame_push(main_frame, a + b);
    printf("  iadd -> stack = [8]\n");
    
    // Verifica resultado
    frame_pop(main_frame, &value);
    assert(value == 8);
    printf("V Resultado da operacao: 5 + 3 = %u\n", value);
    
    jvm_free(jvm);
    printf("V Teste de integracao passou!\n");
}

void test_debug_functions() {
    printf("\n--- TEST: Debug Functions ---\n");
    
    JVMState *jvm = jvm_new();
    
    Frame *frame = (Frame *)calloc(1, sizeof(Frame) + 30 * sizeof(Slot));
    frame->local_vars = frame->slots_data;
    frame->operand_stack = frame->slots_data + 10;
    frame->stack_top = frame->operand_stack;
    frame->next = NULL;
    
    jvm->call_stack = frame;
    
    // Popula o frame com alguns dados
    frame_set_local(frame, 0, 42);
    frame_set_local(frame, 1, 100);
    frame_push(frame, 999);
    frame_push(frame, 888);
    
    printf("\nTestando frame_print_state:\n");
    frame_print_state(frame);
    
    printf("\nTestando jvm_print_call_stack:\n");
    jvm_print_call_stack(jvm);
    
    jvm_free(jvm);
    printf("V Funcoes de debug executadas com sucesso!\n");
}

int main() {
    printf("----------------------------------------------------------\n");
    printf("|  TESTE DO MoDULO DE GERENCIAMENTO DA STACK (PESSOA 1)  |\n");
    printf("----------------------------------------------------------\n");
    
    test_operand_stack();
    test_local_variables();
    test_call_stack();
    test_integration();
    test_debug_functions();
    
    printf("\n----------------------------------------------------------\n");
    printf("|           V TODOS OS TESTES PASSARAM! V               |\n");
    printf("----------------------------------------------------------\n\n");
    
    return 0;
}
