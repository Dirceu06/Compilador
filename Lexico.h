#ifndef LEXICO_H
#define LEXICO_H

#include <string>
#include <cctype>
#include "LeitorArquivo.h"
#include "Token.h"

using namespace std;

class Lexico {
private:
    LeitorArquivo arq;

    static bool caractereDePalavra(int c) {
        return isalnum(c) || c == '_';
    }

public:
    explicit Lexico(const string& arquivo) : arq(arquivo) {}

    bool arquivoAberto() const { return arq.aberto(); }
    int numeroLinha() const { return arq.numeroLinha(); }
    int numeroColuna() const { return arq.numeroColuna(); }

    Token* proximoToken() {
        int estado = 1;
        string lexema;
        int linha = arq.numeroLinha();
        int coluna = arq.numeroColuna();

        auto erro = [&](const string& trecho, const string& mensagem) {
            Token* token = new Token(trecho, "ErroLexico", linha, coluna);
            token->erro = mensagem;
            return token;
        };

        auto palavraInvalida = [&]() {
            while (caractereDePalavra(arq.espiarProxCaracter())) {
                lexema += static_cast<char>(arq.lerProxCaracter());
            }
            return erro(lexema, "palavra ou identificador invalido '" + lexema + "'");
        };

        auto emitir = [&](const string& texto, const string& tipo) {
            const bool palavraReservada = tipo.compare(0, 2, "PC") == 0
                || tipo == "OpBoolE" || tipo == "OpBoolOu";
            if (palavraReservada && caractereDePalavra(arq.espiarProxCaracter())) {
                return palavraInvalida();
            }
            if (tipo == "PCReal") {
                return erro(texto, "palavra-chave 'REAL' fora da tabela de tokens da AP2");
            }
            if (tipo == "NumReal") {
                return erro(texto, "número '" + texto + "' invalido; a AP2 usa números inteiros");
            }
            if (tipo == "Var" && texto.find('_') != string::npos) {
                return palavraInvalida();
            }
            return new Token(texto, tipo, linha, coluna);
        };

        int c;
        while ((c = arq.lerProxCaracter()) != -1) {
            const auto ch = static_cast<unsigned char>(c);

            // estado 1 inicial
            if (estado == 1) {
                linha = arq.numeroLinha();
                coluna = arq.numeroColuna() - 1;

                // ignora espaço em branco
                if (isspace(ch)) {
                    continue;
                }

                // ignora comentarios (inicia c/ #)
                if (ch == '#') {
                    while ((c = arq.lerProxCaracter()) != -1) {
                        if ((char)c == '\n') break;
                    }
                    continue;
                }

                // op aritmeticos
                if (ch == '+') { return emitir("+", "OpAritSoma"); }
                if (ch == '-') { return emitir("-", "OpAritSub");  }
                if (ch == '*') { return emitir("*", "OpAritMult"); }
                if (ch == '/') { return emitir("/", "OpAritDiv");  }

                if (ch == '(') { return emitir("(", "AbrePar");   }
                if (ch == ')') { return emitir(")", "FechaPar");   }

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
                return erro(string(1, ch), "caractere desconhecido '" + string(1, ch) + "'");
            }

            // estado 2: leu <
            if (estado == 2) {
                if (ch == '=') {
                    return emitir("<=", "OpRelMenorIgual");
                } else {
                    arq.devolverCaracter();
                    return emitir("<", "OpRelMenor");
                }
            }

            // estado 5: leu =
            if (estado == 5) {
                if (ch == '=') {
                    return emitir("==", "OpRelIgual");
                } else {
                    arq.devolverCaracter();
                    return erro("=", "'=' isolado nao e um token valido");
                }
            }

            // estado 7: leu >
            if (estado == 7) {
                if (ch == '=') {
                    return emitir(">=", "OpRelMaiorIgual");
                } else {
                    arq.devolverCaracter();
                    return emitir(">", "OpRelMaior");
                }
            }

            // estado 10: leu !
            if (estado == 10) {
                if (ch == '=') {
                    return emitir("!=", "OpRelDif");
                } else {
                    arq.devolverCaracter();
                    return erro("!", "'!' isolado nao e um token valido");
                }
            }

            // estado 12: leu :
            if (estado == 12) {
                if (ch == '=') {
                    return emitir(":=", "Atrib");
                } else {
                    arq.devolverCaracter();
                    return emitir(":", "Delim");
                }
            }

            // estado 20: dentro de uma cadeia de caracteres
            if (estado == 20) {
                if (ch == '"') {
                    return emitir(lexema, "Cadeia");
                } else if (ch == '\n') {
                    return erro("\"" + lexema, "cadeia de caracteres nao fechada");
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
                    arq.devolverCaracter();
                    return emitir(lexema, "NumInt");
                }
            }

            // estado 31: apos ponto decimal
            if (estado == 31) {
                if (isdigit(ch)) {
                    lexema += ch;
                    continue;
                } else {
                    arq.devolverCaracter();
                    return emitir(lexema, "NumReal");
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
                        arq.devolverCaracter();
                        return emitir(lexema, "Var");
                    }
                }

                // maiuscula: dispatch + consome ch (2a letra) aqui mesmo
                switch (first) {

                    // D => E => C  (DEC)
                    case 'D':
                        if (ch == 'E') { lexema += ch; estado = 42; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // P => R => O => G  (PROG)
                    case 'P':
                        if (ch == 'R') { lexema += ch; estado = 44; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // I => N => T  (INT)
                    // I => N => I  (INI)
                    // I => M => P => R => I => M => I => R  (IMPRIMIR)
                    case 'I':
                        if (ch == 'N') { lexema += ch; estado = 47; }
                        else if (ch == 'M') { lexema += ch; estado = 48; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // R => E => A => L  (REAL)
                    case 'R':
                        if (ch == 'E') { lexema += ch; estado = 55; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // L => E => R  (LER)
                    case 'L':
                        if (ch == 'E') { lexema += ch; estado = 58; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // S => E         (SE)
                    // S => E => N => A => O  (SENAO)
                    case 'S':
                        if (ch == 'E') { lexema += ch; estado = 72; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // E               (E  / OpBoolE)
                    // E => N => T => A => O  (ENTAO)
                    // E => N => Q => T => O  (ENQTO)
                    case 'E':
                        if (ch == 'N') { lexema += ch; estado = 63; }
                        else if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
                            arq.devolverCaracter();
                            return emitir(lexema, "OpBoolE");
                        }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // F => I => M  (FIM)
                    case 'F':
                        if (ch == 'I') { lexema += ch; estado = 68; }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    // O => U  (OU)
                    case 'O':
                        if (ch == 'U') {
                            lexema += ch;
                            return emitir(lexema, "OpBoolOu");
                        }
                        else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                        break;

                    default:
                        if (isalpha(ch) || isdigit(ch) || ch == '_') {
                            lexema += ch;
                        } else {
                            arq.devolverCaracter();
                            return palavraInvalida();
                        }
                }
                continue;
            }

            // D => [E ja consumido no dispatch] => C
            // Estado 42: leu 'DE', espera 'C'
            if (estado == 42) {
                if (ch == 'C') { lexema += ch; return emitir(lexema, "PCDec"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // P => [R ja consumido] => O => G
            // Estado 44: leu 'PR', espera 'O'
            if (estado == 44) {
                if (ch == 'O') { lexema += ch; estado = 45; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 45: leu 'PRO', espera 'G'
            if (estado == 45) {
                if (ch == 'G') { lexema += ch; return emitir(lexema, "PCProg"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // I => [N ja consumido] => T / I
            // Estado 47: leu 'IN', espera 'T' (INT) ou 'I' (INI)
            if (estado == 47) {
                if (ch == 'T') { lexema += ch; return emitir(lexema, "PCInt"); }
                if (ch == 'I') { lexema += ch; return emitir(lexema, "PCIni"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // I => [M ja consumido] => P => R => I => M => I => R
            // Estado 48: leu 'IM', espera 'P'
            if (estado == 48) {
                if (ch == 'P') { lexema += ch; estado = 49; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 49: leu 'IMP', espera 'R'
            if (estado == 49) {
                if (ch == 'R') { lexema += ch; estado = 50; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 50: leu 'IMPR', espera 'I'
            if (estado == 50) {
                if (ch == 'I') { lexema += ch; estado = 51; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 51: leu 'IMPRI', espera 'M'
            if (estado == 51) {
                if (ch == 'M') { lexema += ch; estado = 52; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 52: leu 'IMPRIM', espera 'I'
            if (estado == 52) {
                if (ch == 'I') { lexema += ch; estado = 53; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 53: leu 'IMPRIMI', espera 'R'
            if (estado == 53) {
                if (ch == 'R') { lexema += ch; return emitir(lexema, "PCImprimir"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // R => [E ja consumido] => A => L
            // Estado 55: leu 'RE', espera 'A'
            if (estado == 55) {
                if (ch == 'A') { lexema += ch; estado = 56; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 56: leu 'REA', espera 'L'
            if (estado == 56) {
                if (ch == 'L') { lexema += ch; return emitir(lexema, "PCReal"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // L => [E ja consumido] => R
            // Estado 58: leu 'LE', espera 'R'
            if (estado == 58) {
                if (ch == 'R') { lexema += ch; return emitir(lexema, "PCLer"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // S => [E ja consumido] => aceita SE  ou  => N => A => O (SENAO)
            // Estado 72: leu 'SE'
            if (estado == 72) {
                if (ch == 'N') { lexema += ch; estado = 60; continue; }
                if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
                    arq.devolverCaracter();
                    return emitir(lexema, "PCSe");
                }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 60: leu 'SEN', espera 'A'
            if (estado == 60) {
                if (ch == 'A') { lexema += ch; estado = 61; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 61: leu 'SENA', espera 'O'
            if (estado == 61) {
                if (ch == 'O') { lexema += ch; return emitir(lexema, "PCSenao"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // E => [N ja consumido] => T => A => O  (ENTAO)
            //                      => Q => T => O  (ENQTO)
            // Estado 63: leu 'EN', espera 'T' ou 'Q'
            if (estado == 63) {
                if (ch == 'T') { lexema += ch; estado = 64; continue; }
                if (ch == 'Q') { lexema += ch; estado = 65; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 64: leu 'ENT', espera 'A'
            if (estado == 64) {
                if (ch == 'A') { lexema += ch; estado = 70; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 70: leu 'ENTA', espera 'O'
            if (estado == 70) {
                if (ch == 'O') { lexema += ch; return emitir(lexema, "PCEntao"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 65: leu 'ENQ', espera 'T'
            if (estado == 65) {
                if (ch == 'T') { lexema += ch; estado = 66; continue; }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // Estado 66: leu 'ENQT', espera 'O'
            if (estado == 66) {
                if (ch == 'O') { lexema += ch; return emitir(lexema, "PCEnqto"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

            // F => [I ja consumido] => M
            // Estado 68: leu 'FI', espera 'M'
            if (estado == 68) {
                if (ch == 'M') { lexema += ch; return emitir(lexema, "PCFim"); }
                arq.devolverCaracter();
                return palavraInvalida();
            }

        } // fim while

        // trata tokens pendentes ao atingir fim do arquivo
        if (estado == 2)  { return emitir("<", "OpRelMenor"); }
        if (estado == 7)  { return emitir(">", "OpRelMaior"); }
        if (estado == 12) { return emitir(":", "Delim"); }
        if (estado == 30) { return emitir(lexema, "NumInt"); }
        if (estado == 31) { return emitir(lexema, "NumReal"); }

        if (estado == 40 && !lexema.empty() && islower(lexema[0])) {
            return emitir(lexema, "Var");
        }

        // 'E' sozinho no EOF
        if (estado == 40 && !lexema.empty() && lexema[0] == 'E') {
            return emitir(lexema, "OpBoolE");
        }

        // 'SE' no EOF
        if (estado == 72) { return emitir(lexema, "PCSe"); }

        if (estado == 5)  { return erro("=", "'=' isolado nao e um token valido"); }
        if (estado == 10) { return erro("!", "'!' isolado nao e um token valido"); }
        if (estado == 20) { return erro("\"" + lexema, "cadeia de caracteres nao fechada"); }
        if (estado >= 40 && !lexema.empty()) {
            return palavraInvalida();
        }

        return nullptr;
    }
};

#endif
