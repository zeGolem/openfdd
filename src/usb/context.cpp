#include "context.hpp"
#include "usb/device.hpp"

namespace usb
{

context::context()
{
	if (libusb_init(&m_context) < 0)
		throw std::runtime_error("Can't initialize libusb!");
}

void context::log_level(std::uint8_t level) const noexcept
{
	libusb_set_option(m_context, LIBUSB_OPTION_LOG_LEVEL, level);
}

std::shared_ptr<device> context::get_device(std::uint16_t vendor_id,
                                            std::uint16_t product_id) const
{
	auto* handle =
	    libusb_open_device_with_vid_pid(m_context, vendor_id, product_id);
	if (!handle) throw std::runtime_error("Couldn't open device!");

	return std::make_shared<device>(device(handle));
}

std::unordered_map<address, std::shared_ptr<device>> context::get_devices()
    const
{
	libusb_device** native_list;
	auto device_count = libusb_get_device_list(m_context, &native_list);

	if (device_count < 0)
		throw std::runtime_error("Couldn't get the device list");

	std::unordered_map<address, std::shared_ptr<device>> devices;

	// Casting is kinda ugly, but avoids a warning about types.
	// It's fine to ignore it because we already checked device_count >= 0
	for (std::size_t i = 0; i < (std::size_t)device_count; ++i) {
		auto const& dev             = std::make_shared<device>(native_list[i]);
		devices[dev->get_address()] = dev;
	}

	libusb_free_device_list(native_list, 1);

	return devices;
}

void context::register_hotplug_callback(const hotplug_event& events_to_register,
                                        libusb_hotplug_callback_fn callback,
                                        void*                      data) const
{
	auto event_flag = 0;
	event_flag |=
	    events_to_register.arrived ? LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED : 0;
	event_flag |=
	    events_to_register.left ? LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT : 0;

	auto result = libusb_hotplug_register_callback(m_context,
	                                               event_flag,
	                                               0,
	                                               LIBUSB_HOTPLUG_MATCH_ANY,
	                                               LIBUSB_HOTPLUG_MATCH_ANY,
	                                               LIBUSB_HOTPLUG_MATCH_ANY,
	                                               callback,
	                                               data,
	                                               NULL);

	if (result != LIBUSB_SUCCESS)
		throw std::runtime_error("Can't setup hotplug handler");
}

void context::wait_for_event() const
{
	auto result = libusb_handle_events(m_context);
	if (result != LIBUSB_SUCCESS)
		throw std::runtime_error("Couldn't handle events!");
}

} // namespace usb
