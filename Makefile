CFLAGS=-O3 -march=native
SHELL=bash

all: c-build cpp-build go-build rust-build-release csharp-build java-build ocaml-build

c-build: c
	gcc $(CFLAGS) c/main.c -o c/puzzle.exe

cpp-build: cpp
	g++ $(CFLAGS) -std=c++20 cpp/main.cpp -o cpp/puzzle.exe

go-build: go
	cd go && \
	go build

rust-build-debug: rust/src
	cd rust && \
	cargo build

rust-build-release: rust/src
	cd rust && \
	cargo build --release

ocaml-build: ocaml
	cd ocaml && \
	opam install psq && \
	opam exec dune build

csharp-build: csharp
	cd csharp/npuzzle && \
	dotnet publish -c Release -o publish -p:PublishReadyToRun=true -p:PublishSingleFile=true -p:PublishTrimmed=true --self-contained true -p:IncludeNativeLibrariesForSelfExtract=true

java-build: java
	cd java && \
	javac -d out/ src/*.java && \
	jar cvf out/puzzle.jar -C out/ .

clean:
	rm -rf *.exe *.jar