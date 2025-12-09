#ifndef JSON_H
#define JSON_H

#include "base.h"       // Para definições de Status
#include "classfile.h"  // Para a definição de ClassFile
#include "cli.h"        // (NOVO) Para a estrutura CliOptions

/**
 * @brief Gera uma representação JSON da estrutura ClassFile,
 * respeitando as flags de CLI.
 *
 * @param cf Um ponteiro para a estrutura ClassFile preenchida.
 * @param options As flags parseadas da linha de comando.
 * @return Status
 */
Status json_classfile(ClassFile *cf, const CliOptions *options);

#endif // JSON_H