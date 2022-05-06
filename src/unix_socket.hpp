#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class socket_connection
{
  public:
	socket_connection(int fd);

  private:
	int m_fd;
};

class unix_socket
{
  public:
	unix_socket(std::string const& path);
	void listen() const;
	void wait_for_connection_and_accept(
	    std::function<void(std::shared_ptr<socket_connection>)> handler);

  private:
	int m_fd;

	std::vector<std::unique_ptr<std::thread>> m_connections;
};
