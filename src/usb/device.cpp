#include "device.hpp"

namespace usb
{

void device::open()
{
	if (is_opened()) return;

	if (libusb_open(m_device, &m_handle) != 0)
		throw std::runtime_error("Couldn't open device");
}

descriptor_id device::get_descriptor_id() const
{
	libusb_device_descriptor descriptor;

	if (libusb_get_device_descriptor(m_device, &descriptor) != 0)
		throw std::runtime_error("Couldn't get device descriptor");

	return {descriptor.idVendor, descriptor.idProduct};
}

address device::get_address() const noexcept
{
	return {.bus    = libusb_get_bus_number(m_device),
	        .device = libusb_get_device_address(m_device)};
}

int device::control_transfer(std::uint8_t              request_type,
                             std::uint8_t              b_request,
                             std::uint16_t             w_value,
                             std::uint16_t             w_index,
                             std::vector<std::uint8_t> data,
                             std::uint16_t             timeout) const
{
	if (!is_opened()) throw std::runtime_error("Device not opened!");

	libusb_detach_kernel_driver(m_handle, w_index);
	libusb_claim_interface(m_handle, w_index);

	auto size = libusb_control_transfer(m_handle,
	                                    request_type,
	                                    b_request,
	                                    w_value,
	                                    w_index,
	                                    data.data(),
	                                    data.size(),
	                                    timeout);

	libusb_release_interface(m_handle, w_index);
	libusb_attach_kernel_driver(m_handle, w_index);
	return size;
}

} // namespace usb
