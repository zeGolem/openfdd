#pragma once

#include "config.hpp"
#include "drivers/driver.hpp"
#include "usb/device.hpp"
#include "usb/device_manager.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
namespace drivers
{

typedef std::unordered_map<usb::address, std::shared_ptr<driver>>
    identifiable_driver_map;

class manager
{
  public:
	manager(usb::device_manager& device_manager)
	    : m_device_manager(device_manager),
	      m_config_manager(std::make_shared<config_manager>(
	          config_manager::get_default_filepath()))
	{
	}

	std::optional<std::shared_ptr<driver>> create_driver_if_available(
	    std::shared_ptr<usb::device> device) const;

	identifiable_driver_map create_drivers_for_available_devices() const;

	void start_hotplug_support(
	    std::function<void(identifiable_driver_map& new_driver_list)>
	        driver_list_updated_callback);

  private:
	usb::device_manager&            m_device_manager;
	std::shared_ptr<config_manager> m_config_manager;
};

} // namespace drivers
