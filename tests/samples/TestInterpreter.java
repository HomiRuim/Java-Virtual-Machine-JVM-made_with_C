/**
 * Classe de teste para o interpretador da JVM.
 * Testa operações aritméticas, variáveis locais e estruturas de controle.
 */
public class TestInterpreter {
    public static void main(String[] args) {
        // Teste 1: Operações aritméticas básicas
        int a = 5;
        int b = 3;
        int soma = a + b;        // 8
        int subtracao = a - b;   // 2
        int multiplicacao = a * b; // 15
        int divisao = a / b;     // 1
        int resto = a % b;       // 2
        
        // Teste 2: Incremento
        int contador = 0;
        contador = contador + 1;
        contador = contador + 1;
        contador = contador + 1; // contador = 3
        
        // Teste 3: Negação
        int positivo = 10;
        int negativo = -positivo; // -10
        
        // Teste 4: Estrutura condicional simples
        int x = 10;
        if (x > 5) {
            x = x + 1; // x = 11
        }
        
        // Teste 5: Loop simples
        int i = 0;
        while (i < 5) {
            i = i + 1;
        }
        // i = 5
    }
}
