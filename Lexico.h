#ifndef LEXICO_H
#define LEXICO_H

#include <string>
#include <memory>
#include <cctype>
#include "LeitorArquivo.h"
#include "Token.h"

using namespace std;

class Lexico {
private:
    LeitorArquivo *arq;
    int *id;


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

                // Numero: sequencia de digitos
                if (isdigit(ch)) {
                    lexema = "";
                    lexema += ch;
                    estado = 30;
                    continue;
                }

                // identificador ou palavra-chave
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

            // estado 2: leu <
            if (estado == 2) {
                if (ch == '=') {
                    (*id)++;
                    estado = 1;
                    return new Token("<=", "OpRelMenorIgual");
                } else {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token("<", "OpRelMenor");
                }
            }

            // estado 5: leu =
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

            // estado 20: dentro de uma cadeia de caracteres
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

            // estado 30: lendo numero inteiro
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

            // estado 31: apos ponto decimal
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

            // AUTOMATO DE PALAVRAS-CHAVE e variaveis
            // Estado 40: dispatch pelo 1o caractere (ja em lexema)
            //            ch = 2a letra, ja lida, processada aqui mesmo

            if (estado == 40) {
                char first = lexema[0];

                // variavel: comeca com minuscula, acumula tudo
                if (islower(first)) {
                    if (isalpha(ch) || isdigit(ch) || ch == '_') {
                        lexema += ch;
                        continue;
                    } else {
                        arq->devolverCaracter();
                        (*id)++;
                        estado = 1;
                        return new Token(lexema, "Var");
                    }
                }

                // maiuscula: dispatch + consome ch (2a letra) aqui mesmo
                switch (first) {

                    // D => E => C  (DEC)
                    case 'D':
                        if (ch == 'E') { lexema += ch; estado = 42; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // P => R => O => G  (PROG)
                    case 'P':
                        if (ch == 'R') { lexema += ch; estado = 44; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // I => N => T  (INT)
                    // I => N => I  (INI)
                    // I => M => P => R => I => M => I => R  (IMPRIMIR)
                    case 'I':
                        if (ch == 'N') { lexema += ch; estado = 47; }
                        else if (ch == 'M') { lexema += ch; estado = 48; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // R => E => A => L  (REAL)
                    case 'R':
                        if (ch == 'E') { lexema += ch; estado = 55; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // L => E => R  (LER)
                    case 'L':
                        if (ch == 'E') { lexema += ch; estado = 58; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // S => E         (SE)
                    // S => E => N => A => O  (SENAO)
                    case 'S':
                        if (ch == 'E') { lexema += ch; estado = 72; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // E               (E  / OpBoolE)
                    // E => N => T => A => O  (ENTAO)
                    // E => N => Q => T => O  (ENQTO)
                    case 'E':
                        if (ch == 'N') { lexema += ch; estado = 63; }
                        else if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
                            arq->devolverCaracter();
                            (*id)++;
                            estado = 1;
                            return new Token(lexema, "OpBoolE");
                        }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // F => I => M  (FIM)
                    case 'F':
                        if (ch == 'I') { lexema += ch; estado = 68; }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    // O => U  (OU)
                    case 'O':
                        if (ch == 'U') {
                            lexema += ch;
                            (*id)++;
                            estado = 1;
                            return new Token(lexema, "OpBoolOu");
                        }
                        else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;

                    default:
                        if (isalpha(ch) || isdigit(ch) || ch == '_') {
                            lexema += ch;
                        } else {
                            arq->devolverCaracter(); estado = 1;
                            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                            lexema = "";
                        }
                        break;
                }
                continue;
            }

            // D => [E ja consumido no dispatch] => C
            // Estado 42: leu 'DE', espera 'C'
            if (estado == 42) {
                if (ch == 'C') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCDec"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // P => [R ja consumido] => O => G
            // Estado 44: leu 'PR', espera 'O'
            if (estado == 44) {
                if (ch == 'O') { lexema += ch; estado = 45; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 45: leu 'PRO', espera 'G'
            if (estado == 45) {
                if (ch == 'G') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCProg"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // I => [N ja consumido] => T / I
            // Estado 47: leu 'IN', espera 'T' (INT) ou 'I' (INI)
            if (estado == 47) {
                if (ch == 'T') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCInt"); }
                if (ch == 'I') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCIni"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // I => [M ja consumido] => P => R => I => M => I => R
            // Estado 48: leu 'IM', espera 'P'
            if (estado == 48) {
                if (ch == 'P') { lexema += ch; estado = 49; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 49: leu 'IMP', espera 'R'
            if (estado == 49) {
                if (ch == 'R') { lexema += ch; estado = 50; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 50: leu 'IMPR', espera 'I'
            if (estado == 50) {
                if (ch == 'I') { lexema += ch; estado = 51; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 51: leu 'IMPRI', espera 'M'
            if (estado == 51) {
                if (ch == 'M') { lexema += ch; estado = 52; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 52: leu 'IMPRIM', espera 'I'
            if (estado == 52) {
                if (ch == 'I') { lexema += ch; estado = 53; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 53: leu 'IMPRIMI', espera 'R'
            if (estado == 53) {
                if (ch == 'R') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCImprimir"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // R => [E ja consumido] => A => L
            // Estado 55: leu 'RE', espera 'A'
            if (estado == 55) {
                if (ch == 'A') { lexema += ch; estado = 56; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 56: leu 'REA', espera 'L'
            if (estado == 56) {
                if (ch == 'L') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCReal"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // L => [E ja consumido] => R
            // Estado 58: leu 'LE', espera 'R'
            if (estado == 58) {
                if (ch == 'R') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCLer"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // S => [E ja consumido] => aceita SE  ou  => N => A => O (SENAO)
            // Estado 72: leu 'SE'
            if (estado == 72) {
                if (ch == 'N') { lexema += ch; estado = 60; continue; }
                if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
                    arq->devolverCaracter();
                    (*id)++;
                    estado = 1;
                    return new Token(lexema, "PCSe");
                }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 60: leu 'SEN', espera 'A'
            if (estado == 60) {
                if (ch == 'A') { lexema += ch; estado = 61; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 61: leu 'SENA', espera 'O'
            if (estado == 61) {
                if (ch == 'O') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCSenao"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // E => [N ja consumido] => T => A => O  (ENTAO)
            //                      => Q => T => O  (ENQTO)
            // Estado 63: leu 'EN', espera 'T' ou 'Q'
            if (estado == 63) {
                if (ch == 'T') { lexema += ch; estado = 64; continue; }
                if (ch == 'Q') { lexema += ch; estado = 65; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 64: leu 'ENT', espera 'A'
            if (estado == 64) {
                if (ch == 'A') { lexema += ch; estado = 70; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 70: leu 'ENTA', espera 'O'
            if (estado == 70) {
                if (ch == 'O') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCEntao"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 65: leu 'ENQ', espera 'T'
            if (estado == 65) {
                if (ch == 'T') { lexema += ch; estado = 66; continue; }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // Estado 66: leu 'ENQT', espera 'O'
            if (estado == 66) {
                if (ch == 'O') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCEnqto"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

            // F => [I ja consumido] => M
            // Estado 68: leu 'FI', espera 'M'
            if (estado == 68) {
                if (ch == 'M') { lexema += ch; (*id)++; estado = 1; return new Token(lexema, "PCFim"); }
                arq->devolverCaracter(); estado = 1;
                cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (variaveis devem comecar com letra minuscula)" << endl;
                lexema = "";
                continue;
            }

        } // fim while

        // trata tokens pendentes ao atingir fim do arquivo
        if (estado == 2)  { (*id)++; return new Token("<", "OpRelMenor"); }
        if (estado == 7)  { (*id)++; return new Token(">", "OpRelMaior"); }
        if (estado == 12) { (*id)++; return new Token(":", "Delim"); }
        if (estado == 30) { (*id)++; return new Token(lexema, "NumInt"); }
        if (estado == 31) { (*id)++; return new Token(lexema, "NumReal"); }

        if (estado == 40 && !lexema.empty() && islower(lexema[0])) {
            (*id)++;
            return new Token(lexema, "Var");
        }

        // 'E' sozinho no EOF
        if (estado == 40 && !lexema.empty() && lexema[0] == 'E') {
            (*id)++;
            return new Token(lexema, "OpBoolE");
        }

        // 'SE' no EOF
        if (estado == 72) { (*id)++; return new Token(lexema, "PCSe"); }

        // palavras-chave incompletas no EOF
        if (estado >= 40 && !lexema.empty()) {
            cerr << "Erro Lexico: identificador invalido \"" << lexema << "\" (fim de arquivo inesperado)" << endl;
        }

        return nullptr;
    }
};

#endif