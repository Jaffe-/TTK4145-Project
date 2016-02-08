#include "cmdopts.hpp"
#include <algorithm>

static const std::string option_prefix = "--";

static void print_usage(std::string prog_name, std::vector<CmdOption> options)
{
  std::cout << "USAGE: " << prog_name << " ";
  for (auto& option : options)
    std::cout << option_prefix << option.name << " " << option.short_desc << " ";
  std::cout << std::endl << std::endl;

  for (auto& option : options) {
    std::cout << option_prefix << option.name << "\t" << option.long_desc << std::endl;
  }
}

static bool is_option(std::string s)
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
		     std::vector<CmdOption>&& options)
{
  std::vector< std::vector<char*> > splitted;
  CmdOptions result;

  if (!split_options(splitted, argv + 1, argv + argc))
    goto fail;

  for (auto& option_list : splitted) {
    std::string option_str = option_list[0];

    option_str.erase(0, option_prefix.length());
    auto it = std::find_if(options.begin(), options.end(),
			   [&option_str](CmdOption& c)
			   { return c.name == option_str; });
    if (it == options.end()) {
      std::cout << "ERROR: Unknown option " << option_prefix << option_str
		<< std::endl << std::endl;
      goto fail;
    }
    else if (it->arguments != (int)option_list.size() - 1) {
      std::cout << "ERROR: " << option_prefix << option_str << " takes "
		<< it->arguments << " number of arguments"
		<< std::endl << std::endl;
      goto fail;
    }
    else {
      std::vector<std::string> arguments;
      std::copy(option_list.begin() + 1, option_list.end(),
		std::back_inserter(arguments));
      result[option_str] = arguments;
    }
  }

  parsed_opts = result;
  return true;

 fail:
  print_usage(argv[0], options);
  return false;
}
