#include "manager.hpp"
#include "drivers/driver.hpp"
#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "drivers/steelseries/apex_100.hpp"
#include "drivers/steelseries/rival_3_wireless.hpp"
#include "usb.hpp"
#include <memory>

namespace drivers
{

std::optional<std::shared_ptr<driver>> manager::create_driver_if_available(
    std::shared_ptr<usb_device> device) const
{
#define CHECK_DRIVER(drv)                                                      \
	if (drv::is_compatible(device))                                            \
	return std::make_shared<drv>(device, m_config_manager)

	CHECK_DRIVER(steelseries::aerox_3_wireless);
	CHECK_DRIVER(steelseries::rival_3_wireless);
	CHECK_DRIVER(steelseries::apex_100);

	return {};
}

identifiable_driver_map manager::create_drivers_for_available_devices() const
{
	identifiable_driver_map map = {};

	for (auto const& [identifier, device] : m_device_manager.devices()) {
		auto const& new_driver = create_driver_if_available(device);
		if (new_driver.has_value()) map[identifier] = new_driver.value();
	}

	return map;
}

void manager::start_hotplug_support(
    std::function<void(identifiable_driver_map new_driver_list)>
        driver_list_updated_callback)
{
	m_device_manager.set_hotplug_notification([&driver_list_updated_callback,
	                                           this]() {
		driver_list_updated_callback(create_drivers_for_available_devices());
	});
	m_device_manager.handle_hotplugs();
}

} // namespace drivers
