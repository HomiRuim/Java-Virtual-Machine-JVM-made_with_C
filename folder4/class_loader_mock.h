// class_loader_mock.h
#ifndef CLASS_LOADER_MOCK_H
#define CLASS_LOADER_MOCK_H

#include "class_metadata.h"

// Função que simula o carregamento e preenchimento de uma classe.
ClassMetadata* mock_class_loader_load_class(const char *class_name, size_t instance_size);

// Função que simula a destruição dos metadados mock.
void mock_class_loader_unload_class(ClassMetadata *metadata);

#endif // CLASS_LOADER_MOCK_H