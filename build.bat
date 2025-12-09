@echo off
echo === Compilando JVM Interpretador ===
echo.

set CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g

echo Compilando modulos...
gcc %CFLAGS% -c src/io.c -o src/io.o
gcc %CFLAGS% -c src/classfile.c -o src/classfile.o
gcc %CFLAGS% -c src/attributes.c -o src/attributes.o
gcc %CFLAGS% -c src/parse_code.c -o src/parse_code.o
gcc %CFLAGS% -c src/resolve.c -o src/resolve.o
gcc %CFLAGS% -c src/disasm.c -o src/disasm.o
gcc %CFLAGS% -c src/print.c -o src/print.o
gcc %CFLAGS% -c src/json.c -o src/json.o
gcc %CFLAGS% -c src/jvm.c -o src/jvm.o
gcc %CFLAGS% -c src/execute.c -o src/execute.o
gcc %CFLAGS% -c src/cli.c -o src/cli.o
gcc %CFLAGS% -c src/main.c -o src/main.o

echo.
echo Linkando executavel...
gcc src/*.o -o jvm.exe -lm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === Compilacao concluida com sucesso! ===
    echo.
    echo Executavel criado: jvm.exe
    echo.
    echo === Como usar ===
    echo Visualizar: jvm.exe arquivo.class
    echo Executar:   jvm.exe -run arquivo.class
    echo Debug:      jvm.exe -debug arquivo.class
    echo.
    
    if exist tests\samples\TestInterpreter.java (
        echo Compilando arquivo de teste...
        javac tests\samples\TestInterpreter.java
        if exist tests\samples\TestInterpreter.class (
            echo.
            echo Executando teste automaticamente...
            echo.
            jvm.exe -debug tests\samples\TestInterpreter.class
        )
    )
) else (
    echo.
    echo ERRO: Falha na compilacao!
    exit /b 1
)
