INPUT_FILE=$1

if [ -z "$INPUT_FILE" ]; then
    echo "Needs a single argument as the input file"
    exit 1
fi

rows=()

paths=(
    "./c/puzzle.exe"
    "./cpp/puzzle.exe"
    "./cpp/puzzle.exe"
    "./rust/target/release/puzzle.exe"
    "./rust/target/release/puzzle.exe"
    "./csharp/npuzzle/publish/npuzzle.exe"
    "./go/puzzle.exe"
    "java -cp ./java/out/puzzle.jar src.Main"
    # "./rust/target/release/puzzle.exe"
    "./ocaml/_build/install/default/bin/puzzleml.exe"
    "node ./nodejs/puzzle.js"
    "python ./python/puzzle.py"
)

declare -A args
args["Rust Arena"]="arena"
args["Rust RC"]="rc"
args["Rust Slow"]="slow"

languages=(
    "C"
    "C++ Arena"
    "C++ shared_ptr"
    "Rust Arena"
    "Rust Rc"
    # "Rust Slow"
    "C#"
    "Go"
    "Java"
    "OCaml"
    "Node.js"
    "Python"
)

extensions=(
    "c"
    "cpp"
    "cpp"
    "rs"
    "rs"
    # "rs"
    "cs"
    "go"
    "java"
    "ml"
    "js"
    "py"
)

get_row() {
    local str=$(echo "$1" | tail -n 1)
    local total=$(echo "$str" | grep -oP '(?<=Total: )\d+(\.\d+)?')
    local nodes=$(echo "$str" | grep -oP '(?<=, )\d+(?= nodes)')
    echo "$2, $total, $nodes, $3"
}

for ((i = 0; i < ${#paths[@]}; i++)); do
    path="${paths[$i]}"
    language="${languages[$i]}"
    extension="${extensions[$i]}"

    arg="${args[$language]}"

    result=$(eval "$path" "$INPUT_FILE" "$arg")
    echo "$result"
    printf "\n"

    str=$(echo "$result" | tail -n 1)
    total=$(echo "$str" | grep -oP '(?<=Total: )\d+(\.\d+)?')
    nodes=$(echo "$str" | grep -oP '(?<=, )\d+(?= nodes)')
    row="$language, $total, $nodes, $extension"
    rows+=("$row")
done

printf "Language, Time (ms), Nodes, Extension\n"
for row in "${rows[@]}"; do
    echo "$row"
done