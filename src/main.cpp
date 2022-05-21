#include "config.hpp"
#include "drivers/driver.hpp"
#include "drivers/manager.hpp"
#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "drivers/steelseries/apex_100.hpp"
#include "drivers/steelseries/rival_3_wireless.hpp"
#include "unix_socket.hpp"
#include "usb.hpp"
#include "usb/device_manager.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// TODO: Make this user-definable!
constexpr auto SOCKET_PATH = "/var/run/openfdd.socket";

enum command_result {
	success,
	failure,
};

typedef std::function<command_result(
    std::shared_ptr<socket_connection>      connection,
    drivers::identifiable_driver_map const& drivers,
    std::vector<std::string>                argv)>

    socket_command_handler;

std::unordered_map<std::string, socket_command_handler>
get_socket_command_handlers()
{
#define DEFINE_SOCKET_COMMAND(name)                                            \
	socket_command_handler name =                                              \
	    [](std::shared_ptr<socket_connection>      connection,                 \
	       drivers::identifiable_driver_map const& drivers,                    \
	       std::vector<std::string>                argv) -> command_result

	DEFINE_SOCKET_COMMAND(ping)
	{
		(void)drivers;
		(void)argv;
		connection->write_string("pong\n");
		return command_result::success;
	};

	DEFINE_SOCKET_COMMAND(list_devices)
	{
		(void)argv;
		for (auto const& [id, driver] : drivers)
			connection->write_string(id.stringify() + "," + driver->name() +
			                         '\n');
		return command_result::success;
	};

	DEFINE_SOCKET_COMMAND(list_actions)
	{
		if (argv.size() < 2) {
			// TODO: This should throw an error, but there's no proper error
			//       handling yet...
			// TODO: Handle errors thrown

			connection->write_string("fail,Not enough arguements\n");
			return command_result::failure;
		}

		auto const& driver_id = usb_device::identifier::from(argv[1]);
		if (!drivers.contains(driver_id)) {
			connection->write_string(
			    "fail,Driver not found (got: " + driver_id.stringify() + ")\n");
			return command_result::failure;
		}

		auto const& driver = drivers.at(driver_id);

		for (auto const& [action_id, action] : driver->get_actions())
			connection->write_string(action_id + ',' +
			                         utils::escape_commas(action.description) +
			                         '\n');
		return command_result::success;
	};

	DEFINE_SOCKET_COMMAND(list_action_params)
	{
		if (argv.size() < 3) {
			// TODO: This should throw an error, but there's no proper error
			//       handling yet...
			// TODO: Handle errors thrown

			connection->write_string("fail,Not enough arguements\n");
			return command_result::failure;
		}

		auto const& driver_id = usb_device::identifier::from(argv[1]);
		if (!drivers.contains(driver_id)) {
			connection->write_string(
			    "fail,Driver not found (got: " + driver_id.stringify() + ")\n");
			return command_result::failure;
		}

		auto const& driver = drivers.at(driver_id);

		auto const& action_id = argv[2];
		auto const& actions   = driver->get_actions();

		if (!actions.contains(action_id)) {
			connection->write_string("fail,Action not found\n");
			return command_result::failure;
		}

		auto const& action = actions.at(action_id);

		for (auto const& param : action.parameters)
			connection->write_string(param.name + ',' +
			                         utils::escape_commas(param.description) +
			                         '\n');

		return command_result::success;
	};

	DEFINE_SOCKET_COMMAND(action_run)
	{
		if (argv.size() < 3) {
			// TODO: This should throw an error, but there's no proper error
			//       handling yet...
			// TODO: Handle errors thrown

			connection->write_string("fail,Not enough arguements\n");
			return command_result::failure;
		}

		auto const& driver_id = usb_device::identifier::from(argv[1]);
		if (!drivers.contains(driver_id)) {
			connection->write_string(
			    "fail,Driver not found (got: " + driver_id.stringify() + ")\n");
			return command_result::failure;
		}

		auto const& driver = drivers.at(driver_id);

		auto const& action_id = argv[2];
		auto const& actions   = driver->get_actions();

		if (!actions.contains(action_id)) {
			connection->write_string("fail,Action not found\n");
			return command_result::failure;
		}

		auto const& action = actions.at(action_id);

		// This checks if there are enough arguements
		if (argv.size() < action.parameters.size() + 3) {
			connection->write_string("fail,Not enough arguements\n");
			return command_result::failure;
		}

		std::vector<std::string> action_params(argv.begin() + 3, argv.end());

		driver->run_action(action_id, action_params);

		return command_result::success;
	};

#undef DEFINE_SOCKET_COMMAND

	return {
	    {              "ping",               ping},
	    {      "list-devices",       list_devices},
	    {      "list-actions",       list_actions},
	    {"list-action-params", list_action_params},
	    {        "action-run",         action_run},
	};
}

void handle_socket_connection(std::shared_ptr<socket_connection> connection,
                              drivers::identifiable_driver_map const& drivers)
{
	utils::daemon::log("New connection");

	connection->write_string("openfdd\n");

	while (true) {
		auto const& read_result = connection->read_line();

		if (read_result.connection_is_over) return;

		auto const& input = read_result.data;
		auto const& input_argv =
		    utils::split(input, ','); // TODO: Better parsing

		auto const& command = input_argv[0];

		auto const& command_handlers = get_socket_command_handlers();

		if (!command_handlers.contains(command)) {
			connection->write_string("fail,No such command\n");
			continue;
		}

		if (command_handlers.at(command)(connection, drivers, input_argv) ==
		    command_result::success)
			connection->write_string("done\n");
	}
}

void daemon_main()
{
	utils::daemon::become();

	usb_context         ctx;
	usb::device_manager dev_manager(ctx);
	drivers::manager    drv_manager(dev_manager);

	auto drivers = drv_manager.create_drivers_for_available_devices();

	// TODO: Handle disconects! VERY IMPORTANT -> MEMORY LEAK!!
	std::vector<std::shared_ptr<socket_connection>> connections;

	drv_manager.start_hotplug_support(
	    [&drivers, &connections](auto new_driver_list) {
		    drivers = new_driver_list;

		    for (auto const& connection : connections) {
			    // TODO: This is a hack, need to remove closed connection
			    if (!connection->opened()) continue;
			    // TODO: Find a better/more generic way to do this!
			    connection->write_string("notify,hotplug\n");
		    }
	    });

	try {
		unix_socket socket(SOCKET_PATH);

		socket.listen();
		socket.wait_for_connection_and_accept([&drivers,
		                                       &connections](auto connection) {
			// TODO: Poor man's error handling, should be changed into a more
			//       robust solution
			try {
				connections.push_back(connection);
				handle_socket_connection(connection, drivers);
			} catch (std::runtime_error& e) {
				utils::daemon::log(e.what(), utils::daemon::log_level::error);
			}
		});
	} catch (std::runtime_error const& e) {
		utils::daemon::exit_error(e.what());
	}
}

int main(int argc, char** argv)
{
	if (argc > 1 && argv[1] == std::string("--replace")) {
		std::cout << "Killing old daemon\n";
		utils::kill_all("openfdd");
		std::filesystem::remove(SOCKET_PATH);
	}
	daemon_main();
}
