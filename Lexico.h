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

    // tabela de palavras-chave da linguagem GYH
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

            // estado 1 inicial
            if (estado == 1) {

                // ignora espaço em branco
                if (isspace(ch)) {
                    continue;
                }

                // ignora comentarios (inicia c/ #)
                if (ch == '#') {
                    // vai até fim da linha
                    while ((c = arq->lerProxCaracter()) != -1) {
                        if ((char)c == '\n') break;
                    }
                    continue;
                }

                // op aritmeticos
                if (ch == '+') { (*id)++; return new Token("+", "OpAritSoma"); }
                if (ch == '-') { (*id)++; return new Token("-", "OpAritSub");  }
                if (ch == '*') { (*id)++; return new Token("*", "OpAritMult"); }
                if (ch == '/') { (*id)++; return new Token("/", "OpAritDiv");  }

                if (ch == '(') { (*id)++; return new Token("(", "AbrePar");   }
                if (ch == ')') { (*id)++; return new Token(")", "FechaPar");   }

                // < ou <=
                if (ch == '<') { estado = 2; continue; }

                // > ou >=
                if (ch == '>') { estado = 7; continue; }

                // = isolado (erro) ou parte de ==
                if (ch == '=') { estado = 5; continue; }

                // ! deve ser seguido de =
                if (ch == '!') { estado = 10; continue; }

                // : (delim) ou := (atrib)
                if (ch == ':') { estado = 12; continue; }

                // cadeia de caracteres entre aspas duplas
                if (ch == '"') {
                    lexema = "";
                    estado = 20;
                    continue;
                }

                // Numero: sequencia de dígitos
                if (isdigit(ch)) {
                    lexema = "";
                    lexema += ch;
                    estado = 30;
                    continue;
                }

                // identificador ou palavra-chave
                // - variavel: começa com letra minúscula
                // - palavra-chave: começa com letra maiúscula
                if (isalpha(ch)) {
                    lexema = "";
                    lexema += ch;
                    estado = 40;
                    continue;
                }

                // caractere desconhecido
                cerr << "Erro Lexico: caractere desconhecido '" << ch << "'" << endl;
                continue;
            }

            // estado 2: <
            if (estado == 2) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("<=", "OpRelMenorIgual");
                } else {
                    // devolve o caractere lido (não faz parte de <=)
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token("<", "OpRelMenor");
                }
            }

            // estado 5: leu = (pode ser == ou erro léxico)
            if (estado == 5) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("==", "OpRelIgual");
                } else {
                    arq->devolverCaracter();
                    estado = 1;
                    cerr << "Erro Lexico: '=' isolado nao e um token valido" << endl;
                    continue;
                }
            }

            // estado 7: leu >
            if (estado == 7) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token(">=", "OpRelMaiorIgual");
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(">", "OpRelMaior");
                }
            }

            // estado 10: leu !
            if (estado == 10) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("!=", "OpRelDif");
                } else {
                    arq->devolverCaracter();
                    estado = 1;
                    cerr << "Erro Lexico: '!' isolado nao e um token valido" << endl;
                    continue;
                }
            }

            // estado 12: leu :
            if (estado == 12) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token(":=", "Atrib");
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(":", "Delim");
                }
            }

            // estado 20: dentro de uma cadeia de caracteres (string)
            if (estado == 20) {
                if (ch == '"') {
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "Cadeia");
                } else if (ch == '\n' || c == -1) {
                    estado = 1;
                    cerr << "Erro Lexico: cadeia de caracteres nao fechada" << endl;
                    continue;
                } else {
                    lexema += ch;
                    continue;
                }
            }

            // estado 30: lendo número (inteiro ou real)
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
                    return new Token(lexema, "NumInt");
                }
            }

            // estado 31: após o ponto decimal do número real
            if (estado == 31) {
                if (isdigit(ch)) {
                    lexema += ch;
                    continue;
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "NumReal");
                }
            }

            // estado 40: lendo identificador ou palavra chave
            if (estado == 40) {
                if (isalpha(ch) || isdigit(ch) || ch == '_') {
                    lexema += ch;
                    continue;
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;

                    // olha se é palavra-chave (maiuscula) ou variavel (minuscula)
                    auto it = palavrasChave.find(lexema);
                    if (it != palavrasChave.end()) {
                        return new Token(lexema, it->second);
                    }

                    // variavel
                    if (islower(lexema[0])) {
                        return new Token(lexema, "Var");
                    }

                    // começa com maiuscula mas nao é palavra chave
                    cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                    continue;
                }
            }

        }

        // trata tokens pendentes ao atingir fim do arquivo
        if (estado == 2) {
            (*id)++;
            return new Token("<", "OpRelMenor");
        }
        if (estado == 7) {
            (*id)++;
            return new Token(">", "OpRelMaior");
        }
        if (estado == 12) {
            (*id)++;
            return new Token(":", "Delim");
        }
        if (estado == 30) {
            (*id)++;
            return new Token(lexema, "NumInt");
        }
        if (estado == 31) {
            (*id)++;
            return new Token(lexema, "NumReal");
        }
        if (estado == 40) {
            (*id)++;
            auto it = palavrasChave.find(lexema);
            if (it != palavrasChave.end()) {
                return new Token(lexema, it->second);
            }
            if (islower(lexema[0])) {
                return new Token(lexema, "Var");
            }
            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\"" << endl;
        }

        return nullptr;
    }
};

#endif