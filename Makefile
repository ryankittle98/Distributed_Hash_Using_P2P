all: Server Client

Server: UDPEchoServer.c
	g++ UDPEchoServer.c -o Server

Client: UDPEchoClient.c
	g++ UDPEchoClient.c -o Client -std=c++11 -pthread
