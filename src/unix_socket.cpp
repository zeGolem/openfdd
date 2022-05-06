#include "unix_socket.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

socket_connection::socket_connection(int fd) : m_fd(fd) {}

unix_socket::unix_socket(std::string const& path)
{
	m_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (m_fd < 0) throw std::runtime_error("Couldn't create socket!");

	sockaddr_un addr{
	    .sun_family = AF_UNIX,
	    .sun_path   = {},
	};

	strncpy(addr.sun_path,
	        path.c_str(),
	        path.length()); // Working with C APIs is wonderful isn't it?

	auto bind_result = bind(m_fd, (sockaddr*)&addr, sizeof(addr));
	if (bind_result < 0) throw std::runtime_error("Couldn't bind socket");
}

void unix_socket::listen() const
{
	if (::listen(m_fd, 0) < 0)
		throw std::runtime_error("Couldn't listen for new clients!");
}

void unix_socket::wait_for_connection_and_accept(
    std::function<void(std::shared_ptr<socket_connection>)> handler)
{
	while (true) {
		auto client_fd  = accept(m_fd, nullptr, nullptr);
		auto connection = std::make_shared<socket_connection>(client_fd);

		m_connections.push_back(
		    std::make_unique<std::thread>(handler, connection));
	}
}
