#pragma once

#include "usb/context.hpp"
#include "usb/device.hpp"
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

namespace usb
{

class device_manager
{
  public:
	device_manager(usb::context const& context) : m_context(context)
	{
		m_device_list = context.get_devices();
	}

	inline void register_device(std::shared_ptr<usb::device> device)
	{
		m_device_list[device->get_address()] = device;
	}

	inline void unregister_device(usb::address address_of_device_to_remove)
	{
		m_device_list.erase(address_of_device_to_remove);
	}

	void handle_hotplugs();

	std::unordered_map<usb::address, std::shared_ptr<usb::device>> const&
	devices() const noexcept
	{
		return m_device_list;
	}

	void set_hotplug_notification(
	    std::function<void()> notification_callback) noexcept
	{
		m_hotplug_notification = notification_callback;
	}

  private:
	usb::context const& m_context;
	std::unordered_map<usb::address, std::shared_ptr<usb::device>>
	    m_device_list;

	std::unique_ptr<std::thread> m_hotplug_handling_thread;
	std::function<void()>        m_hotplug_notification;
};

} // namespace usb
