// class_loader_mock.c
#include <stdlib.h>
#include <string.h>
#include "class_loader_mock.h"

// Simula o carregamento de uma classe
ClassMetadata* mock_class_loader_load_class(const char *class_name, size_t instance_size) {
    // 1. Aloca memória para a estrutura de metadados
    ClassMetadata *metadata = (ClassMetadata*)malloc(sizeof(ClassMetadata));
    if (!metadata) return NULL;

    // 2. Inicializa campos essenciais
    memset(metadata, 0, sizeof(ClassMetadata));
    metadata->this_class_name = strdup(class_name);
    
    // CRUCIAL: Preenche o tamanho da instância que o Gerente de Heap usará
    metadata->instance_size_in_stackvalues = instance_size;
    
    // Retorna a estrutura pronta para ser usada pela Heap
    return metadata;
}

// Simula a destruição dos metadados
void mock_class_loader_unload_class(ClassMetadata *metadata) {
    if (metadata) {
        if (metadata->this_class_name) {
            free(metadata->this_class_name);
        }
        free(metadata);
    }
}