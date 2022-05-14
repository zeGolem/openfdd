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
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// TODO: Make this user-definable!
constexpr auto SOCKET_PATH = "/var/run/openfdd.socket";

void handle_socket_connection(std::shared_ptr<socket_connection> connection,
                              drivers::identifiable_driver_map const& drivers)
{
	utils::daemon::log("New connection");

	connection->write_string("openfdd\n");

	while (true) {
		auto const& input = connection->read_line();
		auto const& input_argv =
		    utils::split(input, ','); // TODO: Better parsing

		auto const& command = input_argv[0];

		if (command == "ping")
			connection->write_string("pong\n");

		else if (command == "list-devices") {
			// For each driver available
			for (auto const& [id, driver] : drivers) {
				connection->write_string(id.stringify() + "," + driver->name() +
				                         '\n');
			}
		}

		else if (command == "list-actions") {
			if (input_argv.size() < 2) {
				// TODO: This should throw an error, but there's no error
				//       handling yet...
				// TODO: Handle errors thrown

				connection->write_string("fail,Not enough arguements\n");
				continue;
			}

			auto const& driver_id = usb_device::identifier::from(input_argv[1]);

			auto const& driver = drivers.at(driver_id);

			for (auto const& [action_id, action] : driver->get_actions())
				connection->write_string(action_id + ',' + action.description +
				                         '\n');
		}

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

	try {
		unix_socket socket(SOCKET_PATH);

		socket.listen();
		socket.wait_for_connection_and_accept([drivers](auto connection) {
			handle_socket_connection(connection, drivers);
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
