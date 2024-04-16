SHELL=bash

all: c-build cpp-build go-build rust-build csharp-build java-build ocaml-build

c-build: c
	gcc -O3 -march=native c/main.c -o c/puzzle.exe -lm -lpthread

cpp-build: cpp
	g++ -O3 -march=native -std=c++20 cpp/main.cpp -o cpp/puzzle.exe -lm -lpthread

go-build: go
	cd go && \
	go build

rust-build-debug: rust/src
	cd rust && \
	cargo build

rust-build: rust/src
	cd rust && \
	cargo build --release

ocaml-build: ocaml
	cd ocaml && \
	opam install psq && \
	opam exec dune build

csharp-build: csharp
	cd csharp/npuzzle && \
	dotnet publish -c Release -o publish -p:PublishReadyToRun=true -p:PublishSingleFile=true \
		-p:PublishTrimmed=true --self-contained true -p:IncludeNativeLibrariesForSelfExtract=true

java-build: java
	cd java && \
	javac -d out/ src/*.java && \
	jar cvf out/puzzle.jar -C out/ .

clean:
	rm -rf *.exe *.jar