#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

using namespace std;

class Token {
public:
    string lexema;
    string tipo;
    int id;

    Token(string lexema, string tipo)
        : lexema(lexema), tipo(tipo) {}

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
