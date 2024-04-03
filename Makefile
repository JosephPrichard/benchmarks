CFLAGS=-O3

all: c-build cpp-build go-build rust-build-release node-build ocaml-build csharp-build java-build

c-build: c/
	cd c && \
	gcc $(CFLAGS) puzzle.c -o puzzle.exe

cpp-build: cpp/
	cd cpp && \
	g++ $(CFLAGS) main.cpp -o puzzle.exe

go-build: go/
	cd go && \
	go build

node-build: nodejs/
	cd nodejs && \
	npm run build

rust-build-debug: rust/src/
	cd rust && \
	cargo build

rust-build-release: rust/src/
	cd rust && \
	cargo build --release

ocaml-build: ocaml/
	cd ocaml && \
	eval $(opam env) && \
	dune build

csharp-build: csharp/
	cd csharp/npuzzle && \
	dotnet build -c release

java-build: java/
	cd java && \
	javac -d out/ src/*.java && \
	jar cvf out/puzzle.jar -C out/ .

clean:
	rm -rf c/*.exe
	rm -rf cpp/*.exe
	rm -rf rust/target
	rm -rf go/puzzle.exe
	rm -rf java/out/puzzle.jar
	rm -rf java/out/src
	rm -rf nodejs/solver.js