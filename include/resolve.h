// include/resolve.h

#ifndef RESOLVE_H
#define RESOLVE_H

#include <stdint.h>
#include <stdlib.h> // Para size_t e char*

// Stubs para evitar a inclusão de classfile.h em todos os lugares,
// mas garantir que os tipos ClassFile/Classe sejam reconhecidos
typedef struct class_file ClassFile; // O tipo principal da Pessoa B

// =======================================================
// HELPER PRINCIPAL PARA REFERÊNCIAS (Methodref, Fieldref, etc.)
// =======================================================

/**
 * @brief Resolve uma referencia completa de metodo, campo ou interface.
 *
 * Utiliza a funcao cp_referencia_metodo da Pessoa B para desreferenciar
 * a cadeia de indices (Ex: Methodref -> Class -> UTF8) e monta a string final
 * no formato: Classe.Nome:Descritor.
 *
 * @param cf Estrutura ClassFile completa.
 * @param index O indice u2 lido do bytecode (Ex: 26).
 * @return String alocada dinamicamente (deve ser liberada pelo chamador).
 */
char* resolve_ref_to_string(const ClassFile *cf, uint16_t index);


// =======================================================
// HELPER PARA LITERAIS E CLASSES SIMPLES
// =======================================================

/**
 * @brief Resolve constantes que contem uma string ou um valor literal (String, Utf8, Integer, Float...).
 *
 * Utiliza o cp_utf8 ou acessa diretamente a union do CP (depende da implementacao).
 * Para este projeto, vamos focar em Utf8 e String.
 *
 * @param cf Estrutura ClassFile completa.
 * @param index O indice u1 ou u2 lido do bytecode (Ex: 5).
 * @return String alocada dinamicamente com o valor literal.
 */
char* resolve_literal_to_string(const ClassFile *cf, uint16_t index);


/**
 * @brief Resolve apenas o nome da classe (usado por opcodes como NEW, CHECKCAST).
 *
 * Utiliza a funcao cp_nome_classe da Pessoa B.
 *
 * @param cf Estrutura ClassFile completa.
 * @param index O indice u2 para uma CONSTANT_Class.
 * @return String alocada dinamicamente com o nome da classe.
 */
char* resolve_class_name_to_string(const ClassFile *cf, uint16_t index);


#endif // RESOLVE_H