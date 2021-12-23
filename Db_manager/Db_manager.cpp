#include<sqlite3.h>
#include<iostream>
#include<cstring>
#define CHECK if (err) {				\
    std::cerr << sqlite3_errmsg(db) << '\n';		\
  }

namespace db {

  int callback(void *NotUsed, int argc, char **argv, char **azColName){
    for (int i=0; i < argc; ++i)
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
  }

  int count_result(void* result, int argc, char **argv, char **azColName){
    int *_result = (int *) result;
    *_result = argc;
    return 0;
  }

  constexpr char db_filename[] = "test.db";
  enum status {OK, FAILED, USER_EXISTS};
  class Db_manager {
    sqlite3 *db;
    std::string cmd;
    int err;
    char *errmsg;
  public:
    Db_manager() {
      err = sqlite3_open(db_filename, &db);
      CHECK;
      cmd = "CREATE TABLE IF NOT EXISTS UserList ("
	"name TEXT PRIMARY KEY,"
	"password TEXT NOT NULL)";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;
      cmd = "CREATE TABLE IF NOT EXISTS UserInfo ("
	"name TEXT PRIMARY KEY"
	"friendList TEXT NOT NULL"
	"chatroomList TEXT NOT NULL)";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;
      cmd = "CREATE TABLE IF NOT EXISTS FriendRequest ("
	"from TEXT NOT NULL"
	"to TEXT NOT NULL"
	"UNIQUE(from, to))";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;

    }

    ~Db_manager() {
      sqlite3_close(db);
    }

    db::status sign_up(std::string name, std::string password) {
      cmd = "SELECT name, name FROM UserList WHERE name='";
      cmd += name + "'";
      int callback_val = 0;
      err = sqlite3_exec(db, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
      CHECK;
      if (callback_val > 0)
	return db::status::USER_EXISTS;
      cmd = "INSERT INTO UserList (name, password) VALUES ('";
      cmd += name + "', '" + password + "')";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      return db::status::OK;
    }

    db::status sign_in(std::string name, std::string password) {
      cmd = "SELECT name FROM UserList WHERE name='";
      cmd += name + "' AND password='" + password + "'";
      int callback_val = 0;
      err = sqlite3_exec(db, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
      CHECK;
      return callback_val == 0? db::status::FAILED : db::status::OK;
    }
  };
}

