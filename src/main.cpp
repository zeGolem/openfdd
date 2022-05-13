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
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

std::shared_ptr<drivers::driver> get_driver_if_available(
    std::shared_ptr<usb_device> dev, std::shared_ptr<config_manager> config)
{
#define CHECK_DRIVER(drv)                                                      \
	if (drv::is_compatible(dev)) return std::make_shared<drv>(dev, config);

	CHECK_DRIVER(drivers::steelseries::apex_100);
	CHECK_DRIVER(drivers::steelseries::rival_3_wireless);
	CHECK_DRIVER(drivers::steelseries::aerox_3_wireless);
#undef CHECK_DRIVER

	return nullptr;
}

std::vector<std::shared_ptr<drivers::driver>> get_drivers_for_connected_devices(
    usb_context const& ctx)
{
	auto devices = ctx.get_devices();

	// TODO: Is there a better place for this?
	auto config = std::make_shared<config_manager>(
	    config_manager::get_default_filepath());

	std::vector<std::shared_ptr<drivers::driver>> drivers;

	for (auto const& [_, dev] : devices) {
		if (auto driver = get_driver_if_available(dev, config)) {
			dev->open();
			drivers.push_back(driver);
		}
	}

	return drivers;
}

int show_available_devices(
    std::vector<std::shared_ptr<drivers::driver>> const& drivers)
{
	std::cout << "Drivers available:\n";
	for (std::size_t i = 0; i < drivers.size(); ++i) {
		auto const& driver = drivers[i];
		std::cout << " - [" << i << "] -> " << driver->name() << '\n';

		auto const& driver_actions = driver->get_actions();
		for (auto const& [action_id, action] : driver_actions) {
			std::cout << "   - " << action_id << ": " << action.description
			          << '\n';
			if (action.parameters.size() > 0)
				std::cout << "     Parameters: \n";
			for (auto const& param : action.parameters) {
				std::cout << "      " << param.name << " - "
				          << param.description << '\n';
			}

			std::cout << '\n';
		}

		std::cout << '\n';
	}

	return 0;
}

int handle_actions(std::vector<std::shared_ptr<drivers::driver>> const& drivers,
                   int                                                  argc,
                   char**                                               argv)
{
	std::size_t driver_id;
	try {
		driver_id = std::stoi(argv[1]);
	} catch (std::invalid_argument const& e) {
		throw std::runtime_error("Wrong driver id.");
	}

	auto const& driver    = drivers[driver_id];
	std::string action_id = argv[2];

	auto const& actions = driver->get_actions();
	if (!actions.contains(action_id))
		throw std::runtime_error("Unexpected action: " + action_id);

	std::uint32_t action_parameter_count =
	    argc - 3; // argv[1] is the driver id, argv[2] is the action id.
	std::vector<std::string> action_params(action_parameter_count);

void handle_socket_connection(std::shared_ptr<socket_connection> connection,
                              drivers::identifiable_driver_map   drivers)
{
	utils::daemon::log("New connection");

	connection->write_string("openfdd\n");

	while (true) {
		auto const& input = connection->read_line();
		auto const& input_argv =
		    utils::split(input, ' '); // TODO: Better parsing
		                              // (support spaces and quotes)

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

				connection->write_string("Not enough arguements\n");
				connection->write_string("FAIL\n");
				continue;
			}

			auto const& driver_id = usb_device::identifier::from(input_argv[1]);

			auto const& driver = drivers[driver_id];

			for (auto const& [action_id, action] : driver->get_actions())
				connection->write_string(action_id + ':' + action.description +
				                         '\n');
		}

		connection->write_string("DONE\n");
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
		unix_socket socket("/var/run/openfdd.socket");

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
	if (argc > 1 && argv[1] == std::string("daemon")) {
		daemon_main();
		return 0;
	}

	usb_context ctx;
	ctx.log_level(3);

	usb::device_manager dev_manager(ctx);
	drivers::manager    drv_manager(dev_manager);

	auto drivers = drv_manager.create_drivers_for_available_devices();

	// TODO: Hack because the rest of the code doesn't yet understand driver
	//       maps

	std::vector<std::shared_ptr<drivers::driver>> driver_list = {};
	for (auto const& [identifier, driver] : drivers)
		driver_list.push_back(driver);

	try {
		if (argc == 1) return show_available_devices(driver_list);
		if (argc > 2) return handle_actions(driver_list, argc, argv);
	} catch (std::runtime_error const& e) {
		std::cerr << "Runtime error: " << e.what() << "\n";
		return -1;
	}
}
