#ifndef SINTATICO_H
#define SINTATICO_H

#include <iostream>
#include <memory>
#include <string>
#include "Lexico.h"

class Sintatico {
public:
    explicit Sintatico(Lexico& lexico) : lexico(lexico) {}

    bool analisar() {
        avancar();
        try {
            programa();
        } catch (const FalhaSintatica&) {
            // Se a estrutura do programa se perdeu, ainda relata erros léxicos até o EOF.
            while (atual) {
                avancar();
            }
        }
        return valido;
    }

private:
    struct FalhaSintatica {};
    enum class Forma { Aritmetica, Condicao };
    enum Precedencia { SemOperador, Logica, Relacional, Aditiva, Multiplicativa };

    Lexico& lexico;
    std::unique_ptr<Token> atual;
    bool valido = true;

    bool eh(const char* tipo) const {
        return atual && atual->tipo == tipo;
    }

    void avancar() {
        if (eh("ErroLexico")) {
            diagnosticar("Erro lexico", atual->erro);
        }
        atual.reset(lexico.proximoToken());
    }

    bool aceitar(const char* tipo) {
        if (!eh(tipo)) {
            return false;
        }
        avancar();
        return true;
    }

    void exigir(const char* tipo, const std::string& descricao) {
        if (!aceitar(tipo)) {
            falhar(descricao);
        }
    }

    void diagnosticar(const char* categoria, const std::string& mensagem) {
        valido = false;
        const int linha = atual ? atual->linha : lexico.numeroLinha();
        const int coluna = atual ? atual->coluna : lexico.numeroColuna();
        std::cerr << categoria << " (linha " << linha << ", coluna " << coluna
                  << "): " << mensagem << '\n';
    }

    void diagnosticarEsperado(const std::string& esperado) {
        const std::string encontrado = atual
            ? atual->tipo + " ('" + atual->lexema + "')"
            : "fim de arquivo";
        diagnosticar("Erro sintatico", "esperado " + esperado + " antes de " + encontrado);
    }

    [[noreturn]] void falhar(const std::string& esperado) {
        if (eh("ErroLexico")) {
            avancar();
        } else {
            diagnosticarEsperado(esperado);
        }
        throw FalhaSintatica{};
    }

    void programa() {
        exigir("Delim", "delimitador ':'");
        exigir("PCDec", "'DEC'");
        listaDeclaracoes();
        exigir("Delim", "delimitador ':'");
        exigir("PCProg", "'PROG'");
        listaComandos(false);
    }

    bool fimDasDeclaracoes() const {
        return !atual || eh("Delim") || eh("PCProg");
    }

    void listaDeclaracoes() {
        if (fimDasDeclaracoes()) {
            diagnosticarEsperado("ao menos uma declaração");
        }
        while (!fimDasDeclaracoes()) {
            try {
                declaracao();
            } catch (const FalhaSintatica&) {
                while (!fimDasDeclaracoes() && !eh("Var")) {
                    avancar();
                }
            }
        }
    }

    void declaracao() {
        exigir("Var", "variável");
        if (eh("PCInt")) {
            // O tipo já está na posição correta. Recupera o ':' ausente por inserção.
            diagnosticarEsperado("delimitador ':'");
        } else {
            exigir("Delim", "delimitador ':'");
        }
        exigir("PCInt", "'INT'");
    }

    bool inicioDeComando() const {
        return eh("Var") || eh("PCLer") || eh("PCImprimir") || eh("PCSe")
            || eh("PCEnqto") || eh("PCIni");
    }

    bool fimDaListaDeComandos(bool dentroDeBloco) const {
        return !atual || (dentroDeBloco && (eh("PCFim") || eh("PCSenao")));
    }

    void listaComandos(bool dentroDeBloco) {
        if (fimDaListaDeComandos(dentroDeBloco)) {
            diagnosticarEsperado("ao menos um comando");
        }
        while (!fimDaListaDeComandos(dentroDeBloco)) {
            try {
                comando();
            } catch (const FalhaSintatica&) {
                while (!fimDaListaDeComandos(dentroDeBloco) && !inicioDeComando()) {
                    avancar();
                }
            }
        }
    }

    bool igualIsolado() const {
        return eh("ErroLexico") && atual->lexema == "=";
    }

    void comando() {
        if (aceitar("Var")) {
            if (igualIsolado()) {
                avancar(); // Recupera como se fosse ':=', preservando o erro léxico.
            } else {
                exigir("Atrib", "atribuição ':='");
            }
            expressao(Aditiva, Aditiva);
        } else if (aceitar("PCLer")) {
            exigir("Var", "variável depois de 'LER'");
        } else if (aceitar("PCImprimir")) {
            if (!aceitar("Var") && !aceitar("Cadeia")) {
                falhar("variável ou cadeia de caracteres depois de 'IMPRIMIR'");
            }
        } else if (aceitar("PCSe")) {
            condicao();
            exigir("PCEntao", "'ENTAO'");
            comando();
            if (aceitar("PCSenao")) {
                comando();
            }
        } else if (aceitar("PCEnqto")) {
            condicao();
            bloco();
        } else if (eh("PCIni")) {
            bloco();
        } else {
            falhar("comando");
        }
    }

    void bloco() {
        exigir("PCIni", "'INI'");
        listaComandos(true);
        exigir("PCFim", "'FIM'");
    }

    void condicao() {
        if (expressao(Logica, Logica) != Forma::Condicao) {
            falhar("operador relacional");
        }
    }

    Precedencia precedencia(int nivelDoGrupo) const {
        if (eh("OpAritMult") || eh("OpAritDiv")) {
            return Multiplicativa;
        }
        if (eh("OpAritSoma") || eh("OpAritSub")) {
            return Aditiva;
        }
        if ((atual && atual->tipo.compare(0, 5, "OpRel") == 0)
            || (nivelDoGrupo == Logica && igualIsolado())) {
            return Relacional;
        }
        if (eh("OpBoolE") || eh("OpBoolOu")) {
            return Logica;
        }
        return SemOperador;
    }

    Forma expressao(int minimo, int nivelDoGrupo) {
        Forma esquerda = fator(nivelDoGrupo);
        for (Precedencia nivel = precedencia(nivelDoGrupo); nivel >= minimo;
             nivel = precedencia(nivelDoGrupo)) {
            const Forma operando = nivel == Logica ? Forma::Condicao : Forma::Aritmetica;
            exigirForma(esquerda, operando);
            avancar();

            // Operadores mais fortes entram no operando direito. Os do mesmo nível
            // ficam para a repetição deste laço, o que dá associatividade à esquerda.
            const Forma direita = expressao(nivel + 1, nivelDoGrupo);
            exigirForma(direita, operando);
            esquerda = nivel <= Relacional ? Forma::Condicao : Forma::Aritmetica;
        }
        return esquerda;
    }

    void exigirForma(Forma encontrada, Forma esperada) {
        if (encontrada != esperada) {
            falhar(esperada == Forma::Condicao ? "condição relacional" : "expressão aritmética");
        }
    }

    Forma fator(int nivelDoGrupo) {
        if (aceitar("Var") || aceitar("NumInt")) {
            return Forma::Aritmetica;
        }
        if (aceitar("AbrePar")) {
            const Forma forma = expressao(nivelDoGrupo, nivelDoGrupo);
            exigir("FechaPar", "')'");
            return forma;
        }
        falhar("variável, número inteiro ou expressão entre parênteses");
    }
};

#endif
