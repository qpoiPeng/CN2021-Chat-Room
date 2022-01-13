all: server web console

server:
	g++ -std=c++11 -o server server.cpp TcpListener.cpp MultiClientChat.cpp Db_manager/Db_manager.cpp HttpParser/HttpParser.cpp HttpParser/HttpResponse.cpp -lsqlite3 -lpthread
	
web:	
	g++ -std=c++11 -o web_client web_client.cpp TcpListener.cpp WebServer.cpp -lpthread

console:
	g++ -std=c++11 -o console_client console_client.cpp

clean:
	rm console_client web_client server
