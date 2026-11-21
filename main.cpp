/**
 * @file main.cpp
 * 
 * @brief Entrega Parcial AP2 - Analisador Sintático
 * 
 * Dirceu Morais da Costa Júnior (2669455)
 * Lucas Henrique Motta			 (2669730)
 */

#include <iostream>
#include "Sintatico.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo.gyh>\n";
        return 2;
    }

    Lexico lexico(argv[1]);
    if (!lexico.arquivoAberto()) {
        return 2;
    }

    Sintatico sintatico(lexico);
    if (!sintatico.analisar()) {
        return 1;
    }

    std::cout << "Programa sintaticamente válido.\n";
    return 0;
}
