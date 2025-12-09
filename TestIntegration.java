public class TestIntegration {
    public static void main(String[] args) {
        int a = 10;
        int b = 20;
        int c = a + b;
        int d = c - 5;
        int e = d * 2;
        int f = e / 3;
        int g = f % 7;
        int h = -g;
        
        // Teste de controle de fluxo
        if (h > 0) {
            h = h + 1;
        } else {
            h = h - 1;
        }
        
        // Teste de switch
        int result = 0;
        switch (h) {
            case 0:
                result = 100;
                break;
            case 1:
                result = 200;
                break;
            case 2:
                result = 300;
                break;
            default:
                result = 999;
                break;
        }
    }
}
