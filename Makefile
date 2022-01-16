all: server web console

server:server.cpp TcpListener.cpp MultiClientChat.cpp Db_manager/Db_manager.cpp HttpParser/HttpParser.cpp HttpParser/HttpResponse.cpp
	g++ -std=c++11 -o server server.cpp TcpListener.cpp MultiClientChat.cpp Db_manager/Db_manager.cpp HttpParser/HttpParser.cpp HttpParser/HttpResponse.cpp -lsqlite3 -lpthread

web: web_client.cpp TcpListener.cpp WebServer.cpp
	g++ -std=c++11 -o web_client web_client.cpp TcpListener.cpp WebServer.cpp -lpthread

console: console_client.cpp HttpParser/HttpParser.cpp HttpParser/HttpResponse.cpp Db_manager/Db_manager.cpp
	g++ -std=c++11 -O -lsqlite3 -lpthread -o console_client console_client.cpp HttpParser/HttpParser.cpp HttpParser/HttpResponse.cpp Db_manager/Db_manager.cpp

clean:
	rm console_client web_client server
