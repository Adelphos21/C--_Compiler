#include<iostream>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"

using namespace std;

// =============================
// Métodos de la clase Parser
// =============================

Parser::Parser(Scanner* sc) : scanner(sc) {
    previous = nullptr;
    current = scanner->nextToken();
    if (current->type == Token::ERR) {
        throw runtime_error("Error léxico");
    }
}

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return current->type == ttype;
}

bool Parser::isType(const std::string& t) {
    return t == "int" || t == "long" || t == "bool";
}


bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp = current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;

        if (check(Token::ERR)) {
            throw runtime_error("Error lexico");
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}


// =============================
// Reglas gramaticales
// =============================

Program* Parser::parseProgram() {
    Program* p = new Program();

    while (check(Token::ID)) {
        match(Token::ID);
        string tipo = previous->text;

        match(Token::ID);
        string nombre = previous->text;

        if(check(Token::LPAREN)){   
            p->fdlist.push_back(parseFunDec(tipo, nombre));
      //} else if(check(Token::COMA) || check(Token::SEMICOL)) {
        } else { 
            p->vdlist.push_back(parseVarDec(tipo, nombre));
        }
    }

    cout << "Parser exitoso" << endl;
    return p;
}

VarDec* Parser::parseVarDec(const std::string& tipo, const std::string& firstVarName){
    VarDec* vd = new VarDec();
    vd->type = tipo;

    //Parsear Arrays
    if (match(Token::LARRAYBRACKET)) {           // '['
        match(Token::NUM);
        int len = stoi(previous->text);
        match(Token::RARRAYBRACKET);            // ']'
        vd->vars.push_back(VarItem(firstVarName,true,len));
    } else {
        vd->vars.push_back(VarItem(firstVarName,false,0));
    }
    
    while(match(Token::COMA)) {
        //Parsear Arrays de nuevo
        match(Token::ID);
        string id = previous->text;
        if (match(Token::LARRAYBRACKET)) {           // '['
            match(Token::NUM);
            int len = stoi(previous->text);
            match(Token::RARRAYBRACKET);            // ']'
            vd->vars.push_back(VarItem(id,true,len));
        } else {
            vd->vars.push_back(VarItem(id,false,0));
        }
    }
    match(Token::SEMICOL);
    return vd;
}

FunDec *Parser::parseFunDec(const std::string& tipo, const std::string& nombre) {
    FunDec* fd = new FunDec();
    fd->tipo   = tipo;
    fd->nombre = nombre;

    match(Token::LPAREN);

    if(check(Token::ID)) {
        while(match(Token::ID)) {
            fd->Ptipos.push_back(previous->text);
            match(Token::ID);
            fd->Pnombres.push_back(previous->text);
            match(Token::COMA);
        }
    }
    match(Token::RPAREN);
    fd->cuerpo = parseBody();
    return fd;
}

Body* Parser::parseBody(){
    Body* b = new Body();

    match(Token::LBRACKET);

    while(check(Token::ID) && isType(current->text)) {
        match(Token::ID);
        std::string tipo = previous->text;

        match(Token::ID);
        std::string nombre = previous->text;

        b->declarations.push_back(parseVarDec(tipo, nombre));
    }

    /*
    if(!check(Token::RBRACKET)){
        b->StmList.push_back(parseStm());
        while(match(Token::SEMICOL)){
            if (check(Token::RBRACKET)) break; // para no exigir ';' justo antes de '}'
            b->StmList.push_back(parseStm());
        }
    }
    */

    if(!check(Token::RBRACKET)){
        while(!check(Token::RBRACKET)){
            match(Token::SEMICOL);
            b->StmList.push_back(parseStm());
            match(Token::SEMICOL);
        }
    }

    match(Token::RBRACKET);
    return b;
}

Stm* Parser::parseStm() {
    Stm* a;
    Exp* e;
    string variable;
    Body* tb = nullptr;
    Body* fb = nullptr;

    if(match(Token::ID)){
        variable = previous->text;

        if (match(Token::LARRAYBRACKET)) {
            Exp* idx = parseCE();
            match(Token::RARRAYBRACKET);
            match(Token::ASSIGN);
            e = parseCE();
            return new ArrayAssignStm(variable, idx, e);
        } else {
            match(Token::ASSIGN);
            e = parseCE();
            return new AssignStm(variable, e);
        }
    }
    else if(match(Token::PRINTF)){
        match(Token::LPAREN);
        e = parseCE();
        match(Token::RPAREN);
        return new PrintStm(e);
    }
    else if(match(Token::RETURN)) {
        ReturnStm* r  = new ReturnStm();
        if (match(Token::LPAREN)) {
            r->e = parseCE();
            match(Token::RPAREN);
        } else {
            r->e = parseCE();
        }

        return r;
    }
else if (match(Token::IF)) {
        //if(e)
        match(Token::LPAREN);
        e = parseCE();
        match(Token::RPAREN);

        tb = parseBody();   // cuerpo para el "then"

        if (match(Token::ELSE)) {
            fb = parseBody();   // cuerpo para el "else"
        }

        a = new IfStm(e, tb, fb);
    }
    else if (match(Token::WHILE)) {
        //while(e)
        match(Token::LPAREN);
        e = parseCE();
        match(Token::RPAREN);

        tb = parseBody();
        a = new WhileStm(e, tb);
    }

    else if (match(Token::FOR)){
        match(Token::LPAREN);

        VarDec* initDec = nullptr;
        AssignStm* initAssign = nullptr;

        // for(int i = 0; ... )  o  for(i = 0; ...)
        if (check(Token::ID) && isType(current->text)){
            
            match(Token::ID);
            string tipo = previous->text;

            match(Token::ID);
            string id = previous->text;

            initDec = new VarDec();
            initDec->type = tipo;
            initDec->vars.push_back(id);

            if (match(Token::ASSIGN)) {
                Exp* initExp = parseCE();
                initAssign = new AssignStm(id, initExp);
            }

            Exp* initExp = nullptr;
            if (match(Token::ASSIGN)) {
                initExp = parseCE();
            }
        } else{
            match(Token::ID);
            string id = previous->text;

            match(Token::ASSIGN);
            Exp* e = parseCE();

            initAssign = new AssignStm(id, e);
        }

        match(Token::SEMICOL);

        // ---- CONDICIÓN ----
        Exp* cond = parseCE();
        match(Token::SEMICOL);

        // ---- STEP ----
        match(Token::ID);
        string id = previous->text;

        match(Token::ASSIGN);
        Exp* e = parseCE();

        AssignStm* step = new AssignStm(id, e);

        match(Token::RPAREN);

        
        // ---- CUERPO ----
        Body* cuerpo = parseBody();

        a = new ForStm(initDec, initAssign, cond, step, cuerpo);
    }
    else{
        throw runtime_error("Error sintáctico");
    }
    return a;
}

Exp* Parser::parseCE() {
    Exp* l = parseBE();
    
    if (match(Token::LE)) {              // <
        BinaryOp op = LE_OP;
        Exp* r = parseBE();
        l = new BinaryExp(l, r, op);
    }
    else if (match(Token::LEQ)) {        // <=
        BinaryOp op = LEQ_OP;
        Exp* r = parseBE();
        l = new BinaryExp(l, r, op);
    }
    else if (match(Token::GE)) {         // >
        BinaryOp op = GE_OP;
        Exp* r = parseBE();
        l = new BinaryExp(l, r, op);
    }
    else if (match(Token::GEQ)) {        // >=
        BinaryOp op = GEQ_OP;
        Exp* r = parseBE();
        l = new BinaryExp(l, r, op);
    }
    return l;
}


Exp* Parser::parseBE() {
    Exp* l = parseE();
    while (match(Token::PLUS) || match(Token::MINUS)) {
        BinaryOp op;
        if (previous->type == Token::PLUS){
            op = PLUS_OP;
        }
        else{
            op = MINUS_OP;
        }
        Exp* r = parseE();
        l = new BinaryExp(l, r, op);
    }
    return l;
}


Exp* Parser::parseE() {
    Exp* l = parseT();
    while (match(Token::MUL) || match(Token::DIV)) {
        BinaryOp op;
        if (previous->type == Token::MUL){
            op = MUL_OP;
        }
        else{
            op = DIV_OP;
        }
        Exp* r = parseT();
        l = new BinaryExp(l, r, op);
    }
    return l;
}


Exp* Parser::parseT() {
    Exp* l = parseF();
    if (match(Token::POW)) {
        BinaryOp op = POW_OP;
        Exp* r = parseF();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

Exp* Parser::parseF() {
    Exp* e;
    string nom;
    if (match(Token::NUM)) {
        return new NumberExp(stoi(previous->text));
    }
    else if (match(Token::TRUE)) {
        return new NumberExp(1);
    }
    else if (match(Token::FALSE)) {
        return new NumberExp(0);
    }
    else if (match(Token::LPAREN))
    {
        e = parseCE();
        match(Token::RPAREN);
        return e;
    }
    else if (match(Token::ID)) {
        nom = previous->text;
        if(check(Token::LARRAYBRACKET)) {
            match(Token::LARRAYBRACKET);
            Exp* idx = parseCE();
            match(Token::RARRAYBRACKET);
            return new ArrayAccessExp(nom, idx);
        }
        if(check(Token::LPAREN)) {
            match(Token::LPAREN);

            FcallExp* fcall = new FcallExp();
            fcall->nombre = nom;

            if (!check(Token::RPAREN)) {
                fcall->argumentos.push_back(parseCE());
                while(match(Token::COMA)) {
                    fcall->argumentos.push_back(parseCE());
                }
            }
            match(Token::RPAREN);
            return fcall;
        }
        else {
            return new IdExp(nom);
            }
    }
    else if(match(Token::STRING)) {
        return new StringExp(previous->text);   
    } else {
        throw runtime_error("Error sintáctico");
    }
}
