import os
import subprocess
import shutil
from pathlib import Path

# =========================
# 1. Archivos C++
# =========================
programa = ["main.cpp", "scanner.cpp", "token.cpp", "parser.cpp", "ast.cpp", "visitor.cpp", "typechecker.cpp"]

# =========================
# 2. Compilar
# =========================
compile_cmd = ["g++"] + programa
print("Compilando:", " ".join(compile_cmd))
result = subprocess.run(compile_cmd, capture_output=True, text=True)

if result.returncode != 0:
    print("Error en compilación:\n", result.stderr)
    exit(1)

print("Compilación exitosa")

# Detectar nombre de ejecutable típico
exe_name = "./a.exe" if os.name == "nt" else "./a.out"
if not os.path.isfile(exe_name):
    # por si g++ deja solo a.exe en windows o a.out en linux
    exe_name = "./a.exe" if os.path.isfile("./a.exe") else "./a.out"

# =========================
# 3. Directorios
# =========================
input_dir = Path("inputs")
output_dir = Path("outputs")

tokens_dir = output_dir / "tokens"
parser_dir = output_dir / "parser"
codigo_dir = output_dir / "codigo"

tokens_dir.mkdir(parents=True, exist_ok=True)
parser_dir.mkdir(parents=True, exist_ok=True)
codigo_dir.mkdir(parents=True, exist_ok=True)

# =========================
# 4. Ejecutar todos inputs
# =========================
inputs = sorted(input_dir.glob("*.txt"))

if not inputs:
    print("No hay archivos .txt en inputs/")
    exit(0)

for filepath in inputs:
    filename = filepath.name
    stem = filepath.stem  # nombre sin extension, ej: input1

    print(f"\nEjecutando {filename}")

    # limpiar artefactos viejos que se regeneran en raíz
    if os.path.isfile("ast.dot"):
        os.remove("ast.dot")

    # correr compilador
    run_cmd = [exe_name, str(filepath)]
    result = subprocess.run(run_cmd, capture_output=True, text=True)

    # =========================
    # 4.1 Guardar stdout/stderr
    # =========================
    run_output_file = codigo_dir / f"{stem}_run.txt"
    with open(run_output_file, "w", encoding="utf-8") as f:
        f.write("=== STDOUT ===\n")
        f.write(result.stdout)
        f.write("\n=== STDERR ===\n")
        f.write(result.stderr)

    # =========================
    # 4.2 Mover tokens
    # =========================
    # tu scanner parece generar <input>_tokens.txt en inputs/
    tokens_file_inputs = input_dir / f"{stem}_tokens.txt"
    tokens_file_root = Path(f"{stem}_tokens.txt")

    if tokens_file_inputs.is_file():
        shutil.move(str(tokens_file_inputs), str(tokens_dir / f"{stem}_tokens.txt"))
    elif tokens_file_root.is_file():
        shutil.move(str(tokens_file_root), str(tokens_dir / f"{stem}_tokens.txt"))

    # =========================
    # 4.3 Mover AST y convertir PNG
    # =========================
    ast_file = Path("ast.dot")
    if ast_file.is_file():
        dest_ast = parser_dir / f"{stem}.dot"
        shutil.move(str(ast_file), str(dest_ast))

        # convertir a PNG si dot esta instalado
        dest_png = parser_dir / f"{stem}.png"
        dot_cmd = ["dot", "-Tpng", str(dest_ast), "-o", str(dest_png)]
        subprocess.run(dot_cmd, capture_output=True, text=True)

    # =========================
    # 4.4 Si generas código asm extra, muévelo
    # =========================
    # Ejemplos típicos: out.s, out.asm, codigo.s, etc.
    for extra_ext in [".s", ".asm", ".S"]:
        for extra_file in Path(".").glob(f"*{extra_ext}"):
            shutil.move(str(extra_file), str(codigo_dir / f"{stem}{extra_ext}"))

print("\nListo. Revisa outputs/ con tokens/, parser/ y codigo/.")
