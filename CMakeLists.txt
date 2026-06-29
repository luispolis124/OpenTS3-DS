cmake_minimum_required(VERSION 3.10)
project(OpenTS3-DS VERSION 1.0.0 LANGUAGES C)

# Define o padrão C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define o diretório de includes para facilitar o uso no código
include_directories(include)

# Lista os arquivos fonte atualizados conforme o nosso mapeamento
set(SOURCES 
    src/main.c
    src/game/entity.c
    # Se você criar o sistema de gestão (system.c) no futuro, adicione aqui
)

# Adiciona o executável
add_executable(OpenTS3-DS ${SOURCES})

# Renomeia o executável de saída
set_target_properties(OpenTS3-DS PROPERTIES OUTPUT_NAME "OpenTS3DS")

# Flags de aviso para ajudar no debug do seu port
if(MSVC)
    target_compile_options(OpenTS3-DS PRIVATE /W4)
else()
    target_compile_options(OpenTS3-DS PRIVATE -Wall -Wextra)
endif()
