#include "typeChecker.h"
#include <iostream>
#include <stdexcept>
using namespace std;


Type* NumberExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* IdExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BinaryExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FcallExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* BoolExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* TernaryExp::accept(TypeVisitor* v) { return v->visit(this); }
Type* FieldAccessExp::accept(TypeVisitor* v) { return v->visit(this); }


void AssignStm::accept(TypeVisitor* v) { v->visit(this); }
void PrintStm::accept(TypeVisitor* v) { v->visit(this); }
void ReturnStm::accept(TypeVisitor* v) { v->visit(this); }

void IfStm::accept(TypeVisitor* v){ v->visit(this); }
void WhileStm::accept(TypeVisitor* v){ v->visit(this); }
void ForStm::accept(TypeVisitor* v){ v->visit(this); }

Type* ArrayAccessExp::accept(TypeVisitor* v){ return v->visit(this); }
void ArrayAssignStm::accept(TypeVisitor* v){ v->visit(this); }

void FieldAssignStm::accept(TypeVisitor* v) { v->visit(this); }
void StructDec::accept(TypeVisitor* v) { v->visit(this); }
void TypedefDec::accept(TypeVisitor* v) { v->visit(this); }
void VarDec::accept(TypeVisitor* v) { v->visit(this); }
void FunDec::accept(TypeVisitor* v) { v->visit(this); }
void Body::accept(TypeVisitor* v) { v->visit(this); }
void Program::accept(TypeVisitor* v) { v->visit(this); }

Type* StringExp::accept(TypeVisitor* v){ return v->visit(this); }
// ===========================================================
//   Constructor del TypeChecker
// ===========================================================

TypeChecker::TypeChecker() {
    intType = new Type(Type::INT);
    boolType = new Type(Type::BOOL);
    voidType = new Type(Type::VOID);
    charType = new Type(Type::CHAR);
}

// ===========================================================
//   Registrar funciones globales
// ===========================================================

void TypeChecker::add_function(FunDec* fd) {
    if (functions.find(fd->nombre) != functions.end()) {
        cerr << "Error: función '" << fd->nombre << "' ya fue declarada." << endl;
        exit(0);
    }

    Type* returnType = new Type();
    if (!returnType->set_basic_type(fd->tipo)) {
        cerr << "Error: tipo de retorno no válido en función '" << fd->nombre << "'." << endl;
        exit(0);
    }

    functions[fd->nombre] = returnType;
}

// ===========================================================
//   Método principal de verificación
// ===========================================================

void TypeChecker::typecheck(Program* program) {
    if (program) program->accept(this);
    cout << "Revisión exitosa" << endl;
}

// ===========================================================
//   Nivel superior: Programa y Bloque
// ===========================================================

void TypeChecker::visit(Program* p) {
    // 1) registrar structs
    for (auto s: p->sdlist) s->accept(this);

    // 2) registrar typedefs
    for (auto t: p->tdlist) t->accept(this);

    // 3) registrar funciones
    for (auto f : p->fdlist) add_function(f);

    // 4) variables globales y funcs
    env.add_level();
    for (auto v : p->vdlist) v->accept(this);
    for (auto f : p->fdlist) f->accept(this);
    env.remove_level();
}

void TypeChecker::visit(Body* b) {
    env.add_level();
    for (auto v : b->declarations) v->accept(this);
    for (auto s : b->StmList) s->accept(this);
    env.remove_level();
}

// ===========================================================
//   Declaraciones
// ===========================================================

void TypeChecker::visit(VarDec* v) {
    Type* t = new Type();

    if (!t->set_basic_type(v->type)) {
        if (typeAliases.count(v->type)) t = typeAliases[v->type];
        else if (structTypes.count(v->type)) t = structTypes[v->type];
        else {
            cerr << "Error: tipo de variable no válido: " << v->type << endl;
            exit(0);
        }
    }
    Type* resolved = t;
    if (resolved->ttype == Type::TYPEALIAS) resolved = resolved->base;
    bool isStruct = resolved && resolved->ttype == Type::STRUCT;

    for (const auto& var : v->vars) {
        if (env.check(var.name)) {
            cerr << "Error: variable '" << var.name << "' ya declarada." << endl;
            exit(0);
        }
        if (isStruct) {
            varStructType[var.name] = resolved->structName;
        }
        if(!var.isArray){
            env.add_var(var.name, t);
        } else {
            Type* arrT = new Type(Type::ARRAY);
            arrT->base = t;
            arrT->length = var.length;
            env.add_var(var.name, arrT);
        }
    }
}

void TypeChecker::visit(FunDec* f) {
    env.add_level();
    for (size_t i = 0; i < f->Pnombres.size(); ++i) {
        Type* pt = new Type();
        if (!pt->set_basic_type(f->Ptipos[i])) {
            cerr << "Error: tipo de parámetro inválido en función '" << f->nombre << "'." << endl;
            exit(0);
        }
        env.add_var(f->Pnombres[i], pt);
    }
    f->cuerpo->accept(this); 
    env.remove_level();
}

void TypeChecker::visit(StructDec* s){
    if (structTypes.count(s->name)) {
        cerr << "struct ya declarado: " << s->name << endl; exit(0);
    }

    Type* st = new Type(Type::STRUCT);
    st->structName = s->name;

    // scope temporal para campos
    Environment<Type*> fieldEnv;
    fieldEnv.add_level();

    for (auto vd : s->fields){
        // Reutilizamos visit(VarDec) pero NO debe meter campos al env global
        // así que los procesamos manualmente:
        Type* t = new Type();
        if (!t->set_basic_type(vd->type)) {
            // puede ser alias o struct
            if (typeAliases.count(vd->type)) t = typeAliases[vd->type];
            else if (structTypes.count(vd->type)) t = structTypes[vd->type];
            else { cerr << "tipo de campo invalido\n"; exit(0); }
        }

        for (auto &var : vd->vars) {
            if (st->fields.count(var.name)) {
                cerr << "campo duplicado " << var.name << " en struct " << s->name << endl;
                exit(0);
            }

            if(!var.isArray){
                st->fields[var.name] = t;
            } else {
                Type* arrT = new Type(Type::ARRAY);
                arrT->base = t;
                arrT->length = var.length;
                st->fields[var.name] = arrT;
            }
        }
    }

    structTypes[s->name] = st;
}

void TypeChecker::visit(TypedefDec* t){
    if (typeAliases.count(t->alias)) {
        cerr << "alias ya declarado: " << t->alias << endl; exit(0);
    }

    Type* baseT = new Type();
    string orig = t->original;

    if (orig.rfind("struct ",0)==0) {
        string sname = orig.substr(7);
        if (!structTypes.count(sname)) {
            cerr << "typedef a struct desconocido: " << sname << endl; exit(0);
        }
        baseT = structTypes[sname];
    } else {
        if (!baseT->set_basic_type(orig)) {
            if (typeAliases.count(orig)) baseT = typeAliases[orig];
            else if (structTypes.count(orig)) baseT = structTypes[orig];
            else { cerr << "typedef a tipo desconocido: " << orig << endl; exit(0); }
        }
    }

    Type* aliasT = new Type(Type::TYPEALIAS);
    aliasT->aliasName = t->alias;
    aliasT->base = baseT;

    typeAliases[t->alias] = aliasT;
}

Type* TypeChecker::visit(FieldAccessExp* e){
    Type* bt = e->baseExp->accept(this);

    // resolver alias
    if (bt->ttype == Type::TYPEALIAS) bt = bt->base;

    if (bt->ttype != Type::STRUCT) {
        cerr << "acceso a campo sobre no-struct\n"; exit(0);
    }

    if (!bt->fields.count(e->field)) {
        cerr << "campo no existe: " << e->field << endl; exit(0);
    }
    return bt->fields[e->field];
}

void TypeChecker::visit(FieldAssignStm* s){
    Type* lt = (new FieldAccessExp(s->baseExp, s->field))->accept(this);
    Type* rt = s->e->accept(this);

    if (!lt->match(rt)) {
        cerr << "tipos incompatibles en asignacion a campo\n"; exit(0);
    }
}


// ===========================================================
//   Sentencias
// ===========================================================

void TypeChecker::visit(PrintStm* stm) {
    Type* t = stm->e->accept(this);

    bool ok_basic = t->match(intType) || t->match(boolType);

    bool ok_string =
        (t->ttype == Type::ARRAY &&
         t->base && t->base->ttype == Type::CHAR);


    if (!(ok_basic || ok_string )) {
        cerr << "Error: tipo inválido en print.\n";
        exit(0);
    }
}

void TypeChecker::visit(AssignStm* stm) {
    if (!env.check(stm->id)) {
        cerr << "Error: variable '" << stm->id << "' no declarada." << endl;
        exit(0);
    }

    Type* varType = env.lookup(stm->id);
    Type* expType = stm->e->accept(this);

    if (!varType->match(expType)) {
        cerr << "Error: tipos incompatibles en asignación a '" << stm->id << "'." << endl;
        exit(0);
    }
}

void TypeChecker::visit(ReturnStm* stm) {
    if (stm->e) {
        Type* t = stm->e->accept(this);
        if (!(t->match(intType) || t->match(boolType) || t->match(voidType))) {
            cerr << "Error: tipo inválido en return." << endl;
            exit(0);
        }
    }
}

void TypeChecker::visit(ForStm* stm) {
    env.add_level(); // scope del for

    if (stm->initDec) stm->initDec->accept(this);
    if (stm->initAssign) stm->initAssign->accept(this);

    Type* ct = stm->condition->accept(this);
    if (!(ct->match(boolType) || ct->match(intType))) {
        cerr << "condición de for debe ser bool/int" << endl;
        exit(0);
    }

    stm->cuerpo->accept(this);
    stm->step->accept(this);

    env.remove_level();
}

void TypeChecker::visit(IfStm* stm){
    Type* ct = stm->condition->accept(this);
    if(!(ct->match(boolType) || ct->match(intType))){
        cerr << "condición de if debe ser bool/int\n";
        exit(0);
    }
    stm->then->accept(this);
    if(stm->els) stm->els->accept(this);
}

void TypeChecker::visit(WhileStm* stm){
    Type* ct = stm->condition->accept(this);
    if(!(ct->match(boolType) || ct->match(intType))){
        cerr << "condición de while debe ser bool/int\n";
        exit(0);
    }
    stm->b->accept(this);
}

void TypeChecker::visit(ExprStm* stm){
    stm->e->accept(this);
}



// ===========================================================
//   Expresiones
// ===========================================================

Type* TypeChecker::visit(BinaryExp* e) {
    Type* left = e->left->accept(this);
    Type* right = e->right->accept(this);

    switch (e->op) {
        case PLUS_OP: 
        case MINUS_OP: 
        case MUL_OP: 
        case DIV_OP: 
        case POW_OP:
            if (!(left->match(intType) && right->match(intType))) {
                cerr << "Error: operación aritmética requiere operandos int." << endl;
                exit(0);
            }
            return intType;
        case LE_OP:
        case LEQ_OP:
        case GE_OP:
        case GEQ_OP:
            if (!(left->match(intType) && right->match(intType))) {
                cerr << "Error: operación aritmética requiere operandos int." << endl;
                exit(0);
            }
        return boolType;
        default:
            cerr << "Error: operador binario no soportado." << endl;
            exit(0);
    }
}

Type* TypeChecker::visit(TernaryExp* e) {
    Type* condType = e->condition->accept(this);
    
    // La condición debe ser booleana o entera
    if (!(condType->match(boolType) || condType->match(intType))) {
        cerr << "Error: condición en operador ternario debe ser bool o int." << endl;
        exit(0);
    }
    
    Type* trueType = e->trueExpr->accept(this);
    Type* falseType = e->falseExpr->accept(this);
    
    // Ambos tipos deben ser compatibles
    if (!trueType->match(falseType)) {
        cerr << "Error: tipos incompatibles en ramas del operador ternario." << endl;
        exit(0);
    }
    
    return trueType;
}

Type* TypeChecker::visit(NumberExp* e) { return intType; }

Type* TypeChecker::visit(BoolExp* e) { return boolType; }

Type* TypeChecker::visit(IdExp* e) {
    if (!env.check(e->value)) {
        cerr << "Error: variable '" << e->value << "' no declarada." << endl;
        exit(0);
    }
    return env.lookup(e->value);
}

Type* TypeChecker::visit(FcallExp* e) {
    auto it = functions.find(e->nombre);
    if (it == functions.end()) {
        cerr << "Error: llamada a función no declarada '" << e->nombre << "'." << endl;
        exit(0);
    }
    return it->second;
}


// ===========================================================
//   Arrays
// ===========================================================

Type* TypeChecker::visit(ArrayAccessExp* e){
    if(!env.check(e->id)){
        cerr << "array no declarado: " << e->id << endl; exit(0);
    }
    Type* t = env.lookup(e->id);
    if(t->ttype != Type::ARRAY){
        cerr << e->id << " no es array\n"; exit(0);
    }

    Type* idxT = e->index->accept(this);
    if(!idxT->match(intType)){
        cerr << "indice de array debe ser int\n"; exit(0);
    }
    return t->base;
}

void TypeChecker::visit(ArrayAssignStm* s){
    if(!env.check(s->id)){
        cerr << "array no declarado: " << s->id << endl; exit(0);
    }
    Type* t = env.lookup(s->id);
    if(t->ttype != Type::ARRAY){
        cerr << s->id << " no es array\n"; exit(0);
    }

    Type* idxT = s->index->accept(this);
    if(!idxT->match(intType)){
        cerr << "indice de array debe ser int\n"; exit(0);
    }

    Type* rightT = s->e->accept(this);
    if(!t->base->match(rightT)){
        cerr << "tipos incompatibles en asignacion a array\n"; exit(0);
    }
}

// ===========================================================
//   Strings
// ===========================================================

Type* TypeChecker::visit(StringExp* e) {
    // string literal: array de char
    Type* arr = new Type(Type::ARRAY);
    arr->base = charType;
    arr->length = static_cast<int>(e->value.size()) + 1; //longitud len+1 (por el '\0')
    return arr;
}
