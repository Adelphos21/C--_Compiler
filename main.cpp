#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "visitor.h"

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
        // Sacar el nombre base del archivo de entrada
        string inputFile(filename);
        size_t dotPos = inputFile.find_last_of('.');
        string baseName = (dotPos == string::npos)
                            ? inputFile
                            : inputFile.substr(0, dotPos);

        // El .s se crea al lado del input, e.g. inputs/input1.s
        string outputFilename = baseName + ".s";
        ofstream outfile(outputFilename);
        if (!outfile.is_open()) {
            cerr << "Error al crear el archivo de salida: " << outputFilename << endl;
            return 1;
        }

        cout << "Generando codigo ensamblador en " << outputFilename << endl;

        GenCodeVisitor codigo(outfile);
        codigo.generar(program);

        outfile.close();
    }

    return 0;
}
