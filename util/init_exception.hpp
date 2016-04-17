#pragma once

#include <exception>
#include <string>

struct InitException : public std::exception {
  InitException(const std::string& what) : what(what) {
    LOG_ERROR(what);
  }
  std::string what;
};
