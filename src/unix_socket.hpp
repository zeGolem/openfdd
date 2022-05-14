#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class socket_connection
{
  public:
	struct read_result {
		std::string const data;
		bool              connection_is_over = false;
	};

	socket_connection(int fd);

	read_result const read_line();
	void              write_string(std::string const&) const;

	bool opened() const noexcept { return m_opened; }

  private:
	int  m_fd;
	bool m_opened;
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
