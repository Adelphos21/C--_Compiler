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
    knownTypes.insert("int");
    knownTypes.insert("long");
    knownTypes.insert("bool");
    knownTypes.insert("char");
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

bool Parser::isType(const string& t){
    return knownTypes.count(t) > 0;
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

    while (!isAtEnd()) {
        if (check(Token::STRUCT)) {
            p->sdlist.push_back(parseStructDec());
        }
        else if (check(Token::TYPEDEF)) {
            p->tdlist.push_back(parseTypedefDec());
        }
        else if (check(Token::ID)) {
            parseTopDeclaration(p); // tu lógica original var/fun
        }
        else {
            throw runtime_error("Declaración de nivel superior inválida");
        }
    }
    return p;
}

void Parser::parseTopDeclaration(Program* p){
    match(Token::ID);
    string tipo = previous->text;

    match(Token::ID);
    string nombre = previous->text;

    if (check(Token::LPAREN)) {
        p->fdlist.push_back(parseFunDec(tipo, nombre));
    } else {
        p->vdlist.push_back(parseVarDec(tipo, nombre));
    }
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

StructDec* Parser::parseStructDec(){
    match(Token::STRUCT);

    match(Token::ID);
    string structName = previous->text;

    match(Token::LBRACKET); // '{'

    list<VarDec*> fields;

    while (check(Token::ID) && isType(current->text)) {
        match(Token::ID);
        string fieldType = previous->text;

        match(Token::ID);
        string fieldName = previous->text;

        fields.push_back(parseVarDec(fieldType, fieldName));
        // parseVarDec ya consume ';'
    }

    match(Token::RBRACKET); // '}'
    match(Token::SEMICOL);  // ';' final de struct

    // registrar struct como tipo válido para futuras declaraciones
    knownTypes.insert(structName);

    return new StructDec(structName, fields);
}

TypedefDec* Parser::parseTypedefDec(){
    match(Token::TYPEDEF);

    // tipo original (puede ser "struct" ID o tipo simple/alias)
    string original;

    if (match(Token::STRUCT)) {
        match(Token::ID);
        original = "struct " + previous->text;
    } else {
        match(Token::ID);
        original = previous->text;
        if (!isType(original))
            throw runtime_error("typedef a tipo desconocido: " + original);
    }

    match(Token::ID);
    string alias = previous->text;

    match(Token::SEMICOL);

    knownTypes.insert(alias);

    return new TypedefDec(original, alias);
}


Body* Parser::parseBody(){
    Body* b = new Body();

    match(Token::LBRACKET);

    while((check(Token::ID) && isType(current->text)) || match(Token::STRUCT)) {
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
        Exp* base = new IdExp(previous->text);
        if (match(Token::DOT)) {
            match(Token::ID);
            string field = previous->text;
            match(Token::ASSIGN);
            Exp* rhs = parseCE();
            return new FieldAssignStm(base, field, rhs);
        }    

        
        if (match(Token::LARRAYBRACKET)) {
            Exp* idx = parseCE();
            match(Token::RARRAYBRACKET);
            match(Token::ASSIGN);
            e = parseCE();
            return new ArrayAssignStm(variable, idx, e);
        }
        match(Token::ASSIGN);
        Exp* rhs = parseCE();
        return new AssignStm(dynamic_cast<IdExp*>(base)->value, rhs);

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

    while (check(Token::LE) || check(Token::LEQ) ||
           check(Token::GE) || check(Token::GEQ)) {

        BinaryOp op;
        if (match(Token::LE))      op = LE_OP;
        else if (match(Token::LEQ)) op = LEQ_OP;
        else if (match(Token::GE))  op = GE_OP;
        else /* GEQ */              { match(Token::GEQ); op = GEQ_OP; }

        Exp* r = parseBE();
        l = new BinaryExp(l, r, op);
    }

    // 2) Ternario right-associative
    if (match(Token::TERNARY)) {
        Exp* trueExpr = parseCE();
        if (!match(Token::COLON))
            throw runtime_error("Se esperaba ':' en operador ternario");

        Exp* falseExpr = parseCE();
        l = new TernaryExp(l, trueExpr, falseExpr);
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
    Exp* e = nullptr;

    // -------- Primary --------
    if (match(Token::NUM)) {
        e = new NumberExp(stoi(previous->text));
    }
    else if (match(Token::TRUE)) {
        e = new NumberExp(1);
    }
    else if (match(Token::FALSE)) {
        e = new NumberExp(0);
    }

    else if (match(Token::STRING)) {
        e = new StringExp(previous->text);  // sin comillas
    }
    else if (match(Token::LPAREN)) {
        e = parseCE();
        if (!match(Token::RPAREN))
            throw runtime_error("Se esperaba ')'");
    }
    else if (match(Token::ID)) {
        e = new IdExp(previous->text);
    }
    else {
        throw runtime_error("Error sintáctico en factor");
    }

    // -------- Postfix repetidos --------
    while (true) {

        // Acceso a array: e[expr]
        if (match(Token::LARRAYBRACKET)) {
            Exp* idx = parseCE();
            if (!match(Token::RARRAYBRACKET))
                throw runtime_error("Se esperaba ']'");
            
            // e debe ser IdExp o ArrayAccess previo
            // Para mantener tu AST simple, solo soportamos id[...]
            // Si quieres full C, necesitarías otro nodo Postfix.
            IdExp* ide = dynamic_cast<IdExp*>(e);
            if (!ide)
                throw runtime_error("Acceso a array requiere identificador");

            e = new ArrayAccessExp(ide->value, idx);
            delete ide;
            continue;
        }

        // Llamada a función: e(...)
        if (match(Token::LPAREN)) {
            // de nuevo, por simplicidad: solo id(...)
            IdExp* ide = dynamic_cast<IdExp*>(e);
            if (!ide)
                throw runtime_error("Llamada requiere identificador");

            FcallExp* fcall = new FcallExp();
            fcall->nombre = ide->value;
            delete ide;

            if (!check(Token::RPAREN)) {
                fcall->argumentos.push_back(parseCE());
                while (match(Token::COMA)) {
                    fcall->argumentos.push_back(parseCE());
                }
            }

            if (!match(Token::RPAREN))
                throw runtime_error("Se esperaba ')' en llamada");

            e = fcall;
            continue;
        }
        if (match(Token::DOT)) {
            match(Token::ID);
            string field = previous->text;
            e = new FieldAccessExp(e, field);
            continue;
        }    

        break;
    }

    return e;
}
