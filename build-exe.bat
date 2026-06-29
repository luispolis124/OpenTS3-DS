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

:: Compila todos os arquivos .c dentro das pastas src e subpastas
:: -Iinclude: aponta para a pasta de headers
:: -lSDL2 -lSDL2_mixer: linka as bibliotecas necessárias
echo Compilando com GCC...
gcc src/*.c src/core/*.c src/engine/*.c src/game/*.c -Iinclude -o build/OpenTS3DS.exe -O3 -Wall -lSDL2 -lSDL2_mixer

:: Verifica se houve erro na compilação
if %errorlevel% neq 0 (
    echo.
    echo [ERRO] A compilacao falhou! Verifique seu codigo e as libs do SDL2.
    pause
    exit /b
)

echo.
echo ===========================================
echo   Build concluido com sucesso!
echo   Executavel gerado: /build/OpenTS3DS.exe
echo ===========================================
pause
