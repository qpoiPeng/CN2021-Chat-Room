#include "Db_manager.hpp"

int db::callback(void *NotUsed, int argc, char **argv, char **azColName) {  // example
    for (int i = 0; i < argc; ++i)
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
}

int db::count_result(void* result, int argc, char **argv, char **azColName) {
    int *_result = (int *) result;
    *_result = argc;
    return 0;
}

int db::get_request_list(void* list, int argc, char **argv, char **colName) {
    std::vector<std::string> *_l = (std::vector<std::string> *) list;
    for (int i = 0; i < argc; ++i)
        _l->push_back(argv[i]);
    return 0;
}

int db::get_request_string(void* s, int argc, char **argv, char **colName) {
    std::string *_s = (std::string *) s;
    *_s = argv[0];
    return 0;
}

std::vector<std::string> db::split_string(std::string s, std::string delim = " ") {
    std::vector<std::string> ret;
    int cur = 0, pos = 0;
    while ((pos = s.find(delim, cur)) != -1) {
        ret.push_back(s.substr(cur, pos - cur));
        cur = pos + delim.size();
    }
    if (s.substr(cur) != "")
        ret.push_back(s.substr(cur));
    return ret;
}

std::string db::merge_string(std::vector<std::string> v, std::string delim = ",") {
    std::string ret;
    for (auto s : v)
        ret += s + delim;
    ret = ret.substr(0, std::max((int) ret.length() - (int)delim.length(), 0));
    return ret;
}

db::status db::Db_manager::sign_up(std::string name, std::string password) {

    cmd = "SELECT name FROM UserList WHERE name='";
    cmd += name + "'";
    int callback_val = 0;
    err = sqlite3_exec(database, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
    CHECK;
    if (callback_val > 0)
        return status::USER_EXISTS;
    cmd = "INSERT INTO UserList (name, password) VALUES ('";
    cmd += name + "', '" + password + "')";
    err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
    CHECK;
    return status::OK;
}

db::status db::Db_manager::sign_in(std::string name, std::string password) {
    cmd = "SELECT name FROM UserList WHERE name='";
    cmd += name + "' AND password='" + password + "'";
    int callback_val = 0;
    err = sqlite3_exec(database, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
    CHECK;
    return callback_val == 0? status::FAILED : status::OK;
}

db::status db::Db_manager::if_user_exists(std::string name) {
    cmd = "SELECT name FROM UserList WHERE name='";
    cmd += name + "'";
    int callback_val = 0;
    err = sqlite3_exec(database, cmd.c_str(), count_result, (void*) (&callback_val), &errmsg);
    CHECK;
    if (callback_val > 0)
        return status::USER_EXISTS;
    return status::USER_NOT_EXISTS;
}

db::status db::Db_manager::create_friend_request(std::string source, std::string destination) {
    cmd = "INSERT INTO FriendRequest (source, destination) VALUES ('";
    cmd += source + "', '" + destination + "')";
    err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
    CHECK;
    fprintf(stderr, "Db_manager.create_friend_request : %d\n", err);
    if (err == 19)
        return status::DUPLICATED_REQUEST;
    return status::OK;
}

db::status db::Db_manager::get_friend_request_list(std::string to, std::vector<std::string>& list) {
    cmd = "SELECT source FROM FriendRequest WHERE destination ='";
    cmd += to + "'";
    err = sqlite3_exec(database, cmd.c_str(), get_request_list, &list, &errmsg);
    CHECK;
    return status::OK;
}

db::status db::Db_manager::confirm_friend_request(std::string to, std::string source) {

    cmd = "DELETE FROM FriendRequest WHERE destination ='";
    cmd += to + "'";
    cmd += " AND source = '" + source + "'";
    err = sqlite3_exec(database, cmd.c_str(), 0, 0, &errmsg);
    CHECK;

    cmd = "SELECT friendList FROM UserInfo WHERE name = '";
    cmd += to + "'";
    std::string s;
    err = sqlite3_exec(database, cmd.c_str(), get_request_string, &s, &errmsg);
    CHECK;
    fprintf(stderr, "confirm_friend_request1 : %s\n", s.c_str());
    std::vector<std::string> fl = split_string(s, ",");

    fl.push_back(source);
    s = merge_string(fl, ",");
    fprintf(stderr, "confirm_friend_request2 : %s\n", s.c_str());

    cmd = "UPDATE UserInfo SET friendList ='";
    cmd += s + "' WHERE name = '" + to + "'";
    fprintf(stderr, "confirm_friend_request3 : %s\n", cmd.c_str());
    err = sqlite3_exec(database, cmd.c_str(), get_request_string, &s, &errmsg);
    CHECK;

    /* second time */

    cmd = "SELECT friendList FROM UserInfo WHERE name = '";
    cmd += source + "'";
    s = "";
    err = sqlite3_exec(database, cmd.c_str(), get_request_string, &s, &errmsg);
    CHECK;
    fl = split_string(s, ",");

    fl.push_back(to);
    s = merge_string(fl, ",");

    cmd = "UPDATE UserInfo SET friendList ='";
    cmd += s + "' WHERE name = '" + source + "'";
    err = sqlite3_exec(database, cmd.c_str(), get_request_string, &s, &errmsg);
    CHECK;

    return status::OK;
}
