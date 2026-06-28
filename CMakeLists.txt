cmake_minimum_required(VERSION 3.10)
project(OpenTS3-DS VERSION 1.0.0 LANGUAGES C)

# Define o padrão C11 (Bom para portabilidade)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Inclui os diretórios de cabeçalhos
include_directories(include)

# Lista os arquivos fonte
set(SOURCES 
    src/main.c
    src/core/entity_system.c
    src/core/entity_renderer.c
)

# Adiciona o executável
add_executable(OpenTS3-DS ${SOURCES})

# Renomeia o executável de saída para OpenTS3DS
set_target_properties(OpenTS3-DS PROPERTIES OUTPUT_NAME "OpenTS3DS")

# Opcional: Adiciona flags de aviso para ajudar no debug do seu port
if(MSVC)
    target_compile_options(OpenTS3-DS PRIVATE /W4)
else()
    target_compile_options(OpenTS3-DS PRIVATE -Wall -Wextra)
endif()
