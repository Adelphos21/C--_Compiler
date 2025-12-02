#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <ostream>

using namespace std;

class Token {
public:
    // Tipos de token
    enum Type {
        PLUS,    // +
        MINUS,   // -
        MUL,     // *
        DIV,     // /
        POW,     // **
        LBRACKET,  // {
        RBRACKET,  // }
        LARRAYBRACKET, // [
        RARRAYBRACKET, // ] 
        LPAREN,  // (
        RPAREN,  // )
        SQRT,    // sqrt
        NUM,     // Número
        ERR,     // Error
        ID,      // ID
        LE,      // <
        LEQ,     // <=
        GE,      // >
        GEQ,     // >=
        FUN,
        ENDFUN,
        RETURN,
        SEMICOL, // ;    
        ASSIGN,
        //PRINT,
        PRINTF,
        IF,
        WHILE,
        FOR,
        DO,
        THEN,
        ENDIF,
        ENDWHILE,
        ELSE,
        END,      // Fin de entrada
        //VAR,
        COMA,
        TERNARY,
        COLON,
        TRUE,
        FALSE,
        STRING,   // string literal
        STRUCT,
        TYPEDEF,
        DOT
    };

    // Atributos
    Type type;
    string text;

    // Constructores
    Token(Type type);
    Token(Type type, char c);
    Token(Type type, const string& source, int first, int last);

    // Sobrecarga de operadores de salida
    friend ostream& operator<<(ostream& outs, const Token& tok);
    friend ostream& operator<<(ostream& outs, const Token* tok);
};

#endif // TOKEN_H