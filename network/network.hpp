#pragma once

#include <string>

namespace Network {
  
  void start(std::string port);
  void stop();
  void run();

  unsigned int allocate_queue();
  void send_message(const std::string& msg, unsigned int queue);
  
}


  
