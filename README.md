Esse projeto é um fork do CompilerRinha (https://github.com/Erik-Morbach/CompilerRinha),
porém ao invés de interpretar o programa, esse projeto gera um programa C++, compila e executa.

## Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release . && cmake --build .
```

## Run

```bash
./run.sh <path-to-.json-file>
```

## Docker
Usando docker:
```bash
docker buildx build --platform linux/amd64 -f ./Dockerfile -t jsc-rinher .

docker run -v $(pwd)/$1:/var/rinha/source.rinha.json --memory=2gb --cpus=2 jsc-rinher
```
ou
```bash
./docker.sh <path-to-ast.json>
```

## tests
```bash
./tests.sh
```
