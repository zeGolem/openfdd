#include "utils.hpp"
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

void kill_all(std::string const& process_name)
{
	for (auto const& current_process_dir :
	     std::filesystem::directory_iterator("/proc/")) {

		std::ifstream cmdline_file(current_process_dir.path().string() +
		                           "/cmdline");

		std::string executable_name;
		cmdline_file >>
		    executable_name; // This works because the cmdline file contains
		                     // argv separated by NULL bytes, so reading until
		                     // the first \0 gets us argv[0]

		if (executable_name == process_name) {
			auto const& pid_string = current_process_dir.path().stem().string();
			if (kill(std::stoi(pid_string), SIGTERM) < 0)
				throw std::runtime_error("Couldn't kill process " + pid_string);
		}
	}
}

std::string escape_commas(std::string const& input)
{
	std::string output;

	for (auto const& chr : input) {
		if (chr == ',') output += '\\';
		output += chr;
	}

	return output;
}

} // namespace utils
