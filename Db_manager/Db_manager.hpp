#pragma once

#include<sqlite3.h>
#include<iostream>
#include<cstring>
#include<vector>
#include<ctime>
#include<functional>
#define CHECK if (err) {					\
    std::cerr << sqlite3_errmsg(database) << '\n';		\
}

namespace db {

  struct Message {
    std::string type, from, to, content;
    unsigned int timestamp;
  };

    constexpr char db_filename[] = "test.db";
  enum status {OK, FAILED, USER_EXISTS, USER_NOT_EXISTS, DUPLICATED_REQUEST, IS_FRIEND_ALREADY, NOT_FRIEND};

    int callback(void *NotUsed, int argc, char **argv, char **azColName);

    int count_result(void* result, int argc, char **argv, char **azColName);

    int get_request_list(void* list, int argc, char **argv, char **colName);

    int get_request_string(void* s, int argc, char **argv, char **colName);

    int get_message(void* msg_list, int argc, char **argv, char **colName);

    std::string get_token(std::string name);

    std::vector<std::string> split_string(std::string s, std::string delim = " ");

    std::string merge_string(std::vector<std::string> v, std::string delim = ",");

    std::string get_dmname(std::string& user1, std::string& user2); // this will sort user1 and user2 !

    class Db_manager {
        public:
            Db_manager() {
                err = sqlite3_open(db::db_filename, &database);
                CHECK;
                cmd = "CREATE TABLE IF NOT EXISTS UserList ("
                    "name TEXT PRIMARY KEY, "
                    "password TEXT NOT NULL, "
                    "friendList TEXT NOT NULL, "
                    "chatroomList TEXT NOT NULL)";
                err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
                CHECK;
                cmd = "CREATE TABLE IF NOT EXISTS FriendRequest ("
                    "source TEXT NOT NULL, "
                    "destination TEXT NOT NULL, "
                    "UNIQUE(source, destination))";
                err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
                CHECK;
                cmd = "CREATE TABLE IF NOT EXISTS Token ("
		  "token TEXT PRIMARY KEY, "
		  "name TEXT NOT NULL) ";
                err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
                CHECK;
            }

            ~Db_manager() {
                sqlite3_close(database);
            }

            status sign_up(std::string name, std::string password);

            status sign_in(std::string name, std::string password);

            status if_user_exists(std::string name);

            status create_friend_request(std::string source, std::string destination);

            status get_friend_request_list(std::string to, std::vector<std::string>& list);

            status confirm_friend_request(std::string to, std::string source);

            status get_friend_list(std::string user, std::vector<std::string>& list);

            status write_message(std::string user1, std::string user2, std::string msg);

            status get_chat(std::string user1, std::string user2, std::vector<Message>& chat);

            status token2name(std::string token, std::string& name);

            status create_token(std::string name, std::string& token);

            status delete_friend(std::string user, std::string notfriend);
    private:
            sqlite3 *database;
            std::string cmd;
            int err;
            char *errmsg;
    };

}

