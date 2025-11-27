#include <iostream>
#include "token.h"

using namespace std;

// -----------------------------
// Constructores
// -----------------------------

Token::Token(Type type) 
    : type(type), text("") { }

Token::Token(Type type, char c) 
    : type(type), text(string(1, c)) { }

Token::Token(Type type, const string& source, int first, int last) 
    : type(type), text(source.substr(first, last)) { }

// -----------------------------
// Sobrecarga de operador <<
// -----------------------------

// Para Token por referencia
ostream& operator<<(ostream& outs, const Token& tok) {
    switch (tok.type) {
        case Token::PLUS:   outs << "TOKEN(PLUS, \""   << tok.text << "\")"; break;
        case Token::MINUS:  outs << "TOKEN(MINUS, \""  << tok.text << "\")"; break;
        case Token::MUL:    outs << "TOKEN(MUL, \""    << tok.text << "\")"; break;
        case Token::DIV:    outs << "TOKEN(DIV, \""    << tok.text << "\")"; break;
        case Token::LPAREN:    outs << "TOKEN(LPAREN, \""    << tok.text << "\")"; break;
        case Token::RPAREN:    outs << "TOKEN(RPAREN, \""    << tok.text << "\")"; break;
        case Token::LBRACKET:    outs << "TOKEN(LBRACKET, \""    << tok.text << "\")"; break;
        case Token::RBRACKET:    outs << "TOKEN(RBRACKET, \""    << tok.text << "\")"; break;
        case Token::LARRAYBRACKET:    outs << "TOKEN(LARRAYBRACKET, \""    << tok.text << "\")"; break;
        case Token::RARRAYBRACKET:    outs << "TOKEN(RARRAYBRACKET, \""    << tok.text << "\")"; break;
        case Token::POW:    outs << "TOKEN(POW, \""    << tok.text << "\")"; break;
        case Token::SQRT:    outs << "TOKEN(SQRT, \""    << tok.text << "\")"; break;
        case Token::ID:    outs << "TOKEN(ID, \""    << tok.text << "\")"; break;
        case Token::NUM:    outs << "TOKEN(NUM, \""    << tok.text << "\")"; break;
        case Token::ERR:    outs << "TOKEN(ERR, \""    << tok.text << "\")"; break;
        case Token::LE:    outs << "TOKEN(LE, \""    << tok.text << "\")"; break;
        case Token::LEQ:    outs << "TOKEN(LEQ, \""    << tok.text << "\")"; break;
        case Token::GE:    outs << "TOKEN(GE, \""    << tok.text << "\")"; break;
        case Token::GEQ:    outs << "TOKEN(GEQ, \""    << tok.text << "\")"; break;
        //case Token::PRINT:    outs << "TOKEN(PRINT, \""    << tok.text << "\")"; break;
        case Token::PRINTF:    outs << "TOKEN(PRINTF, \""    << tok.text << "\")"; break;
        case Token::SEMICOL:    outs << "TOKEN(SEMICOL, \""    << tok.text << "\")"; break;
        case Token::ASSIGN:    outs << "TOKEN(ASSIGN, \""    << tok.text << "\")"; break;
        case Token::IF:    outs << "TOKEN(IF, \""    << tok.text << "\")"; break;
        case Token::WHILE:    outs << "TOKEN(WHILE, \""    << tok.text << "\")"; break;
        case Token::FOR:    outs << "TOKEN(WHILE, \""    << tok.text << "\")"; break;
        case Token::THEN:    outs << "TOKEN(THEN, \""    << tok.text << "\")"; break;
        case Token::DO:    outs << "TOKEN(DO, \""    << tok.text << "\")"; break;
        case Token::ENDIF:    outs << "TOKEN(ENDIF, \""    << tok.text << "\")"; break;
        case Token::ENDWHILE:    outs << "TOKEN(ENDWHILE, \""    << tok.text << "\")"; break;
        case Token::ELSE:    outs << "TOKEN(ELSE, \""    << tok.text << "\")"; break;
        //case Token::VAR:    outs << "TOKEN(VAR, \""    << tok.text << "\")"; break;
        case Token::COMA:    outs << "TOKEN(COMA, \""    << tok.text << "\")"; break;
        case Token::TERNARY:    outs << "TOKEN(TERNARY, \""    << tok.text << "\")"; break;
        case Token::COLON:    outs << "TOKEN(COLON, \""    << tok.text << "\")"; break;
        case Token::FUN:    outs << "TOKEN(FUN, \""    << tok.text << "\")"; break;
        case Token::ENDFUN:    outs << "TOKEN(ENDFUN, \""    << tok.text << "\")"; break;
        case Token::RETURN:    outs << "TOKEN(RETURN, \""    << tok.text << "\")"; break;

        case Token::TRUE:    outs << "TOKEN(TRUE)"; break;
        case Token::FALSE:    outs  << "TOKEN(FALSE)"; break;
        case Token::STRING:    outs << "TOKEN(STRING, \"" << tok.text << "\")"; break;

        case Token::END:    outs << "TOKEN(END)"; break;
    }
    return outs;
}

// Para Token puntero
ostream& operator<<(ostream& outs, const Token* tok) {
    if (!tok) return outs << "TOKEN(NULL)";
    return outs << *tok;  // delega al otro
}