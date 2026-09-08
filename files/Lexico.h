#ifndef LEXICO_H
#define LEXICO_H

#include <string>
#include <memory>
#include <cctype>
#include <map>
#include "LeitorArquivo.h"
#include "Token.h"

using namespace std;

class Lexico {
private:
    LeitorArquivo *arq;
    int *id;

    // Tabela de palavras-chave da linguagem GYH
    map<string, string> palavrasChave = {
        {"DEC",      "PCDec"},
        {"PROG",     "PCProg"},
        {"INT",      "PCInt"},
        {"REAL",     "PCReal"},
        {"LER",      "PCLer"},
        {"IMPRIMIR", "PCImprimir"},
        {"SE",       "PCSe"},
        {"ENTAO",    "PCEntao"},
        {"SENAO",    "PCSenao"},
        {"ENQTO",    "PCEnqto"},
        {"INI",      "PCIni"},
        {"FIM",      "PCFim"},
        {"E",        "OpBoolE"},
        {"OU",       "OpBoolOu"}
    };

public:
    Lexico(string arquivo) {
        id = new int(0);
        arq = new LeitorArquivo(arquivo);
    }

    ~Lexico() {
        delete id;
        delete arq;
    }

    Token* proximoToken() {
        int c;
        char ch;
        int estado = 1;
        string lexema = "";

        while ((c = arq->lerProxCaracter()) != -1) {
            ch = (char)c;

            // Estado 1: estado inicial
            if (estado == 1) {

                // Ignora espaços em branco
                if (isspace(ch)) {
                    continue;
                }

                // Ignora comentários iniciados por '#'
                if (ch == '#') {
                    // Consome até o fim da linha
                    while ((c = arq->lerProxCaracter()) != -1) {
                        if ((char)c == '\n') break;
                    }
                    continue;
                }

                // Operadores aritméticos simples
                if (ch == '+') { (*id)++; return new Token("+", "OpAritSoma", id); }
                if (ch == '-') { (*id)++; return new Token("-", "OpAritSub", id);  }
                if (ch == '*') { (*id)++; return new Token("*", "OpAritMult", id); }
                if (ch == '/') { (*id)++; return new Token("/", "OpAritDiv", id);  }

                // Parênteses
                if (ch == '(') { (*id)++; return new Token("(", "AbrePar", id);   }
                if (ch == ')') { (*id)++; return new Token(")", "FechaPar", id);   }

                // '<' -> pode ser '<' ou '<='
                if (ch == '<') { estado = 2; continue; }

                // '>' -> pode ser '>' ou '>='
                if (ch == '>') { estado = 7; continue; }

                // '=' -> pode ser '=' isolado (erro) ou parte de '=='
                if (ch == '=') { estado = 5; continue; }

                // '!' -> deve ser seguido de '=' para formar '!='
                if (ch == '!') { estado = 10; continue; }

                // ':' -> pode ser ':' (Delim) ou ':=' (Atrib)
                if (ch == ':') { estado = 12; continue; }

                // Cadeia de caracteres: sequência entre aspas duplas
                if (ch == '"') {
                    lexema = "";
                    estado = 20;
                    continue;
                }

                // Número: sequência de dígitos
                if (isdigit(ch)) {
                    lexema = "";
                    lexema += ch;
                    estado = 30;
                    continue;
                }

                // Identificador ou palavra-chave:
                // - Variável: começa com letra minúscula
                // - Palavra-chave: começa com letra maiúscula
                if (isalpha(ch)) {
                    lexema = "";
                    lexema += ch;
                    estado = 40;
                    continue;
                }

                // Caractere desconhecido -> Erro Léxico
                cerr << "Erro Lexico: caractere desconhecido '" << ch << "'" << endl;
                continue;
            }

            // -------------------------------------------------------
            // Estado 2: leu '<'
            // -------------------------------------------------------
            if (estado == 2) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("<=", "OpRelMenorIgual", id);
                } else {
                    // Devolve o caractere lido (não faz parte de '<=')
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token("<", "OpRelMenor", id);
                }
            }

            // -------------------------------------------------------
            // Estado 5: leu '=' (pode ser '==' ou erro léxico)
            // -------------------------------------------------------
            if (estado == 5) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("==", "OpRelIgual", id);
                } else {
                    arq->devolverCaracter();
                    estado = 1;
                    cerr << "Erro Lexico: '=' isolado nao e um token valido" << endl;
                    continue;
                }
            }

            // -------------------------------------------------------
            // Estado 7: leu '>'
            // -------------------------------------------------------
            if (estado == 7) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token(">=", "OpRelMaiorIgual", id);
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(">", "OpRelMaior", id);
                }
            }

            // -------------------------------------------------------
            // Estado 10: leu '!'
            // -------------------------------------------------------
            if (estado == 10) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("!=", "OpRelDif", id);
                } else {
                    arq->devolverCaracter();
                    estado = 1;
                    cerr << "Erro Lexico: '!' isolado nao e um token valido" << endl;
                    continue;
                }
            }

            // -------------------------------------------------------
            // Estado 12: leu ':'
            // -------------------------------------------------------
            if (estado == 12) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token(":=", "Atrib", id);
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(":", "Delim", id);
                }
            }

            // -------------------------------------------------------
            // Estado 20: dentro de uma cadeia de caracteres (string)
            // -------------------------------------------------------
            if (estado == 20) {
                if (ch == '"') {
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "Cadeia", id);
                } else if (ch == '\n' || c == -1) {
                    estado = 1;
                    cerr << "Erro Lexico: cadeia de caracteres nao fechada" << endl;
                    continue;
                } else {
                    lexema += ch;
                    continue;
                }
            }

            // -------------------------------------------------------
            // Estado 30: lendo número (inteiro ou real)
            // -------------------------------------------------------
            if (estado == 30) {
                if (isdigit(ch)) {
                    lexema += ch;
                    continue;
                } else if (ch == '.') {
                    lexema += ch;
                    estado = 31;
                    continue;
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "NumInt", id);
                }
            }

            // -------------------------------------------------------
            // Estado 31: após o ponto decimal do número real
            // -------------------------------------------------------
            if (estado == 31) {
                if (isdigit(ch)) {
                    lexema += ch;
                    continue;
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "NumReal", id);
                }
            }

            // -------------------------------------------------------
            // Estado 40: lendo identificador ou palavra-chave
            // -------------------------------------------------------
            if (estado == 40) {
                if (isalpha(ch) || isdigit(ch) || ch == '_') {
                    lexema += ch;
                    continue;
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;

                    // Verifica se é palavra-chave (maiúsculas) ou variável (minúscula inicial)
                    auto it = palavrasChave.find(lexema);
                    if (it != palavrasChave.end()) {
                        return new Token(lexema, it->second, id);
                    }

                    // Variável: deve começar com letra minúscula
                    if (islower(lexema[0])) {
                        return new Token(lexema, "Var", id);
                    }

                    // Começa com maiúscula mas não é palavra-chave -> Erro Léxico
                    cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                    continue;
                }
            }

        } // fim do while

        // Trata tokens pendentes ao atingir fim de arquivo
        if (estado == 2) {
            (*id)++;
            return new Token("<", "OpRelMenor", id);
        }
        if (estado == 7) {
            (*id)++;
            return new Token(">", "OpRelMaior", id);
        }
        if (estado == 12) {
            (*id)++;
            return new Token(":", "Delim", id);
        }
        if (estado == 30) {
            (*id)++;
            return new Token(lexema, "NumInt", id);
        }
        if (estado == 31) {
            (*id)++;
            return new Token(lexema, "NumReal", id);
        }
        if (estado == 40) {
            (*id)++;
            auto it = palavrasChave.find(lexema);
            if (it != palavrasChave.end()) {
                return new Token(lexema, it->second, id);
            }
            if (islower(lexema[0])) {
                return new Token(lexema, "Var", id);
            }
            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\"" << endl;
        }

        return nullptr;
    }
};

#endif
