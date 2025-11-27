#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "visitor.h"
#include <filesystem>
namespace fs = std::filesystem;


using namespace std;

int main(int argc, const char* argv[]) {
    // ==========================
    // Validar argumentos
    // ==========================
    if (argc != 2) {
        cout << "Número incorrecto de argumentos.\n";
        cout << "Uso: " << argv[0] << " <archivo_de_entrada>" << endl;
        return 1;
    }

    const char* filename = argv[1];

    // ==========================
    // Leer archivo completo
    // ==========================
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "No se pudo abrir el archivo: " << filename << endl;
        return 1;
    }

    string input, line;
    while (getline(infile, line)) {
        input += line + '\n';
    }
    infile.close();

    // ======================================================
    // 1) SCANNER: imprimir tokens como antes (Scanner exitoso)
    // ======================================================
    {
        Scanner scanner1(input.c_str());
        // Esto genera <base>_tokens.txt en el mismo directorio
        // con el formato:
        // Scanner
        //
        // TOKEN(...)
        // ...
        // Scanner exitoso
        ejecutar_scanner(&scanner1, string(filename));
    }

    // ======================================================
    // 2) PARSER: construir AST
    // ======================================================
    Program* program = nullptr;
    {
        Scanner scanner2(input.c_str());
        Parser parser(&scanner2);

        try {
            program = parser.parseProgram();
        } catch (const std::exception& e) {
            cerr << "Error al parsear: " << e.what() << endl;
            return 1;
        }

        cout << "Parseo exitoso" << endl;
    }

    // ======================================================
    // 3) GENERADOR DE CÓDIGO X86: GenCodeVisitor
    // ======================================================
    {
        // Ruta del input (ej: inputs/input1.txt)
        fs::path inPath(filename);

        // stem = "input1"
        std::string stem = inPath.stem().string();

        // carpeta outputs/codigo
        fs::path outDir = fs::path("outputs") / "codigo";
        fs::create_directories(outDir);  // crea outputs/ y outputs/codigo si no existen

        // archivo final: outputs/codigo/input1.s
        fs::path outPath = outDir / (stem + ".s");

        ofstream outfile(outPath);
        if (!outfile.is_open()) {
            cerr << "Error al crear el archivo de salida: " << outPath << endl;
            return 1;
        }

        cout << "Generando codigo ensamblador en " << outPath << endl;

        GenCodeVisitor codigo(outfile);
        codigo.generar(program);

        outfile.close();
    }


    return 0;
}
