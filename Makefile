main: server.cpp
	npm run build
	g++ server.cpp -o server -pthread
	./server

clean:
	rm -rf server dist
