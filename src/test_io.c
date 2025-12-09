#include "../include/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_EQ(expected, actual, message) do { \
    if ((expected) != (actual)) { \
        printf("FALHOU: %s - Esperou %d, recebeu %d\n", message, (int)(expected), (int)(actual)); \
        return 1; \
    } \
    printf("PASSOU: %s\n", message); \
} while (0)

#define ASSERT_EQ_U4(expected, actual, message) do { \
    if ((expected) != (actual)) { \
        printf("FALHOU: %s - Esperou %u, recebeu %u\n", message, (unsigned int)(expected), (unsigned int)(actual)); \
        return 1; \
    } \
    printf("PASSOU: %s\n", message); \
} while (0)

#define ASSERT_EQ_U1(expected, actual, message) do { \
    if ((expected) != (actual)) { \
        printf("FALHOU: %s - Esperou %u, recebeu %u\n", message, (unsigned int)(expected), (unsigned int)(actual)); \
        return 1; \
    } \
    printf("PASSOU: %s\n", message); \
} while (0)


int test_buffer_from_file_success() {
    printf("\n--- Executando test_buffer_from_file_success ---\n");
    const char* test_file_path = "test_data_success.bin";
    FILE* fp = fopen(test_file_path, "wb");
    if (!fp) return 1;
    u1 data[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x00, 0x00, 0x00, 0x34};
    fwrite(data, 1, sizeof(data), fp);
    fclose(fp);

    Buffer buffer;
    Status status = buffer_from_file(test_file_path, &buffer);
    ASSERT_EQ(OK, status, "o buffer do arquivo deve retornar OK");
    ASSERT_EQ_U4(sizeof(data), buffer.size, "O tamanho do buffer deve corresponder ao tamanho do arquivo");
    ASSERT_EQ_U4(0, buffer.offset, "O offset do buffer deve ser 0");
    ASSERT_EQ_U1(0xCA, buffer.data[0], "Primeiro byte deve ser 0xCA");
    ASSERT_EQ_U1(0x34, buffer.data[7], "Ultimo byte deve ser 0x34");

    buffer_free(&buffer);
    remove(test_file_path);
    return 0;
}

int test_buffer_from_file_non_existent() {
    printf("\n--- Executando test_buffer_from_file_non_existent ---\n");
    const char* test_file_path = "non_existent.bin";
    Buffer buffer;
    Status status = buffer_from_file(test_file_path, &buffer);
    ASSERT_EQ(ERR_FILE, status, "buffer_from_file deve retornar ERR_FILE para um arquivo nao existente.");
    return 0;
}

int test_buffer_from_file_empty() {
    printf("\n--- Executando test_buffer_from_file_empty ---\n");
    const char* test_file_path = "empty_file.bin";
    FILE* fp = fopen(test_file_path, "wb");
    if (!fp) return 1;
    fclose(fp);

    Buffer buffer;
    Status status = buffer_from_file(test_file_path, &buffer);
    ASSERT_EQ(OK, status, "buffer_from_file deve retornar OK se for um arquivo vazio");
    ASSERT_EQ_U4(0, buffer.size, "O tamanho do buffer deve ser 0 se for um arquivo vazio");
    ASSERT_EQ_U4(0, buffer.offset, "O offset do buffer deve ser 0 se for um arquivo vazio");

    buffer_free(&buffer);
    remove(test_file_path);
    return 0;
}

int test_read_u1() {
    printf("\n--- Executando test_read_u1 ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(3);
    buffer.data[0] = 0xAA;
    buffer.data[1] = 0xBB;
    buffer.data[2] = 0xCC;
    buffer.size = 3;
    buffer.offset = 0;

    u1 val;
    Status status = read_u1(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u1 deve retornar OK");
    ASSERT_EQ_U1(0xAA, val, "read_u1 deve ler 0xAA");
    ASSERT_EQ_U4(1, buffer.offset, "Offset deve ser 1 depois de read_u1");

    status = read_u1(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u1 deve retornar OK");
    ASSERT_EQ_U1(0xBB, val, "read_u1 deve ler 0xBB");
    ASSERT_EQ_U4(2, buffer.offset, "Offset deve ser 2 depois de read_u1");

    status = read_u1(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u1 deve retornar OK");
    ASSERT_EQ_U1(0xCC, val, "read_u1 deve ler 0xCC");
    ASSERT_EQ_U4(3, buffer.offset, "Offset deve ser 3 depois de read_u1");

    free(buffer.data);
    return 0;
}

int test_read_u1_eof() {
    printf("\n--- Executando test_read_u1_eof ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(1);
    buffer.data[0] = 0x01;
    buffer.size = 1;
    buffer.offset = 0;

    u1 val;
    read_u1(&buffer, &val);
    Status status = read_u1(&buffer, &val);
    ASSERT_EQ(ERR_EOF, status, "read_u1 deve retornar ERR_EOF no final do buffer");

    free(buffer.data);
    return 0;
}

int test_read_u2() {
    printf("\n--- Executando test_read_u2 ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(4);
    buffer.data[0] = 0xDE;
    buffer.data[1] = 0xAD;
    buffer.data[2] = 0xBE;
    buffer.data[3] = 0xEF;
    buffer.size = 4;
    buffer.offset = 0;

    u2 val;
    Status status = read_u2(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u2 deve retornar OK");
    ASSERT_EQ_U4(0xDEAD, val, "read_u2 deve ler 0xDEAD (big-endian)");
    ASSERT_EQ_U4(2, buffer.offset, "Offset deve ser 2 depois de read_u2");

    status = read_u2(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u2 deve retornar OK");
    ASSERT_EQ_U4(0xBEEF, val, "read_u2 deve ler 0xBEEF (big-endian)");
    ASSERT_EQ_U4(4, buffer.offset, "Offset deve ser 4 depois de read_u2");

    free(buffer.data);
    return 0;
}

int test_read_u2_eof() {
    printf("\n--- Executando test_read_u2_eof ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(1);
    buffer.data[0] = 0x01;
    buffer.size = 1;
    buffer.offset = 0;

    u2 val;
    Status status = read_u2(&buffer, &val);
    ASSERT_EQ(ERR_EOF, status, "read_u2 deve retornar ERR_EOF se nao tiver bytes suficientes");

    free(buffer.data);
    return 0;
}

int test_read_u4() {
    printf("\n--- Executando test_read_u4 ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(8);
    buffer.data[0] = 0xCA;
    buffer.data[1] = 0xFE;
    buffer.data[2] = 0xBA;
    buffer.data[3] = 0xBE;
    buffer.data[4] = 0x00;
    buffer.data[5] = 0x00;
    buffer.data[6] = 0x00;
    buffer.data[7] = 0x34;
    buffer.size = 8;
    buffer.offset = 0;

    u4 val;
    Status status = read_u4(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u4 deve retornar OK");
    ASSERT_EQ_U4(0xCAFEBABE, val, "read_u4 deve ler 0xCAFEBABE (big-endian)");
    ASSERT_EQ_U4(4, buffer.offset, "Offset deve ser 4 depois de read_u4");

    status = read_u4(&buffer, &val);
    ASSERT_EQ(OK, status, "read_u4 deve retornar OK");
    ASSERT_EQ_U4(0x00000034, val, "read_u4 deve ler 0x00000034 (big-endian)");
    ASSERT_EQ_U4(8, buffer.offset, "Offset deve ser 8 depois de read_u4");

    free(buffer.data);
    return 0;
}

int test_read_u4_eof() {
    printf("\n--- Executando test_read_u4_eof ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(3);
    buffer.data[0] = 0x01;
    buffer.data[1] = 0x02;
    buffer.data[2] = 0x03;
    buffer.size = 3;
    buffer.offset = 0;

    u4 val;
    Status status = read_u4(&buffer, &val);
    ASSERT_EQ(ERR_EOF, status, "read_u4 deve retornar ERR_EOF se nao tiver bytes suficientes");

    free(buffer.data);
    return 0;
}

int test_read_bytes() {
    printf("\n--- Executando test_read_bytes ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(5);
    buffer.data[0] = 0x01;
    buffer.data[1] = 0x02;
    buffer.data[2] = 0x03;
    buffer.data[3] = 0x04;
    buffer.data[4] = 0x05;
    buffer.size = 5;
    buffer.offset = 0;

    u1 dest[3];
    Status status = read_bytes(&buffer, dest, 3);
    ASSERT_EQ(OK, status, "read_bytes deve retornar OK");
    ASSERT_EQ_U1(0x01, dest[0], "read_bytes primeiro byte");
    ASSERT_EQ_U1(0x02, dest[1], "read_bytes segundo byte");
    ASSERT_EQ_U1(0x03, dest[2], "read_bytes terceiro byte");
    ASSERT_EQ_U4(3, buffer.offset, "Offset deve ser 3 depois de read_bytes");

    u1 dest2[2];
    status = read_bytes(&buffer, dest2, 2);
    ASSERT_EQ(OK, status, "read_bytes deve retornar OK");
    ASSERT_EQ_U1(0x04, dest2[0], "read_bytes quarto byte");
    ASSERT_EQ_U1(0x05, dest2[1], "read_bytes quinto byte");
    ASSERT_EQ_U4(5, buffer.offset, "Offset deve ser 5 depois de read_bytes");

    free(buffer.data);
    return 0;
}

int test_read_bytes_eof() {
    printf("\n--- Executando test_read_bytes_eof ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(2);
    buffer.data[0] = 0x01;
    buffer.data[1] = 0x02;
    buffer.size = 2;
    buffer.offset = 0;

    u1 dest[3];
    Status status = read_bytes(&buffer, dest, 3);
    ASSERT_EQ(ERR_EOF, status, "read_bytes deve retornar ERR_EOF se nao tiver bytes suficientes");

    free(buffer.data);
    return 0;
}

int test_buffer_tell() {
    printf("\n--- Executando test_buffer_tell ---\n");
    Buffer buffer;
    buffer.data = (u1*)malloc(5);
    buffer.size = 5;
    buffer.offset = 2;

    ASSERT_EQ_U4(2, buffer_tell(&buffer), "buffer_tell deve retornar offset atual");

    free(buffer.data);
    return 0;
}

int main() {
    int failures = 0;

    failures += test_buffer_from_file_success();
    failures += test_buffer_from_file_non_existent();
    failures += test_buffer_from_file_empty();
    failures += test_read_u1();
    failures += test_read_u1_eof();
    failures += test_read_u2();
    failures += test_read_u2_eof();
    failures += test_read_u4();
    failures += test_read_u4_eof();
    failures += test_read_bytes();
    failures += test_read_bytes_eof();
    failures += test_buffer_tell();

    if (failures == 0) {
        printf("\nAll tests passed!\n");
    } else {
        printf("\n%d tests failed.\n", failures);
    }

    return failures;
}

