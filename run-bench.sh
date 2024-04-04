INPUT_FILE=$1

if [ -z "$INPUT_FILE" ]; then
    echo "Needs a single argument as the input file"
    exit 1
fi

rows=()

paths=(
    "./c/puzzle.exe"
    "./cpp/puzzle.exe"
    "./csharp/npuzzle/publish/npuzzle.exe"
    "./go/puzzle.exe"
    "./rust/target/release/puzzle.exe"
    "./ocaml/_build/install/default/bin/puzzleml.exe"
    "java -cp ./java/out/puzzle.jar src.Main"
    "node ./nodejs/puzzle.js"
    "python ./python/puzzle.py"
)

languages=(
    "C"
    "C++"
    "C#"
    "Go"
    "Rust"
    "OCaml"
    "Java"
    "Node.js"
    "Python"
)

extensions=(
    "c"
    "cpp"
    "cs"
    "go"
    "rs"
    "ml"
    "java"
    "js"
    "py"
)

get_row() {
    local str=$(echo "$1" | tail -n 1)
    local total=$(echo "$str" | grep -oP '(?<=Total: )\d+\.\d+')
    local nodes=$(echo "$str" | grep -oP '(?<=, )\d+(?= nodes)')
    echo "$2, $total, $nodes, $3"
}

for ((i = 0; i < ${#paths[@]}; i++)); do
    path="${paths[$i]}"
    language="${languages[$i]}"
    extension="${extensions[$i]}"

    result=$(eval "$path" "$INPUT_FILE")
    echo "$result"
    row=$(get_row "$result" "$language" "$extension")
    rows+=("$row")
done

printf "\nLanguage, Time (ms), Nodes, Extension\n"
for row in "${rows[@]}"; do
    echo "$row"
done