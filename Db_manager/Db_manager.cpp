#include<sqlite3.h>
#include<iostream>
#include<cstring>
#include<vector>
#define CHECK if (err) {				\
    std::cerr << sqlite3_errmsg(db) << '\n';		\
  }

namespace db {

  int callback(void *NotUsed, int argc, char **argv, char **azColName){  // example
    for (int i = 0; i < argc; ++i)
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
  }

  int count_result(void* result, int argc, char **argv, char **azColName){
    int *_result = (int *) result;
    *_result = argc;
    return 0;
  }

  int get_request_list(void* list, int argc, char **argv, char **colName) {
    std::vector<std::string> *_l = (std::vector<std::string> *) list;
    for (int i = 0; i < argc; ++i)
      _l->push_back(argv[i]);
    return 0;
  }

  int get_request_string(void* s, int argc, char **argv, char **colName) {
    std::string *_s = (std::string *) s;
    *_s = argv[0];
    return 0;
  }

  std::vector<std::string> split_string(std::string s, std::string delim = " ") {
    std::vector<std::string> ret;
    int cur = 0, pos = 0;
    while ((pos = s.find(delim, cur)) != -1) {
      ret.push_back(s.substr(cur, pos - cur));
      cur = pos + 1;
    }
    if (s.substr(cur) != "")
      ret.push_back(s.substr(cur));
    return ret;
  }

  std::string merge_string(std::vector<std::string> v, std::string delim = ",") {
    std::string ret;
    for (auto s : v)
      ret += s + delim;
    ret = ret.substr(0, std::max((int) ret.length() - (int)delim.length(), 0));
    return ret;
  }
  
  constexpr char db_filename[] = "test.db";
  enum status {OK, FAILED, USER_EXISTS, USER_NOT_EXISTS, DUPLICATED_REQUEST};
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
	"name TEXT PRIMARY KEY, "
	"friendList TEXT NOT NULL, "
	"chatroomList TEXT NOT NULL)";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;
      cmd = "CREATE TABLE IF NOT EXISTS FriendRequest ("
	"source TEXT NOT NULL, "
	"destination TEXT NOT NULL, "
	"UNIQUE(source, destination))";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;

    }

    ~Db_manager() {
      sqlite3_close(db);
    }

    status sign_up(std::string name, std::string password) {
      cmd = "SELECT name FROM UserList WHERE name='";
      cmd += name + "'";
      int callback_val = 0;
      err = sqlite3_exec(db, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
      CHECK;
      if (callback_val > 0)
	return status::USER_EXISTS;
      cmd = "INSERT INTO UserList (name, password) VALUES ('";
      cmd += name + "', '" + password + "')";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;
      return status::OK;
    }

    status sign_in(std::string name, std::string password) {
      cmd = "SELECT name FROM UserList WHERE name='";
      cmd += name + "' AND password='" + password + "'";
      int callback_val = 0;
      err = sqlite3_exec(db, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
      CHECK;
      return callback_val == 0? status::FAILED : status::OK;
    }

    status if_user_exists(std::string name) {
      cmd = "SELECT name FROM UserList WHERE name='";
      cmd += name + "'";
      int callback_val = 0;
      err = sqlite3_exec(db, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
      CHECK;
      if (callback_val > 0)
	return status::USER_EXISTS;
      return status::USER_NOT_EXISTS;
    }
    
    status create_friend_request(std::string source, std::string destination) {
      cmd = "INSERT INTO FriendRequest (source, destination) VALUES ('";
      cmd += source + "', '" + destination + "')";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;
      fprintf(stderr, "Db_manager.create_friend_request : %d\n", err);
      if (err == 19)
	return status::DUPLICATED_REQUEST;
      return status::OK;
    }

    status get_friend_request_list(std::string to, std::vector<std::string>& list) {
      cmd = "SELECT source FROM FriendRequest WHERE destination ='";
      cmd += to + "'";
      err = sqlite3_exec(db, cmd.c_str(), get_request_list, &list, &errmsg);
      CHECK;
      return status::OK;
    }

    status confirm_friend_request(std::string to, std::string source) {
      cmd = "DELETE FROM FriendRequest WHERE destination ='";
      cmd += to + "'";
      cmd += " AND source = '" + source + "'";
      err = sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg);
      CHECK;

      cmd = "SELECT friendList FROM UserInfo WHERE name = '";
      cmd += to + "'";
      std::string s;
      err = sqlite3_exec(db, cmd.c_str(), get_request_string, &s, &errmsg);
      CHECK;
      fprintf(stderr, "confirm_friend_request1 : %s\n", s.c_str());
      std::vector<std::string> fl = split_string(s, ",");

      fl.push_back(source);
      s = merge_string(fl, ",");
      fprintf(stderr, "confirm_friend_request2 : %s\n", s.c_str());

      cmd = "UPDATE UserInfo SET friendList ='";
      cmd += s + "' WHERE name = '" + to + "'";
      fprintf(stderr, "confirm_friend_request3 : %s\n", cmd.c_str());
      err = sqlite3_exec(db, cmd.c_str(), get_request_string, &s, &errmsg);
      CHECK;

      /* second time */

      cmd = "SELECT friendList FROM UserInfo WHERE name = '";
      cmd += source + "'";
      s = "";
      err = sqlite3_exec(db, cmd.c_str(), get_request_string, &s, &errmsg);
      CHECK;
      fl = split_string(s, ",");

      fl.push_back(to);
      s = merge_string(fl, ",");

      cmd = "UPDATE UserInfo SET friendList ='";
      cmd += s + "' WHERE name = '" + source + "'";
      err = sqlite3_exec(db, cmd.c_str(), get_request_string, &s, &errmsg);
      CHECK;

      return status::OK;
    }
  };
}

