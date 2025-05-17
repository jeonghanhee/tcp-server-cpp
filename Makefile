all: server

server: server.c
	g++ -std=c++17 -o server server.cpp -lws2_32 -pthread