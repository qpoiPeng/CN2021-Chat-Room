#pragma once

#include<sqlite3.h>
#include<iostream>
#include<cstring>
#include<vector>

#define CHECK if (err) {				\
    std::cerr << sqlite3_errmsg(database) << '\n';		\
}

namespace db {

    constexpr char db_filename[] = "test.db";
    enum status {OK, FAILED, USER_EXISTS, USER_NOT_EXISTS, DUPLICATED_REQUEST};

    int callback(void *NotUsed, int argc, char **argv, char **azColName);

    int count_result(void* result, int argc, char **argv, char **azColName);

    int get_request_list(void* list, int argc, char **argv, char **colName);

    int get_request_string(void* s, int argc, char **argv, char **colName);

    std::vector<std::string> split_string(std::string s, std::string delim = " ");

    std::string merge_string(std::vector<std::string> v, std::string delim = ",");

    class Db_manager {
        public:
            Db_manager() {
                err = sqlite3_open(db::db_filename, &database);
                CHECK;
                cmd = "CREATE TABLE IF NOT EXISTS UserList ("
                    "name TEXT PRIMARY KEY,"
                    "password TEXT NOT NULL)";
                err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
                CHECK;
                cmd = "CREATE TABLE IF NOT EXISTS UserInfo ("
                    "name TEXT PRIMARY KEY, "
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

        private:
            sqlite3 *database;
            std::string cmd;
            int err;
            char *errmsg;
    };
}

