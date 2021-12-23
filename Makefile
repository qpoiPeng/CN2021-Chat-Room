all:
	g++ -std=c++11 -lpthread -lsqlite3 -o server server.cpp 
	g++ -std=c++11 -o client client.cpp
