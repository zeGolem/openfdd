#include "usb.hpp"

#include <cstddef>
#include <libusb-1.0/libusb.h>
#include <memory>
#include <stdexcept>
#include <vector>

void usb_device::open()
{
	if (is_opened()) return;

	if (libusb_open(m_device, &m_handle) != 0)
		throw std::runtime_error("Couldn't open device");
}

usb_device::id usb_device::get_id() const
{
	libusb_device_descriptor descriptor;

	if (libusb_get_device_descriptor(m_device, &descriptor) != 0)
		throw std::runtime_error("Couldn't get device descriptor");

	return {descriptor.idVendor, descriptor.idProduct};
}

int usb_device::control_transfer(std::uint8_t              request_type,
                                 std::uint8_t              b_request,
                                 std::uint16_t             w_value,
                                 std::uint16_t             w_index,
                                 std::vector<std::uint8_t> data,
                                 std::uint16_t             timeout) const
{
	if (!is_opened()) throw std::runtime_error("Device not opened!");

	libusb_detach_kernel_driver(m_handle, w_index);

	auto size = libusb_control_transfer(m_handle,
	                                    request_type,
	                                    b_request,
	                                    w_value,
	                                    w_index,
	                                    data.data(),
	                                    data.size(),
	                                    timeout);

	libusb_attach_kernel_driver(m_handle, w_index);
	return size;
}

usb_context::usb_context()
{
	if (libusb_init(&m_context) < 0)
		throw std::runtime_error("Can't initialize libusb!");
}

void usb_context::log_level(std::uint8_t level) const noexcept
{
	libusb_set_option(m_context, LIBUSB_OPTION_LOG_LEVEL, level);
}

std::shared_ptr<usb_device> usb_context::get_device(
    std::uint16_t vendor_id, std::uint16_t product_id) const
{
	auto* handle =
	    libusb_open_device_with_vid_pid(m_context, vendor_id, product_id);
	if (!handle) throw std::runtime_error("Couldn't open device!");

	return std::make_shared<usb_device>(usb_device(handle));
}

std::vector<std::shared_ptr<usb_device>> usb_context::get_devices() const
{
	libusb_device** native_list;
	auto device_count = libusb_get_device_list(m_context, &native_list);

	if (device_count < 0)
		throw std::runtime_error("Couldn't get the device list");

	std::vector<std::shared_ptr<usb_device>> devices(device_count);

	// Casting is kinda ugly, but avoids a warning about types.
	// It's fine to ignore it because we already checked device_count >= 0
	for (std::size_t i = 0; i < (std::size_t)device_count; ++i)
		devices[i] = std::make_shared<usb_device>(native_list[i]);

	libusb_free_device_list(native_list, 1);

	return devices;
}

void usb_context::register_hotplug_callback(
    const hotplug_event&       events_to_register,
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
