@echo off
title OpenTS3DS - Build Process
echo ===========================================
echo   Iniciando o build do OpenTS3DS (.exe)
echo ===========================================

:: Cria a pasta build se ela não existir
if not exist build (
    echo Criando diretorio de build...
    mkdir build
)

:: Compila o código fonte diretamente
:: Ajuste 'src/*.c' para os arquivos que você quiser incluir
echo Compilando com GCC...
gcc src/*.c -Iinclude -o build/OpenTS3DS.exe -O3 -Wall

:: Verifica se houve erro na compilação
if %errorlevel% neq 0 (
    echo.
    echo [ERRO] A compilacao falhou! Verifique seu codigo.
    pause
    exit /b
)

echo.
echo ===========================================
echo   Build concluido com sucesso!
echo   Executavel gerado: /build/OpenTS3DS.exe
echo ===========================================
pause
