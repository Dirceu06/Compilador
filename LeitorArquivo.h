#ifndef LEITOR_ARQUIVO_H
#define LEITOR_ARQUIVO_H

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

class LeitorArquivo {
private:
    ifstream file;
    int linha = 1;
    int coluna = 1;
    int linhaAnterior = 1;
    int colunaAnterior = 1;

public:
    LeitorArquivo(string arquivo) {
        file.open(arquivo);
        if (!file.is_open()) {
            cerr << "Erro: arquivo nao encontrado -> " << arquivo << endl;
        }
    }

    bool aberto() const { return file.is_open(); }
    int numeroLinha() const { return linha; }
    int numeroColuna() const { return coluna; }

    int lerProxCaracter() {
        const int caractere = file.get();
        linhaAnterior = linha;
        colunaAnterior = coluna;
        if (caractere == '\n') {
            ++linha;
            coluna = 1;
        } else if (caractere != -1) {
            ++coluna;
        }
        return caractere;
    }

    int espiarProxCaracter() { return file.peek(); }

    void devolverCaracter() {
        if (file.unget()) {
            linha = linhaAnterior;
            coluna = colunaAnterior;
        }
    }
};
#endif
