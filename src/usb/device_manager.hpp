#pragma once

#include "usb.hpp"
#include <memory>
#include <thread>
#include <unordered_map>

namespace usb
{

class device_manager
{
  public:
	device_manager(std::shared_ptr<usb_context> context) : m_context(context)
	{
		m_device_list = context->get_devices();
	}

	inline void register_device(std::shared_ptr<usb_device> device)
	{
		m_device_list[device->get_identifier()] = device;
	}

	inline void unregister_device(usb_device::identifier id_of_device_to_remove)
	{
		m_device_list.erase(id_of_device_to_remove);
	}

	void handle_hotplugs();

	std::unordered_map<usb_device::identifier,
	                   std::shared_ptr<usb_device>> const&
	devices() const noexcept
	{
		return m_device_list;
	}

  private:
	std::shared_ptr<usb_context> m_context;
	std::unordered_map<usb_device::identifier, std::shared_ptr<usb_device>>
	    m_device_list;

	std::unique_ptr<std::thread> m_hotplug_handling_thread;
};

} // namespace usb
