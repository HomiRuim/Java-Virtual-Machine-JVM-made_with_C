#include "../include/io.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <caminho_para_arquivo.class>\n", argv[0]);
        return 1;
    }

    const char* file_path = argv[1];
    Buffer buffer;
    Status status = buffer_from_file(file_path, &buffer);

    if (status != OK) {
        fprintf(stderr, "Erro ao carregar o arquivo %s: %d\n", file_path, status);
        return 1;
    }

    printf("Arquivo %s carregado com sucesso. Tamanho: %u bytes.\n", file_path, buffer.size);

    u4 magic;
    status = read_u4(&buffer, &magic);
    if (status != OK) {
        fprintf(stderr, "Erro ao ler o magic number: %d\n", status);
        buffer_free(&buffer);
        return 1;
    }

    printf("Magic Number: 0x%08X\n", magic);
    if (magic == 0xCAFEBABE) {
        printf("Magic Number CORRETO (0xCAFEBABE)!\n");
    } else {
        printf("Magic Number INCORRETO! Esperado 0xCAFEBABE.\n");
    }

    u2 minor_version;
    status = read_u2(&buffer, &minor_version);
    if (status != OK) {
        fprintf(stderr, "Erro ao ler a minor_version: %d\n", status);
        buffer_free(&buffer);
        return 1;
    }
    printf("Minor Version: %u\n", minor_version);

    u2 major_version;
    status = read_u2(&buffer, &major_version);
    if (status != OK) {
        fprintf(stderr, "Erro ao ler a major_version: %d\n", status);
        buffer_free(&buffer);
        return 1;
    }
    printf("Major Version: %u\n", major_version);

    printf("Offset atual: %u\n", buffer_tell(&buffer));

    buffer_free(&buffer);
    return 0;
}

