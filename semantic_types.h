#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include <iostream>
#include <string>
using namespace std;

// ===========================================================
//  Representación de tipos básicos del lenguaje
// ===========================================================

class Type {
public:
    enum TType { NOTYPE, VOID, INT, BOOL, CHAR, STRUCT, ARRAY /* luego LONG, PTR */ };
    static const char* type_names[4];

    TType ttype;
    Type* base = nullptr;   //Tipo base, solo para Arrays //Ej. int a[5] ttype = Array y base = int
    int length = 0;         //para Arrays
    string alias;
    Type() : ttype(NOTYPE) {}
    Type(TType tt) : ttype(tt) {}

    Type* resolve() {
        // Si tiene alias, retornar copia sin alias
        if (!alias.empty()) {
            Type* t = new Type(ttype);
            t->base = base;
            t->length = length;
            return t;
        }
        return this;
    }

    // Comparación de tipos
    bool match(Type* t) const {
        return this->ttype == t->ttype;
    }

    // Asignación de tipo básico desde string
    bool set_basic_type(const string& s) {
        TType tt = string_to_type(s);
        if (tt == NOTYPE) return false;
        ttype = tt;
        return true;
    }

    // Conversión string 
    static TType string_to_type(const string& s) {
        if (s == "int") return INT;
        if (s == "bool") return BOOL;
        if (s == "void") return VOID;
        if (s == "char") return CHAR;
        return NOTYPE;
    }


};

inline const char* Type::type_names[4] = { "notype", "void", "int", "bool" };

#endif // SEMANTIC_TYPES_H
