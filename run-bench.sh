INPUT_FILE=$1

if [ -z "$INPUT_FILE" ]; then
    echo "Needs a single argument as the input file"
    exit 1
fi

get_row() {
    local str=$(echo "$1" | tail -n 2)
    local total=$(echo "$str" | grep -oP '(?<=Total: )\d+(\.\d+)?')
    local nodes=$(echo "$str" | grep -oP '(?<=, )\d+(?= nodes)')
    local ete=$(echo "$str" | grep -oP '(?<=End-to-end: )\d+(\.\d+)?')
    echo "$2, $ete, $total, $nodes", $3
}

rows=()

result=$(eval "./c/puzzle.exe" "$INPUT_FILE" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "C" "yes")
rows+=("$row")

result=$(eval "./c/puzzle.exe" "$INPUT_FILE" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "C" "no")
rows+=("$row")

result=$(eval "./cpp/puzzle.exe" "$INPUT_FILE" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "C++" "yes")
rows+=("$row")

result=$(eval "./cpp/puzzle.exe" "$INPUT_FILE" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "C++" "no")
rows+=("$row")

result=$(eval "./rust/target/release/puzzle.exe" "$INPUT_FILE" "arena" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "Rust Arena" "yes")
rows+=("$row")

result=$(eval "./rust/target/release/puzzle.exe" "$INPUT_FILE" "arena" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "Rust Arena" "no")
rows+=("$row")

result=$(eval "./rust/target/release/puzzle.exe" "$INPUT_FILE" "rc" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "Rust Rc" "yes")
rows+=("$row")

result=$(eval "./rust/target/release/puzzle.exe" "$INPUT_FILE" "rc" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "Rust Rc" "no")
rows+=("$row")

result=$(eval "./csharp/npuzzle/publish/npuzzle.exe" "$INPUT_FILE" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "C#" "yes")
rows+=("$row")

result=$(eval "./csharp/npuzzle/publish/npuzzle.exe" "$INPUT_FILE" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "C#" "no")
rows+=("$row")

result=$(eval "./go/puzzle.exe" "$INPUT_FILE" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "Go" "yes")
rows+=("$row")

result=$(eval "./go/puzzle.exe" "$INPUT_FILE" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "Go" "no")
rows+=("$row")

result=$(eval "java -cp ./java/out/puzzle.jar src.Main" "$INPUT_FILE" "par")
echo "$result"
printf "\n"
row=$(get_row "$result" "Java" "yes")
rows+=("$row")

result=$(eval "java -cp ./java/out/puzzle.jar src.Main" "$INPUT_FILE" "seq")
echo "$result"
printf "\n"
row=$(get_row "$result" "Java" "no")
rows+=("$row")

# result=$(eval "./ocaml/_build/install/default/bin/puzzleml.exe" "$INPUT_FILE")
# echo "$result"
# printf "\n"
# row=$(get_row "$result" "Ocaml" "no")
# rows+=("$row")

result=$(eval "node ./nodejs/puzzle.js" "$INPUT_FILE")
echo "$result"
printf "\n"
row=$(get_row "$result" "Nodejs" "no")
rows+=("$row")

result=$(eval "py ./python/puzzle.py" "$INPUT_FILE")
echo "$result"
printf "\n"
row=$(get_row "$result" "Python" "no")
rows+=("$row")

printf "Lang, Ete (ms), Total (ms), Nodes, Parallel\n"
for row in "${rows[@]}"; do
    echo "$row"
done