import subprocess
import re
import sys

def get_row(output, lang, parallel):
    lines = output.strip().splitlines()[-2:]
    print(lang, lines)
    total = re.search(r'(?<=Total: )\d+(\.\d+)?', lines[0])
    nodes = re.search(r'(?<=, )\d+(?= nodes)', lines[0])
    ete = re.search(r'(?<=End-to-end: )\d+(\.\d+)?', lines[1])
    return f"{lang}, {ete.group() if ete else ''}, {total.group() if total else ''}, {nodes.group() if nodes else ''}, {parallel}"

def run_command(command):
    args = command.split(' ');
    print("Executing ", args)
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, shell=True)
    (out, _) = proc.communicate()
    return out

if len(sys.argv) != 2:
    print("Needs a single argument as the input file")
    sys.exit(1)

commands = [
    (".\\rust\\target\\release\\puzzle.exe", "Rust Arena", "arena par", "yes"),
    (".\\rust\\target\\release\\puzzle.exe", "Rust Arena", "arena seq", "no"),
    (".\\rust\\target\\release\\puzzle.exe", "Rust Rc", "rc par", "yes"),
    (".\\rust\\target\\release\\puzzle.exe", "Rust Rc", "rc seq", "no"),
    # (".\\zig\\main.exe", "Zig", "seq", "no"),
    (".\\c\\puzzle.exe", "C", "par", "yes"),
    (".\\c\\puzzle.exe", "C", "seq", "no"),
    (".\\go\\puzzle.exe", "Go", "par", "yes"),
    (".\\go\\puzzle.exe", "Go", "seq", "no"),
    ("java -cp .\\java\\out\\puzzle.jar src.Main", "Java", "par", "yes"),
    ("java -cp .\\java\\out\\puzzle.jar src.Main", "Java", "seq", "no"),
    (".\\csharp\\npuzzle\\publish\\npuzzle.exe", "C#", "par", "yes"),
    (".\\csharp\\npuzzle\\publish\\npuzzle.exe", "C#", "seq", "no"),
    ("node .\\nodejs\\puzzle.js", "Nodejs", "", "no"),
    # (".\\ocaml\\_build\\install\\default\\bin\\puzzleml.exe", "Ocaml", "", "no"),
    ("py .\\python\\puzzle.py", "Python", "par", "yes"),
    ("py .\\python\\puzzle.py", "Python", "", "no")
]

input_file = sys.argv[1]
rows = []

for command, lang, mode, parallel in commands:
    full_command = f'{command} {input_file} {mode}'.strip()
    result = run_command(full_command)
    result = result.decode("utf-8")
    row = get_row(result, lang, parallel)
    rows.append((row, result))

for (_, result) in rows:
    print(result)

print("Lang, Ete (ms), Total (ms), Nodes, Parallel")
for (row, _) in rows:
    print(row)