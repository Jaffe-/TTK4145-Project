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

class CmdOptions {
public:
  CmdOptions(int argc, char** argv, std::vector<CmdOptionDesc>&& options_descs);

  bool has(const std::string& name) const;
  std::string operator[](const std::string& name);

private:
  std::map<std::string, std::string> options;
};
