#include <iostream>
#include <map>
#include <vector>

struct CmdOption {
  std::string name;
  int arguments;
  std::string short_desc;
  std::string long_desc;
};

bool options_get(std::map<std::string, std::vector<std::string> >& parsed_opts, int argc, char** argv, std::vector<CmdOption> options);
