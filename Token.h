#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

using namespace std;

class Token {
public:
    string lexema;
    string tipo;
    string erro;
    int id = 0;
    int linha;
    int coluna;

    Token(string lexema, string tipo, int linha = 1, int coluna = 1)
        : lexema(lexema), tipo(tipo), linha(linha), coluna(coluna) {}

    string getLexema() { return lexema; }
    void setLexema(string l) { lexema = l; }

    string getTipo() { return tipo; }
    void setTipo(string t) { tipo = t; }

    int getId() { return id; }
    void setId(int *i) { id = *i; }

    string toString() {
        return "<" + tipo + ", " + lexema + ">";
    }
};

#endif
