#include "usb/device_manager.hpp"
#include "usb.hpp"
#include <libusb-1.0/libusb.h>
#include <memory>
#include <stdexcept>
#include <thread>

namespace usb
{

void device_manager::handle_hotplugs()
{
	if (!usb_context::supports_hotplug())
		throw std::runtime_error("Hotplug isn't supported!");

	m_context.register_hotplug_callback(
	    {.arrived = true, .left = true},
	    [](libusb_context*,
	       libusb_device*       device,
	       libusb_hotplug_event event,
	       void*                data) -> int {
		    auto this_ = (device_manager*)data;

		    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
			    this_->register_device(std::make_shared<usb_device>(device));
		    else
			    this_->unregister_device(usb_device(device).get_identifier());

		    if (this_->m_hotplug_notification) this_->m_hotplug_notification();

		    return 0;
	    },
	    (void*)this);

	// TODO: Error handling. usb_context::wait_for_event can throw.
	m_hotplug_handling_thread.reset(new std::thread([this]() {
		while (true)
			this->m_context.wait_for_event();
	}));
}

} // namespace usb
