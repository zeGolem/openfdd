#include "utils.hpp"
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>

namespace utils
{

namespace daemon
{

void log(std::string const& message, log_level ll)
{
	syslog(ll, "%s", message.c_str());
}

void exit_error(std::string const& error)
{
	log(error, log_level::error);
	exit(EXIT_FAILURE);
}

// Inspired from https://gist.github.com/alexdlaird/3100f8c7c96871c5b94e
void become()
{
	// fork() splits the process in two. Both will run the same code, but pid
	// will be 0 for the child and a positive value for the parent.
	auto pid = fork();

	if (pid > 0)            // We're running in the parent
		exit(EXIT_SUCCESS); // Terminate the parent, orphaning the child.

	if (pid < 0) // There was an error forking
		exit(EXIT_FAILURE);

	// Now we only have an orphaned child. This is our daemon.

	// Not sure what this really does, but it gives us permission to write to
	// the log files
	umask(0);

	// We can now open a log file
	openlog("openfddd", LOG_NOWAIT | LOG_PID, LOG_USER);
	log("Daemon started!");

	if (setsid() < 0) // setsid() creates a new session to detach the process
	                  // from the terminal.
	                  // If it's < 0, there was an error
		exit_error("Failed to create a new session");

	if (chdir("/") < 0) // We move to the root, to be sure not to bother anyone
		exit_error("Failed to chdir(\"/\")");

	// Close the terminal FDs
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

} // namespace daemon

int stoi_safe(std::string input,
              stoi_checks checks,
              std::string value_name_for_error)
{
	int converted = 0;
	try {
		converted = std::stoi(input);
	} catch (std::invalid_argument const& e) {
		throw std::runtime_error(value_name_for_error +
		                         " should be a number (got: " + input + ")");
	}

	if (checks.min.has_value() && converted < checks.min.value())
		throw std::runtime_error(value_name_for_error + " should be >" +
		                         std::to_string(checks.min.value()));
	if (checks.max.has_value() && converted > checks.max.value())
		throw std::runtime_error(value_name_for_error + " should be <" +
		                         std::to_string(checks.max.value()));

	return converted;
}

std::vector<std::string> split(std::string const& input, char delimiter)
{
	std::stringstream stream(input);

	std::vector<std::string> result{};

	std::string part;
	while (std::getline(stream, part, delimiter))
		result.push_back(part);

	return result;
}

} // namespace utils
