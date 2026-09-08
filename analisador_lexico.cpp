#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <stdexcept>

// Enums para todas as siglas da tabela GYH
enum class TokenType {
    // Palavras-Chave
    PCDec, PCProg, PCInt, PCReal, PCLer, PCImprimir, 
    PCSe, PCSenao, PCEntao, PCEnqto, PCIni, PCFim,
    // Operadores Booleanos
    OpBoolE, OpBoolOu,
    // Operadores Aritméticos
    OpAritMult, OpAritDiv, OpAritSoma, OpAritSub,
    // Operadores Relacionais
    OpRelMenor, OpRelMenorIgual, OpRelMaior, OpRelMaiorIgual, OpRelIgual, OpRelDif,
    // Delimitador, Atribuição e Parênteses
    Delim, Atrib, AbrePar, FechaPar,
    // Literais e Identificadores
    Var, NumInt, NumReal, Cadeia,
    // Fim de Arquivo
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
};

// Função para exibir o nome exato da sigla
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::PCDec: return "PCDec";
        case TokenType::PCProg: return "PCProg";
        case TokenType::PCInt: return "PCInt";
        case TokenType::PCReal: return "PCReal";
        case TokenType::PCLer: return "PCLer";
        case TokenType::PCImprimir: return "PCImprimir";
        case TokenType::PCSe: return "PCSe";
        case TokenType::PCSenao: return "PCSenao";
        case TokenType::PCEntao: return "PCEntao";
        case TokenType::PCEnqto: return "PCEnqto";
        case TokenType::PCIni: return "PCIni";
        case TokenType::PCFim: return "PCFim";
        case TokenType::OpBoolE: return "OpBoolE";
        case TokenType::OpBoolOu: return "OpBoolOu";
        case TokenType::OpAritMult: return "OpAritMult";
        case TokenType::OpAritDiv: return "OpAritDiv";
        case TokenType::OpAritSoma: return "OpAritSoma";
        case TokenType::OpAritSub: return "OpAritSub";
        case TokenType::OpRelMenor: return "OpRelMenor";
        case TokenType::OpRelMenorIgual: return "OpRelMenorIgual";
        case TokenType::OpRelMaior: return "OpRelMaior";
        case TokenType::OpRelMaiorIgual: return "OpRelMaiorIgual";
        case TokenType::OpRelIgual: return "OpRelIgual";
        case TokenType::OpRelDif: return "OpRelDif";
        case TokenType::Delim: return "Delim";
        case TokenType::Atrib: return "Atrib";
        case TokenType::AbrePar: return "AbrePar";
        case TokenType::FechaPar: return "FechaPar";
        case TokenType::Var: return "Var";
        case TokenType::NumInt: return "NumInt";
        case TokenType::NumReal: return "NumReal";
        case TokenType::Cadeia: return "Cadeia";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "UNKNOWN";
}

class Lexer {
private:
    std::string src;
    size_t pos;
    int line;
    int col;

    // Tabela Hash para Palavras-Chave e OpBool
    const std::unordered_map<std::string, TokenType> keywords = {
        {"DEC", TokenType::PCDec},
        {"PROG", TokenType::PCProg},
        {"INT", TokenType::PCInt},
        {"REAL", TokenType::PCReal},
        {"LER", TokenType::PCLer},
        {"IMPRIMIR", TokenType::PCImprimir},
        {"SE", TokenType::PCSe},
        {"SENAO", TokenType::PCSenao},
        {"ENTAO", TokenType::PCEntao},
        {"ENQTO", TokenType::PCEnqto},
        {"INI", TokenType::PCIni},
        {"FIM", TokenType::PCFim},
        {"E", TokenType::OpBoolE},
        {"OU", TokenType::OpBoolOu}
    };

    char peek() const {
        if (pos < src.length()) return src[pos];
        return '\0';
    }

    char advance() {
        if (pos < src.length()) {
            char c = src[pos++];
            if (c == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            return c;
        }
        return '\0';
    }

    void skipWhitespace() {
        while (pos < src.length() && std::isspace(peek())) {
            advance();
        }
    }

public:
    Lexer(const std::string& input) : src(input), pos(0), line(1), col(1) {}

    Token nextToken() {
        skipWhitespace();

        if (pos >= src.length()) {
            return {TokenType::END_OF_FILE, "", line, col};
        }

        int startLine = line;
        int startCol = col;
        char c = peek();

        // 1. Variável (começa com letra minúscula)
        if (std::islower(c)) {
            std::string lexeme = "";
            while (pos < src.length() && (std::isalnum(peek()) || peek() == '_')) {
                lexeme += advance();
            }
            return {TokenType::Var, lexeme, startLine, startCol};
        }

        // 2. Palavras-Chave e Operadores Booleanos (MAIÚSCULAS)
        if (std::isupper(c)) {
            std::string lexeme = "";
            while (pos < src.length() && std::isupper(peek())) {
                lexeme += advance();
            }
            auto it = keywords.find(lexeme);
            if (it != keywords.end()) {
                return {it->second, lexeme, startLine, startCol};
            } else {
                throw std::runtime_error("Erro Léxico: Identificador em maiúsculas não reconhecido '" + lexeme + "' L:" + std::to_string(startLine) + " C:" + std::to_string(startCol));
            }
        }

        // 3. Números Inteiros (NumInt) e Reais (NumReal)
        if (std::isdigit(c)) {
            std::string lexeme = "";
            bool isReal = false;
            while (pos < src.length() && (std::isdigit(peek()) || peek() == '.')) {
                if (peek() == '.') {
                    if (isReal) break;
                    isReal = true;
                }
                lexeme += advance();
            }
            return {isReal ? TokenType::NumReal : TokenType::NumInt, lexeme, startLine, startCol};
        }

        // 4. Cadeia de caracteres (aspas duplas)
        if (c == '"') {
            std::string lexeme = "";
            lexeme += advance(); // aspas abertura
            while (pos < src.length() && peek() != '"') {
                if (peek() == '\n') {
                    throw std::runtime_error("Erro Léxico: Cadeia não fechada antes da quebra de linha L:" + std::to_string(startLine));
                }
                lexeme += advance();
            }
            if (peek() == '"') {
                lexeme += advance(); // aspas fechamento
                return {TokenType::Cadeia, lexeme, startLine, startCol};
            } else {
                throw std::runtime_error("Erro Léxico: Cadeia não fechada no fim do arquivo L:" + std::to_string(startLine));
            }
        }

        // 5. Delimitador, Atribuição, Operadores e Parênteses
        advance();
        std::string lexeme(1, c);

        switch (c) {
            case ':':
                if (peek() == '=') {
                    lexeme += advance();
                    return {TokenType::Atrib, lexeme, startLine, startCol};
                }
                return {TokenType::Delim, lexeme, startLine, startCol};

            case '<':
                if (peek() == '=') {
                    lexeme += advance();
                    return {TokenType::OpRelMenorIgual, lexeme, startLine, startCol};
                }
                return {TokenType::OpRelMenor, lexeme, startLine, startCol};

            case '>':
                if (peek() == '=') {
                    lexeme += advance();
                    return {TokenType::OpRelMaiorIgual, lexeme, startLine, startCol};
                }
                return {TokenType::OpRelMaior, lexeme, startLine, startCol};

            case '=':
                if (peek() == '=') {
                    lexeme += advance();
                    return {TokenType::OpRelIgual, lexeme, startLine, startCol};
                }
                throw std::runtime_error("Erro Léxico: Símbolo '=' isolado inválido L:" + std::to_string(startLine) + " C:" + std::to_string(startCol));

            case '!':
                if (peek() == '=') {
                    lexeme += advance();
                    return {TokenType::OpRelDif, lexeme, startLine, startCol};
                }
                throw std::runtime_error("Erro Léxico: Símbolo '!' isolado inválido L:" + std::to_string(startLine) + " C:" + std::to_string(startCol));

            case '*': return {TokenType::OpAritMult, lexeme, startLine, startCol};
            case '/': return {TokenType::OpAritDiv, lexeme, startLine, startCol};
            case '+': return {TokenType::OpAritSoma, lexeme, startLine, startCol};
            case '-': return {TokenType::OpAritSub, lexeme, startLine, startCol};
            case '(': return {TokenType::AbrePar, lexeme, startLine, startCol};
            case ')': return {TokenType::FechaPar, lexeme, startLine, startCol};

            default:
                throw std::runtime_error("Erro Léxico: Caractere não reconhecido '" + lexeme + "' L:" + std::to_string(startLine) + " C:" + std::to_string(startCol));
        }
    }
};

int main() {
    std::string programaGYH = R"(
PROG teste
DEC
  fator : INT
  calculo : REAL
INI
  fator := 12
  calculo := 45.75
  SE fator <= 20 E calculo != 0.0 ENTAO
    IMPRIMIR "Resultado OK"
  SENAO
    IMPRIMIR "Erro"
  FIM
FIM
)";

    try {
        Lexer lexer(programaGYH);
        Token t;
        std::cout << "--- TABELA DE TOKENS GERADA ---\n";
        do {
            t = lexer.nextToken();
            if (t.type != TokenType::END_OF_FILE) {
                std::cout << "Sigla: " << tokenTypeToString(t.type)
                          << "\t | Lexema: \"" << t.lexeme << "\"\n";
            }
        } while (t.type != TokenType::END_OF_FILE);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}