all:
	g++ -std=c++11 -o server server.cpp -lpthread -lsqlite3
	g++ -std=c++11 -o client client.cpp
