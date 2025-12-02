#ifndef VISITOR_H
#define VISITOR_H
#include "ast.h"
#include <list>
#include <vector>
#include <unordered_map>
#include <string>
#include "environment.h"
using namespace std;

class BinaryExp;
class NumberExp;
class Program;
class PrintStm;
class WhileStm;
class IfStm;
class AssignStm;
class Body;
class Vardec;
class FcallExp;
class ReturnStm;
class FunDec;
class TernaryExp;
class FieldAccessExp;
class FieldAssignStm;

class Visitor {
public:
    // 
    virtual int visit(BinaryExp* exp) = 0;
    virtual int visit(NumberExp* exp) = 0;
    virtual int visit(IdExp* exp) = 0;
    virtual int visit(Program* p) = 0;
    virtual int visit(PrintStm* stm) = 0;
    virtual int visit(WhileStm* stm) = 0;
    virtual int visit(ForStm* Stm) = 0;
    virtual int visit(IfStm* stm) = 0;
    virtual int visit(AssignStm* stm) = 0;
    virtual int visit(Body* body) = 0;
    virtual int visit(VarDec* vd) = 0;
    virtual int visit(FcallExp* fcall) = 0;
    virtual int visit(ReturnStm* r) = 0;
    virtual int visit(FunDec* fd) = 0;
    virtual int visit(ExprStm* stm) = 0;
    // Ternario uwu
    virtual int visit(TernaryExp* stm) = 0;
    /// ARRAY ///
    virtual int visit(ArrayAccessExp* exp) = 0;
    virtual int visit(ArrayAssignStm* stm) = 0;
    

    /// STRING ///
    virtual int visit(StringExp* exp) = 0;
    virtual int visit(FieldAccessExp* e) = 0;
    virtual int visit(FieldAssignStm* stm) = 0;

};

class LocalsCounterVisitor : public Visitor {
public:
    unordered_map<string,int> fun_locales;
    int locales;
    int tipe(Program* program);
    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(Program* p) override ;
    int visit(PrintStm* stm) override;
    int visit(AssignStm* stm) override;
    int visit(WhileStm* stm) override;
    int visit(IfStm* stm) override;
    int visit(Body* body) override;
    int visit(VarDec* vd) override;
    int visit(FcallExp* fcall) override;
    int visit(ReturnStm* r) override;
    int visit(FunDec* fd) override;
    int visit(ForStm* stm) override;
    int visit(ExprStm* stm) override;

    int visit(TernaryExp* stm) override;
    int visit(ArrayAccessExp* exp) override;
    int visit(ArrayAssignStm* stm) override;
    int visit(FieldAccessExp* e) override;
    int visit(FieldAssignStm* stm) override;
    int visit(StringExp* exp) override;
};

class GenCodeVisitor : public Visitor {
private:
    std::ostream& out;
public:
    LocalsCounterVisitor tipe;
    unordered_map<string,int> fun_reserva;
    GenCodeVisitor(std::ostream& out) : out(out) {}
    int generar(Program* program);
    //unordered_map<string, int> memoria;
    unordered_map<string,int> knownStructSizes; // nombreStruct -> size bytes
    unordered_map<string,string> varStructType; // varName -> structName
    unordered_map<string,string> stringLabels;
    int stringCounter = 0;

    Environment<int> enviroment;
    unordered_map<string, bool> memoriaGlobal;
    unordered_map<string,int> memoriaGlobalArrayLen;

    unordered_map<string, unordered_map<string,int>> structFieldOffset;
    unordered_map<string,int> structSize;


    int offset = -8;
    int labelcont = 0;
    bool entornoFuncion = false;
    string nombreFuncion;
    int visit(BinaryExp* exp) override;
    int visit(NumberExp* exp) override;
    int visit(IdExp* exp) override;
    int visit(Program* p) override ;
    int visit(PrintStm* stm) override;
    int visit(AssignStm* stm) override;
    int visit(WhileStm* stm) override;
    int visit(ForStm* stm) override;
    int visit(IfStm* stm) override;
    int visit(Body* body) override;
    int visit(VarDec* vd) override;
    int visit(FcallExp* fcall) override;
    int visit(ReturnStm* r) override;
    int visit(FunDec* fd) override;
    int visit(ExprStm* stm) override;
    void calcularStructLayout(StructDec* sd);
    int visit(TernaryExp* stm) override;
    int visit(ArrayAccessExp* exp) override;
    int visit(ArrayAssignStm* stm) override;
    int visit(FieldAccessExp* e) override;

    int visit(FieldAssignStm* stm) override;
    int visit(StringExp* exp) override;
};

#endif // VISITOR_H