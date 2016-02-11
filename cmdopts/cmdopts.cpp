#include "cmdopts.hpp"
#include <algorithm>

static const std::string option_prefix = "--";

static void print_option_description(const CmdOptionDesc& option_desc)
{
  std::cout << option_prefix << option_desc.name;
  if (option_desc.takes_arguments)
    std::cout << " <" << option_desc.argumentname << ">";
  std::cout << " ";
}

static void print_usage(std::string prog_name,
			const std::vector<CmdOptionDesc>& descriptions)
{
  std::cout << "USAGE: " << prog_name << " ";
  for (auto& option_desc : descriptions) {
    if (!option_desc.optional)
      print_option_description(option_desc);
  }

  std::cout << "[";
  for (auto& option_desc : descriptions) {
    if (option_desc.optional)
      print_option_description(option_desc);
  }
  std::cout << "\b]" << std::endl << std::endl;

  for (auto& option_desc : descriptions) {
    std::cout << option_prefix << option_desc.name << "\t\t" << option_desc.description
	      << std::endl;
  }
  std::cout << std::endl;
}

static bool is_option(const std::string& s)
{
  return s.find(option_prefix) == 0;
}

static bool split_options(std::vector< std::vector<char*> >& optlist,
			  char** begin, char** end)
{
  std::vector< std::vector<char*> > result;
  char ** itr = begin;

  while (itr != end) {
    if (!is_option(*itr))
      return false;

    std::vector<char*> new_option;
    do {
      new_option.push_back(*itr++);
    } while (itr != end && !(is_option(*itr)));

    result.push_back(new_option);
  }

  optlist = result;
  return true;
}

bool cmd_options_get(CmdOptions& parsed_opts, int argc, char** argv,
		     std::vector<CmdOptionDesc>&& descriptions)
{
  std::vector< std::vector<char*> > option_lists;
  CmdOptions result;
  bool missing_option = false;

  if (!split_options(option_lists, argv + 1, argv + argc)) {
    std::cout << "ERROR: Malformed options" << std::endl;
    goto fail;
  }

  for (auto& option_list : option_lists) {
    std::string option_str = option_list[0];

    option_str.erase(0, option_prefix.length());
    auto it = std::find_if(descriptions.begin(), descriptions.end(),
			   [&](CmdOptionDesc& c)
			   { return c.name == option_str; });

    if (it == descriptions.end()) {
      std::cout << "ERROR: Unknown option " << option_prefix << option_str
		<< std::endl;
      goto fail;
    }
    else if (it->takes_arguments && option_list.size() != 2) {
      std::cout << "ERROR: " << option_prefix << option_str
		<< " takes one argument" << std::endl;
      goto fail;
    }
    else if (!it->takes_arguments && option_list.size() > 1) {
      std::cout << "ERROR: " << option_prefix << option_str
		<< " does not take any argument" << std::endl;
      goto fail;
    }
    else {
      result[option_str] = it->takes_arguments ? option_list[1] : "";
    }
  }

  for (auto& option_desc : descriptions) {
    if (!option_desc.optional) {
      auto it = std::find_if(result.begin(), result.end(),
			     [&](const std::pair<std::string, std::string>& opt)
			     { return option_desc.name == opt.first; });
      if (it == result.end()) {
	missing_option = true;
	std::cout << "ERROR: missing option " << option_prefix << option_desc.name
		  << std::endl;
      }
    }
  }
  if (missing_option) {
    goto fail;
  }

  parsed_opts = result;
  return true;

 fail:
  std::cout << std::endl;
  print_usage(argv[0], descriptions);
  return false;
}
