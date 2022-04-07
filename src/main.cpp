#include "config.hpp"
#include "drivers/driver.hpp"
#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "drivers/steelseries/apex_100.hpp"
#include "drivers/steelseries/rival_3_wireless.hpp"
#include "usb.hpp"
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
#define check_driver(drv)                                                      \
	if (drv::is_compatible(dev)) return std::make_shared<drv>(dev, config);

	check_driver(drivers::steelseries::apex_100);
	check_driver(drivers::steelseries::rival_3_wireless);
	check_driver(drivers::steelseries::aerox_3_wireless);
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

	for (auto const& dev : devices) {
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
		std::cout << " - [" << i << "] ->" << driver->name() << '\n';

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

	// Copy all the provided params to be passed to the action
	// Starting at argv[3] because the previous values have already been read
	for (std::size_t i = 0; i < action_parameter_count; ++i)
		action_params[i] = argv[i + 3];

	driver->run_action(action_id, action_params);

	return 0;
}

int main(int argc, char** argv)
{
	usb_context ctx;
	ctx.log_level(3);

	auto drivers = get_drivers_for_connected_devices(ctx);

	try {
		if (argc == 1) return show_available_devices(drivers);
		if (argc > 2) return handle_actions(drivers, argc, argv);
	} catch (std::runtime_error const& e) {
		std::cerr << "Runtime error: " << e.what() << "\n";
		return -1;
	}
}
