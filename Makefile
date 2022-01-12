all:
	g++ -std=c++11 -o server server.cpp TcpListener.cpp MultiClientChat.cpp Db_manager/Db_manager.cpp HttpParser/HttpParser.cpp -lsqlite3 -lpthread
	g++ -std=c++11 -o web_client web_client.cpp TcpListener.cpp WebServer.cpp -lpthread
	g++ -std=c++11 -o console_client console_client.cpp

clean:
	rm console_client web_client server
