#include "connection_controller.hpp"

void ConnectionController::notify_pong(std::string ip) 
{
  connections[ip] = time(NULL);
}

void ConnectionController::run()
{

}
