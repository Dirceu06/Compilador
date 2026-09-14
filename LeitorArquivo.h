#ifndef LEITOR_ARQUIVO_H
#define LEITOR_ARQUIVO_H

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

class LeitorArquivo {
private:
    ifstream file;

public:
    LeitorArquivo(string arquivo) {
        file.open(arquivo);
        if (!file.is_open()) {
            cerr << "Erro: arquivo nao encontrado -> " << arquivo << endl;
        }
    }

    ~LeitorArquivo() {
        if (file.is_open()) {
            file.close();
        }
    }

    int lerProxCaracter() {
        int caractere = -1;

        if (file.is_open()) {
            char c;
            if (file.get(c)) {
                caractere = c;                 
            } else {
                caractere = -1;
            }
        }

        return caractere;
    }
       
    void devolverCaracter() {
        if (file.is_open()) {
            file.unget();
        }
    } 
};
#endif