#include <iostream>
#include <map>
#include <vector>

struct CmdOptionDesc {
  std::string name;
  bool takes_arguments;
  bool optional;
  std::string argumentname;
  std::string description;
};

using CmdOptions = std::map<std::string, std::string >;

bool cmd_options_get(CmdOptions& parsed_opts, int argc, char** argv,
		     std::vector<CmdOptionDesc>&& options);
