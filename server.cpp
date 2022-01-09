#include "MultiClientChat.hpp"

int main(int argc, char *argv[]) {
    assert(argc > 1);
    int port = atoi(argv[1]);
    MultiClientChat mcc("0.0.0.0", port);
    if(mcc.init() != 0)
      return -1;
    mcc.run();
}

// std::vector<std::string> split_string(std::string s, std::string delim = " ") {
//   std::vector<std::string> ret;
//   int cur = 0, pos = 0;
//   while ((pos = s.find(delim, cur)) != -1) {
//     ret.push_back(s.substr(cur, pos - cur));
//     cur = pos + 1;
//   }
//   if (s.substr(cur) != "")
//     ret.push_back(s.substr(cur));
//   return ret;
// }

// std::string merge_string(std::vector<std::string> v, std::string delim = ",") {
//   std::string ret;
//   for (auto s : v)
//     ret += s + delim;
//   ret = ret.substr(0, std::max((int) ret.length() - (int)delim.length(), 0));
//   return ret;
// }
