#ifndef AST_H
#define AST_H

#include <string>
#include <list>
#include <ostream>
#include <vector>
#include "semantic_types.h"
using namespace std;

class Visitor;
class TypeVisitor;
class VarDec;
class Body;
class Exp;
class Stm;
class AssignStm;
class StructDec;
class TypedefDec;
class FieldAccessExp;
class FieldAssignStm;

// Operadores binarios soportados
enum BinaryOp { 
    PLUS_OP, 
    MINUS_OP, 
    MUL_OP, 
    DIV_OP,
    POW_OP,
    LE_OP,
    LEQ_OP,
    GE_OP,
    GEQ_OP
};


// Clase abstracta Exp
class Exp {
public:
    virtual int  accept(Visitor* visitor) = 0;
    virtual Type* accept(TypeVisitor* v) = 0;
    virtual ~Exp() = 0;  // Destructor puro → clase abstracta
    static string binopToChar(BinaryOp op);  // Conversión operador → string
};

// Expresión binaria
class BinaryExp : public Exp {
public:
    Exp* left;
    Exp* right;
    BinaryOp op;
    int accept(Visitor* visitor);
    BinaryExp(Exp* l, Exp* r, BinaryOp op);
    ~BinaryExp();

    Type* accept(TypeVisitor* visitor);

};

// Expresión numérica
class NumberExp : public Exp {
public:
    int value;
    int accept(Visitor* visitor);
    NumberExp(int v);
    ~NumberExp();

    Type* accept(TypeVisitor* visitor);
};

class TernaryExp : public Exp {
public:
    Exp* condition;
    Exp* trueExpr;
    Exp* falseExpr;
    int accept(Visitor* visitor);
    TernaryExp(Exp* cond, Exp* trueExp, Exp* falseExp);
    ~TernaryExp();

    Type* accept(TypeVisitor* visitor);
};

// Expresión numérica
class IdExp : public Exp {
public:
    string value;
    int accept(Visitor* visitor);
    IdExp(string v);
    ~IdExp();

    Type* accept(TypeVisitor* visitor);
};

class BoolExp : public Exp {
public:
    int valor;

    BoolExp(){};
    ~BoolExp(){};

    int accept(Visitor* visitor);
    Type* accept(TypeVisitor* visitor); // nuevo
};

class ArrayAccessExp : public Exp {
public:
    string id;
    Exp* index;

    ArrayAccessExp(string id, Exp* index): id(id), index(index) {}
    int accept(Visitor* v) override;
    Type* accept(TypeVisitor* v) override;
    ~ArrayAccessExp(){ delete index; }
};

class StringExp : public Exp {
public:
    string value;   // el contenido sin las comillas

    StringExp(const string& v) : value(v) {}
    ~StringExp() {}

    int  accept(Visitor* v) override;
    Type* accept(TypeVisitor* v) override;
};

class Stm{
public:
    virtual int accept(Visitor* visitor) = 0;
    virtual ~Stm() = 0;

    virtual void accept(TypeVisitor* visitor) = 0;
};

class ArrayAssignStm : public Stm {
public:
    string id;
    Exp* index;
    Exp* e;

    ArrayAssignStm(string id, Exp* index, Exp* e)
        : id(id), index(index), e(e) {}

    int accept(Visitor* v) override;
    void accept(TypeVisitor* v) override;
    ~ArrayAssignStm(){ delete index; delete e; }
};


struct VarItem {
    string name;
    bool isArray;
    int length; // solo válido si isArray=true
    VarItem(string n, bool arr=false, int len=0): name(n), isArray(arr), length(len) {}
};

class VarDec{
public:
    string type;
    list<VarItem> vars;

    VarDec();
    int accept(Visitor* visitor);
    void accept(TypeVisitor* visitor);
    ~VarDec();
};


class Body{
public:
    list<Stm*> StmList;
    list<VarDec*> declarations;
    int accept(Visitor* visitor);
    Body();
    ~Body();

    void accept(TypeVisitor* visitor);
};

class IfStm: public Stm {
public:
    Exp* condition;
    Body* then;
    Body* els;
    IfStm(Exp* condition, Body* then, Body* els);
    int accept(Visitor* visitor);
    ~IfStm(){};

    void accept(TypeVisitor* visitor);
};

class WhileStm: public Stm {
public:
    Exp* condition;
    Body* b;
    WhileStm(Exp* condition, Body* b);
    int accept(Visitor* visitor);
    ~WhileStm(){};

    void accept(TypeVisitor* visitor);
};

class ForStm: public Stm {
public:
    VarDec* initDec;       
    AssignStm* initAssign; 
    Exp* condition;
    AssignStm* step;
    Body* cuerpo;
    ForStm(VarDec* d, AssignStm* a, Exp* c, AssignStm* s, Body* b)
        : initDec(d), initAssign(a), condition(c), step(s), cuerpo(b) {}
    int accept(Visitor* visitor);
    ~ForStm(){};

    void accept(TypeVisitor* visitor);
};

class AssignStm: public Stm {
public:
    string id;
    Exp* e;
    AssignStm(string, Exp*);
    ~AssignStm();
    int accept(Visitor* visitor);

    void accept(TypeVisitor* visitor);
};

class PrintStm: public Stm {
public:
    Exp* e;
    PrintStm(Exp*);
    ~PrintStm();
    int accept(Visitor* visitor);

    void accept(TypeVisitor* visitor);
};

class ReturnStm: public Stm {
public:
    Exp* e;
    ReturnStm(){};
    ~ReturnStm(){};
    int accept(Visitor* visitor);

    void accept(TypeVisitor* visitor);
};

class FcallExp: public Exp {
public:
    string nombre;
    vector<Exp*> argumentos;
    int accept(Visitor* visitor);

    FcallExp(){};
    ~FcallExp(){};

    Type* accept(TypeVisitor* visitor);
};

class FunDec{
public:
    string nombre;
    string tipo;
    Body* cuerpo;
    vector<string> Ptipos;
    vector<string> Pnombres;
    int accept(Visitor* visitor);
    FunDec(){};
    ~FunDec(){};

    void accept(TypeVisitor* visitor);
};

class Program{
public:
    list<StructDec*> sdlist;
    list<TypedefDec*> tdlist;
    list<VarDec*> vdlist;
    list<FunDec*> fdlist;

    Program(){};
    ~Program(){};
    int accept(Visitor* visitor);

    void accept(TypeVisitor* visitor);
};

class ExprStm: public Stm {
public:
    Exp* e;
    ExprStm(Exp* e) : e(e) {}
    ~ExprStm() {}
    int accept(Visitor* visitor);
    void accept(TypeVisitor* v) override { 
        e->accept(v); 
    }
};

class StructDec {
public:
    string name;
    list<VarDec*> fields;  // reutilizamos VarDec como "field-decs"

    StructDec(string n, list<VarDec*> f): name(n), fields(f) {}
    int accept(Visitor* v);
    void accept(TypeVisitor* v);
    ~StructDec(){}
};

class TypedefDec {
public:
    string alias;
    string original; // nombre del tipo original (incluye struct o básico)
    TypedefDec(string o, string a): alias(a), original(o) {}
    int accept(Visitor* v);
    void accept(TypeVisitor* v);
    ~TypedefDec(){}
};
class FieldAccessExp : public Exp {
public:
    Exp* baseExp;     // normalmente IdExp o algo que deje dirección
    string field;

    FieldAccessExp(Exp* b, string f): baseExp(b), field(f) {}
    int accept(Visitor* v) override;
    Type* accept(TypeVisitor* v) override;
    ~FieldAccessExp(){ delete baseExp; }
};

class FieldAssignStm : public Stm {
public:
    Exp* baseExp;
    string field;
    Exp* e;

    FieldAssignStm(Exp* b, string f, Exp* e): baseExp(b), field(f), e(e) {}
    int accept(Visitor* v) override;
    void accept(TypeVisitor* v) override;
    ~FieldAssignStm(){ delete baseExp; delete e; }
};


#endif // AST_H
