#ifndef PRINT_H
#define PRINT_H

#include "base.h"       // Para Status
#include "classfile.h"  // PARA A DEFINIÇÃO COMPLETA de ClassFile
#include "cli.h"        // (NOVO) Para a estrutura CliOptions

/**
 * @brief Imprime o ClassFile de forma legivel, 
 * respeitando as flags de CLI.
 *
 * @param cf Estrutura ClassFile completa.
 * @param options As flags parseadas da linha de comando (ex: --no-code).
 * @return Status
 */
Status print_classfile(ClassFile *cf, const CliOptions *options);

#endif // PRINT_H