#include <iostream>
#include "ast.h"
#include "visitor.h"
#include <unordered_map>
#include "typeChecker.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////////

int BinaryExp::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int NumberExp::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int ForStm::accept(Visitor* visitor){
    return visitor->visit(this);
}

int Program::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int IdExp::accept(Visitor* visitor) {
    return visitor->visit(this);
}


int PrintStm::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int AssignStm::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int IfStm::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int WhileStm::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int Body::accept(Visitor* visitor){
    return visitor->visit(this);
}

int VarDec::accept(Visitor* visitor){
    return visitor->visit(this);
}

int FcallExp::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int FunDec::accept(Visitor* visitor){
    return visitor->visit(this);
}

int ReturnStm::accept(Visitor* visitor){
    return visitor->visit(this);
}

int ExprStm::accept(Visitor* visitor){
    return visitor->visit(this);
}

int TernaryExp::accept(Visitor* visitor) {
    return visitor->visit(this);
}

int ArrayAccessExp::accept(Visitor* visitor){
    return visitor->visit(this);
}

int ArrayAssignStm::accept(Visitor* visitor){
    return visitor->visit(this);
}

int StringExp::accept(Visitor* visitor){
    return visitor->visit(this);
}

///////////////////////////////////////////////////////////////////////////////////

int GenCodeVisitor::generar(Program* program) {
    TypeChecker tc;
    tc.typecheck(program);

    tipe.tipe(program);
    fun_reserva = tipe.fun_locales;

    program->accept(this);
        return 0;
}

int GenCodeVisitor::visit(Program* program) {
    enviroment.add_level();
    out << ".data\n";
    out << "print_fmt: .string \"%ld\\n\"\n";
    out << "print_str_fmt: .string \"%s\\n\"\n";
    for (auto dec : program->vdlist){
        dec->accept(this);
    }

    for (auto& [var, _] : memoriaGlobal) {
        if(memoriaGlobalArrayLen.count(var)){
            out << var << ": .zero " << memoriaGlobalArrayLen[var]*8 << "\n";
        } else {
            out << var << ": .quad 0\n";
        }
    }

    out << ".text\n";
    
    for (auto dec : program->fdlist){
        dec->accept(this);
    }

    out << ".section .note.GNU-stack,\"\",@progbits"<<endl;
    enviroment.remove_level();
        return 0;
}

int GenCodeVisitor::visit(VarDec* stm) {
    for (auto var : stm->vars) {
        if (!entornoFuncion) {
            //GLOBAL
            memoriaGlobal[var.name] = true;
            if(var.isArray) memoriaGlobalArrayLen[var.name] = var.length;

        } else {
            enviroment.add_var(var.name, offset);
            if(!var.isArray) offset -= 8;
            else offset -= (var.length * 8);
        }
    }
        return 0;
}

int GenCodeVisitor::visit(NumberExp* exp) {
    out << " movq $" << exp->value << ", %rax"<<endl;
    return 0;
}

int GenCodeVisitor::visit(IdExp* exp) {
    if (memoriaGlobal.count(exp->value))
        out << " movq " << exp->value << "(%rip), %rax"<<endl;
    else
        out << " movq " << (enviroment.lookup(exp->value)) << "(%rbp), %rax"<<endl;
    return 0;
}

int GenCodeVisitor::visit(BinaryExp* exp) {
    exp->left->accept(this);
    out << " pushq %rax\n";
    exp->right->accept(this);
    out << " movq %rax, %rcx\n popq %rax\n";

    switch (exp->op) {
        case PLUS_OP:  out << " addq %rcx, %rax\n"; break;
        case MINUS_OP: out << " subq %rcx, %rax\n"; break;
        case MUL_OP:   out << " imulq %rcx, %rax\n"; break;
        case LE_OP:
            out << " cmpq %rcx, %rax\n"
                      << " movl $0, %eax\n"
                      << " setl %al\n"
                      << " movzbq %al, %rax\n";
            break;
        case LEQ_OP:
            out << " cmpq %rcx, %rax\n"
                <<   " movl $0, %eax\n"
                <<   " setle %al\n"
                <<   " movzbq %al, %rax\n";
            break;

        case GE_OP:
            out << " cmpq %rcx, %rax\n"
                <<   " movl $0, %eax\n"
                <<   " setg %al\n"
                <<   " movzbq %al, %rax\n";
            break;
        case GEQ_OP:
            out << " cmpq %rcx, %rax\n"
                <<   " movl $0, %eax\n"
                <<   " setge %al\n"
                <<   " movzbq %al, %rax\n";
            break;
    }
    return 0;
}


int GenCodeVisitor::visit(AssignStm* stm) {
    stm->e->accept(this);
    if (memoriaGlobal.count(stm->id))
        out << " movq %rax, " << stm->id << "(%rip)"<<endl;
    else
        out << " movq %rax, " << (enviroment.lookup(stm->id)) << "(%rbp)"<<endl;
            return 0;
}

int GenCodeVisitor::visit(PrintStm* stm) {
    if (auto se = dynamic_cast<StringExp*>(stm->e)) {
        stm->e->accept(this);   // leaq str_k(%rip), %rax
        out << " movq %rax, %rdi\n";
        out << " leaq print_fmt(%rip), %rdi\n";
        out << " movl $0, %eax\n";
        out << " call printf@PLT\n";
        return 0;
    }

    stm->e->accept(this);
    out <<
        " movq %rax, %rsi\n"
        " leaq print_fmt(%rip), %rdi\n"
        " movl $0, %eax\n"
        " call printf@PLT\n";
    return 0;
}



int GenCodeVisitor::visit(Body* b) {
    enviroment.add_level();
    for (auto dec : b->declarations){
        dec->accept(this);
    }
    for (auto s : b->StmList){
        s->accept(this);
    }
    enviroment.remove_level();
        return 0;
}

int GenCodeVisitor::visit(IfStm* stm) {
    int label = labelcont++;
    stm->condition->accept(this);
    out << " cmpq $0, %rax"<<endl;
    out << " je else_" << label << endl;
   stm->then->accept(this);
    out << " jmp endif_" << label << endl;
    out << " else_" << label << ":"<< endl;
    if (stm->els) stm->els->accept(this);
    out << "endif_" << label << ":"<< endl;
    return 0;
}

int GenCodeVisitor::visit(WhileStm* stm) {
    int label = labelcont++;
    out << "while_" << label << ":"<<endl;
    stm->condition->accept(this);
    out << " cmpq $0, %rax" << endl;
    out << " je endwhile_" << label << endl;
    stm->b->accept(this);
    out << " jmp while_" << label << endl;
    out << "endwhile_" << label << ":"<< endl;
    return 0;
}

int GenCodeVisitor::visit(ForStm* stm) {
    int label = labelcont++;
    enviroment.add_level();  //scope del for

    // ---- initDec ----
    if (stm->initDec){
        stm->initDec->accept(this);
    }
    
    // ---- initAssign ----
    if (stm->initAssign){
        stm->initAssign->accept(this);
    }

    out << "for_" << label << ":\n";

    stm->condition->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je endfor_" << label << "\n";
    
    stm->cuerpo->accept(this);
    stm->step->accept(this);

    out << " jmp for_" << label << "\n";
    out << "endfor_" << label << ":\n";

    enviroment.remove_level();

    return 0;
}

int GenCodeVisitor::visit(ReturnStm* stm) {
    stm->e->accept(this);
    out << " jmp .end_"<<nombreFuncion << endl;
    return 0;
}

int GenCodeVisitor::visit(FunDec* f) {
    entornoFuncion = true;
    enviroment.add_level();
    offset = -8;
    nombreFuncion = f->nombre;
    vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    out << ".globl " << f->nombre << endl;
    out << f->nombre <<  ":" << endl;
    out << " pushq %rbp" << endl;
    out << " movq %rsp, %rbp" << endl;
    out << " subq $" << fun_reserva[f->nombre]*8 << ", %rsp" << endl;
    int size = f->Pnombres.size();
    for (int i = 0; i < size; i++) {
        enviroment.add_var(f->Pnombres[i], offset);
        out << " movq " << argRegs[i] << "," << offset << "(%rbp)" << endl;
        offset -= 8;
    }
    for (auto i: f->cuerpo->declarations){
        i->accept(this);
    }
    int reserva = -offset - 8;
    
    for (auto i: f->cuerpo->StmList){
        i->accept(this);
    }
    out << ".end_"<< f->nombre << ":"<< endl;
    out << "leave" << endl;
    out << "ret" << endl;
    entornoFuncion = false;
    enviroment.remove_level();
    return 0;
}

int GenCodeVisitor::visit(ExprStm* stm) {
    stm->e->accept(this);
    return 0;
}

int GenCodeVisitor::visit(TernaryExp* exp) {
    int falseLabel = labelcont++;
    int endLabel = labelcont++;
    
    // Evaluar condición
    exp->condition->accept(this);
    out << " cmpq $0, %rax\n";
    out << " je .L" << falseLabel << "\n";
    
    // Rama verdadera
    exp->trueExpr->accept(this);
    out << " jmp .L" << endLabel << "\n";
    
    // Rama falsa
    out << ".L" << falseLabel << ":\n";
    exp->falseExpr->accept(this);
    
    out << ".L" << endLabel << ":\n";
    return 0;
}

int GenCodeVisitor::visit(FcallExp* exp) {
    vector<std::string> argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    int size = exp->argumentos.size();
    for (int i = 0; i < size; i++) {
        exp->argumentos[i]->accept(this);
        out << " mov %rax, " << argRegs[i] <<endl;
    }
    out << "call " << exp->nombre << endl;
    return 0;
}

int GenCodeVisitor::visit(ArrayAccessExp* exp){
    // eval index -> %rax
    exp->index->accept(this);
    out << " imulq $8, %rax\n";  // index*8

    if (memoriaGlobal.count(exp->id)) {
        // GLOBAL: label apunta a a[0], crece hacia arriba
        out << " leaq " << exp->id << "(%rip), %rcx\n";
        out << " addq %rax, %rcx\n";   // base + index*8
    } else {
        // LOCAL: base guardada es a[0] en -8, crece hacia abajo
        int base = enviroment.lookup(exp->id);
        out << " leaq " << base << "(%rbp), %rcx\n";
        out << " subq %rax, %rcx\n";   // base - index*8 
    }

    out << " movq (%rcx), %rax\n";
    return 0;
}

int GenCodeVisitor::visit(ArrayAssignStm* stm){
    // 1) calcula addr en rcx
    stm->index->accept(this);
    out << " imulq $8, %rax\n";

    if(memoriaGlobal.count(stm->id)){
        out << " leaq " << stm->id << "(%rip), %rcx\n";
        out << " addq %rax, %rcx\n";
    } else {
        int base = enviroment.lookup(stm->id);
        out << " leaq " << base << "(%rbp), %rcx\n";
        out << " subq %rax, %rcx\n";
    }
    out << " pushq %rcx\n";
    stm->e->accept(this); 
    out << " popq %rcx\n";
    
    out << " movq %rax, (%rcx)\n";
    return 0;
}

static string escape(const string& s) {
    string r;
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '"') r += "\\\"";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

int GenCodeVisitor::visit(StringExp* exp) {
    string lit = exp->value;
    string label;
    auto it = stringLabels.find(lit);
    if (it == stringLabels.end()) {
        label = "str_" + to_string(stringCounter++);
        stringLabels[lit] = label;

        out << ".section .rodata\n";
        out << label << ": .string \"" << escape(lit) << "\"\n";
        out << ".text\n";
    } else {
        label = it->second;
    }

    out << " leaq " << label << "(%rip), %rax\n";
    return 0;
}


/////////////////////////////////////////////
/////////////////////////////////////////////
///
int LocalsCounterVisitor::tipe(Program *program) {
    for(auto i: program->fdlist) {
        i->accept(this);
    }

    return 0;
}

int LocalsCounterVisitor::visit(FunDec *fd) {
    int parametros = fd->Pnombres.size();
    locales = 0;
    fd->cuerpo->accept(this);
    fun_locales[fd->nombre] = parametros + locales;
    return 0;
}

int LocalsCounterVisitor::visit(Body *body) {

    for (auto i:body->declarations) {
        i->accept(this);
    }

    for(auto i:body->StmList) {
        i->accept(this);
    }
    return 0;
}

int LocalsCounterVisitor::visit(VarDec *vd) {
    for (auto &v : vd->vars) {
        if (!v.isArray) locales += 1;
        else locales += v.length;
    }
    return 0;
}

int LocalsCounterVisitor::visit(IfStm *stm) {
    int a = locales;

    stm-> then -> accept(this);
    int b = locales;

    int c = b;
    if (stm->els) {
        stm->els->accept(this);
        c = locales;
    }

    locales = a + max(b-a,c-b);
    return 0;
}

int LocalsCounterVisitor::visit(BinaryExp *exp) {
    return 0;
}

int LocalsCounterVisitor::visit(NumberExp *exp) {
    return 0;
}

int LocalsCounterVisitor::visit(TernaryExp* exp) {
    return 0;
}

int LocalsCounterVisitor::visit(IdExp *exp) {
    return 0;
}

int LocalsCounterVisitor::visit(Program *p) {
    return 0;
}

int LocalsCounterVisitor::visit(PrintStm *stm) {
    return 0;
}

int LocalsCounterVisitor::visit(AssignStm *stm) {
    return 0;
}

int LocalsCounterVisitor::visit(WhileStm *stm) {
    int saved = locales;
    stm->b->accept(this);
    locales = max(locales, saved);
    return 0;
}

int LocalsCounterVisitor::visit(ForStm *stm) {
    int saved = locales;

    // initDec suma locales, si existe
    if (stm->initDec){
        locales += stm->initDec->vars.size();
    }

    // el cuerpo podría declarar más
    stm->cuerpo->accept(this);

    locales = max(locales, saved + (locales - saved));
    return 0;
}

int LocalsCounterVisitor::visit(ExprStm *stm) {
    return 0;
}

int LocalsCounterVisitor::visit(FcallExp *fcall) {
    return 0;
}

int LocalsCounterVisitor::visit(ReturnStm *r) {
    return 0;
}

int LocalsCounterVisitor::visit(ArrayAssignStm *r) {
    return 0;
}

int LocalsCounterVisitor::visit(ArrayAccessExp *r) {
    return 0;
}

int LocalsCounterVisitor::visit(StringExp* exp) {
    return 0;
}