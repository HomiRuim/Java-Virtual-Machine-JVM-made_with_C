# Script de compilação para Windows PowerShell

Write-Host "=== Compilador de Visualizador Bytecode (JVM Interpretador) ===" -ForegroundColor Cyan
Write-Host ""

$projectRoot = $PSScriptRoot
$includeDir = Join-Path $projectRoot "include"
$srcDir = Join-Path $projectRoot "src"

$cflags = "-Wall", "-Wextra", "-std=c99", "-I$includeDir", "-g"

Write-Host "Verificando ferramentas..." -ForegroundColor Yellow

if (!(Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Host "ERRO: GCC não encontrado!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Instale uma das opções:" -ForegroundColor Yellow
    Write-Host "1. MSYS2 (https://www.msys2.org/)" -ForegroundColor White
    Write-Host "2. MinGW-w64 (https://www.mingw-w64.org/)" -ForegroundColor White
    Write-Host "3. WSL (wsl --install)" -ForegroundColor White
    Write-Host ""
    exit 1
}

$gccVersion = gcc --version | Select-Object -First 1
Write-Host "✓ GCC encontrado: $gccVersion" -ForegroundColor Green

if (!(Get-Command javac -ErrorAction SilentlyContinue)) {
    Write-Host "⚠ AVISO: javac não encontrado. Você não poderá compilar arquivos .java" -ForegroundColor Yellow
} else {
    Write-Host "✓ javac encontrado" -ForegroundColor Green
}

Write-Host ""
Write-Host "Compilando módulos..." -ForegroundColor Yellow

# Limpar arquivos objeto antigos
Get-ChildItem -Path $srcDir -Filter "*.o" | Remove-Item -Force -ErrorAction SilentlyContinue

$sourceFiles = @(
    "src/io.c",
    "src/classfile.c",
    "src/attributes.c",
    "src/parse_code.c",
    "src/resolve.c",
    "src/disasm.c",
    "src/print.c",
    "src/json.c",
    "src/jvm.c",
    "src/execute.c",
    "src/cli.c",
    "src/main.c"
)

$objectFiles = @()

foreach ($sourceFile in $sourceFiles) {
    $fullPath = Join-Path $projectRoot $sourceFile
    $fileName = [System.IO.Path]::GetFileNameWithoutExtension($sourceFile)
    $objectFile = Join-Path $srcDir "$fileName.o"
    
    if (Test-Path $fullPath) {
        Write-Host "  Compilando $fileName..." -NoNewline
        $args = $cflags + @("-c", $fullPath, "-o", $objectFile)
        & gcc $args 2>&1 | Out-Null
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host " OK" -ForegroundColor Green
            $objectFiles += $objectFile
        } else {
            Write-Host " FALHOU" -ForegroundColor Red
            & gcc $args
            exit 1
        }
    } else {
        Write-Host "  ⚠ $sourceFile não encontrado" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Linkando executável principal..." -ForegroundColor Yellow

$mainExecutable = Join-Path $projectRoot "jvm.exe"
$args = $objectFiles + @("-o", $mainExecutable, "-lm")
& gcc $args 2>&1 | Out-Null

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Compilação concluída com sucesso!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Executável criado: jvm.exe" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "=== Como usar ===" -ForegroundColor Yellow
    Write-Host "Visualizar bytecode:" -ForegroundColor White
    Write-Host "  .\jvm.exe arquivo.class" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Executar método main:" -ForegroundColor White
    Write-Host "  .\jvm.exe -run arquivo.class" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Executar com debug:" -ForegroundColor White
    Write-Host "  .\jvm.exe -debug arquivo.class" -ForegroundColor Gray
    Write-Host ""
    
    # Compilar arquivo de teste Java se javac estiver disponível
    if (Get-Command javac -ErrorAction SilentlyContinue) {
        $testJava = Join-Path $projectRoot "tests\samples\TestInterpreter.java"
        $testClass = Join-Path $projectRoot "tests\samples\TestInterpreter.class"
        
        if (Test-Path $testJava) {
            Write-Host "Compilando arquivo de teste Java..." -ForegroundColor Yellow
            & javac $testJava 2>&1 | Out-Null
            
            if ($LASTEXITCODE -eq 0 -and (Test-Path $testClass)) {
                Write-Host "✓ TestInterpreter.class compilado" -ForegroundColor Green
                Write-Host ""
                Write-Host "Executando teste automaticamente..." -ForegroundColor Yellow
                Write-Host ""
                & $mainExecutable -debug $testClass
            }
        }
    }
} else {
    Write-Host "✗ Falha ao linkar executável" -ForegroundColor Red
    & gcc $args
    exit 1
}
