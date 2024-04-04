INPUT_FILE=$1

if [ -z "$INPUT_FILE" ]; then
    echo "Needs a single argument as the input file"
    exit 1
fi

mkdir output

touch output/bench-out-csharp.csv
touch output/out-csharp.txt

./c/puzzle.exe $INPUT_FILE ./output/bench-out-c.csv ./output/out-c.txt 
./cpp/puzzle.exe $INPUT_FILE ./output/bench-out-cpp.csv ./output/out-cpp.txt 
./csharp/npuzzle/publish/npuzzle.exe $INPUT_FILE ./output/bench-out-csharp.csv ./output/out-csharp.txt 
./go/puzzle.exe $INPUT_FILE ./output/bench-out-go.csv ./output/out-go.txt
./rust/target/release/puzzle.exe $INPUT_FILE ./output/bench-out-rust.csv ./output/out-rust.txt 
./ocaml/_build/install/default/bin/puzzleml.exe $INPUT_FILE ./output/bench-out-ocaml.csv ./output/out-ocaml.txt
java -cp ./java/out/puzzle.jar src.Main $INPUT_FILE ./output/bench-out-java.csv ./output/out-java.txt 
node ./nodejs/puzzle.js $INPUT_FILE ./output/bench-out-node.csv ./output/out-node.txt 
python ./python/puzzle.py $INPUT_FILE ./output/bench-out-py.csv ./output/out-py.txt 
