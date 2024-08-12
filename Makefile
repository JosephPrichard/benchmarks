SHELL=bash

all: c-build zig-build go-build rust-build csharp-build java-build node-build ocaml-build 

c-build: c
	gcc -O3 -march=native c/main.c -o c/puzzle.exe -lm -lpthread

go-build: go
	cd go && \
	go build

zig-build: zig
	cd zig && \
	zig build-exe src/main.zig -O ReleaseFast

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

node-build: nodejs
	cd nodejs && \
	npm install

clean:
	rm -rf *.exe *.jar