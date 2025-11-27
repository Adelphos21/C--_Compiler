import os
import subprocess
import shutil
from pathlib import Path

programa = [
    "main.cpp", "scanner.cpp", "token.cpp", "parser.cpp",
    "ast.cpp", "visitor.cpp", "typeChecker.cpp"
]

# Compilar
compile_cmd = ["g++", "-std=c++17"] + programa
print("Compilando:", " ".join(compile_cmd))
result = subprocess.run(compile_cmd, capture_output=True, text=True)

if result.returncode != 0:
    print("Error en compilación:\n", result.stderr)
    exit(1)

print("Compilación exitosa")

exe_name = "./a.out" if os.name == "nt" else "./a.out"
if not os.path.isfile(exe_name):
    exe_name = "./a.out" if os.path.isfile("./a.out") else "./a.out"

input_dir = Path("inputs")
output_dir = Path("outputs")

tokens_dir = output_dir / "tokens"
parser_dir = output_dir / "parser"
codigo_dir = output_dir / "codigo"

tokens_dir.mkdir(parents=True, exist_ok=True)
parser_dir.mkdir(parents=True, exist_ok=True)
codigo_dir.mkdir(parents=True, exist_ok=True)

inputs = sorted(input_dir.glob("*.txt"))
if not inputs:
    print("No hay archivos .txt en inputs/")
    exit(0)

for filepath in inputs:
    stem = filepath.stem  # input1

    print(f"\nEjecutando {filepath.name}")

    # borra ast viejo
    if Path("ast.dot").is_file():
        Path("ast.dot").unlink()

    # correr tu compilador
    run_cmd = [exe_name, str(filepath)]
    result = subprocess.run(run_cmd, capture_output=True, text=True)

    # stdout/stderr -> outputs/parser
    run_output_file = parser_dir / f"{stem}_run.txt"
    with open(run_output_file, "w", encoding="utf-8") as f:
        f.write("=== STDOUT ===\n")
        f.write(result.stdout)
        f.write("\n=== STDERR ===\n")
        f.write(result.stderr)

    # tokens -> outputs/tokens
    tokens_file = input_dir / f"{stem}_tokens.txt"
    if tokens_file.is_file():
        shutil.move(str(tokens_file), str(tokens_dir / f"{stem}_tokens.txt"))

    # ast.dot -> outputs/parser
    ast_file = Path("ast.dot")
    if ast_file.is_file():
        dest_ast = parser_dir / f"{stem}.dot"
        shutil.move(str(ast_file), str(dest_ast))
        dest_png = parser_dir / f"{stem}.png"
        subprocess.run(["dot", "-Tpng", str(dest_ast), "-o", str(dest_png)],
                       capture_output=True, text=True)

    # ============================
    # ACA está el fix importante:
    # mover SOLO el asm de ese input
    # ============================
    asm_inputs = input_dir / f"{stem}.s"   # inputs/input1.s
    asm_root   = Path(f"{stem}.s")         # ./input1.s por si acaso

    if asm_inputs.is_file():
        shutil.move(str(asm_inputs), str(codigo_dir / f"{stem}.s"))
    elif asm_root.is_file():
        shutil.move(str(asm_root), str(codigo_dir / f"{stem}.s"))

print("\nListo. outputs/tokens, outputs/parser, outputs/codigo.")
