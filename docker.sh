docker buildx build --platform linux/amd64 -f ./Dockerfile -t jsc-rinher .

docker run -v $(pwd)/$1:/var/rinha/source.rinha.json --memory=2gb --cpus=2 jsc-rinher
