#pragma once
#include "libusb-1.0/libusb.h"
#include "utils.hpp"
#include <functional>
#include <string>

namespace usb
{

struct descriptor_id {
	std::uint16_t id_vendor;
	std::uint16_t id_product;
};

// Identifies a USB device plugged into the computer
struct address {
	std::uint8_t bus;
	std::uint8_t device;

	bool operator==(const usb::address& other) const
	{
		return bus == other.bus && device == other.device;
	}

	std::string const stringify() const noexcept
	{
		// TODO: This is a hack until we get std::format. String formatting
		//       in c++ sucks without it.
		char buffer[8] = {0};

		std::snprintf(buffer, sizeof(buffer), "%03u:%03u", bus, device);

		return std::string(buffer);
	}

	static address from(std::string const& identifier_string)
	{
		auto const& split_id = utils::split(identifier_string, ':');
		if (split_id.size() < 2) return {};

		std::uint8_t bus =
		    utils::stoi_safe(split_id[0], {.min = 0, .max = 256}, "Bus");
		std::uint8_t device =
		    utils::stoi_safe(split_id[1], {.min = 0, .max = 256}, "Device");

		return {bus, device};
	}
};

class device
{
  public:
	device(libusb_device_handle* dev_handle) noexcept : m_handle(dev_handle)
	{
		m_device = libusb_get_device(dev_handle);
	}

	device(libusb_device* device) noexcept : m_device(device) {}

	~device() noexcept
	{
		if (is_opened()) libusb_close(m_handle);
		m_handle = nullptr;
	}

	bool is_opened() const noexcept { return m_handle; }
	void open();

	descriptor_id get_descriptor_id() const;

	address get_address() const noexcept;

	// TODO: Find better arguements to pass (request direction, recipient,
	// ...)
	// TODO: Use enums when possible
	int control_transfer(std::uint8_t              request_type,
	                     std::uint8_t              b_request,
	                     std::uint16_t             w_value,
	                     std::uint16_t             w_index,
	                     std::vector<std::uint8_t> data,
	                     std::uint16_t             timeout) const;

  private:
	libusb_device_handle* m_handle = nullptr;
	libusb_device*        m_device;
};
} // namespace usb

// Hashing fucntion to use usb::address as a key in a map
// (shamelessly stolent from: https://stackoverflow.com/a/17017281/16158640)
template <> struct std::hash<usb::address> {
	std::size_t operator()(usb::address const& addr) const noexcept
	{
		std::size_t h1 = std::hash<std::uint8_t>{}(addr.bus);
		std::size_t h2 = std::hash<std::uint8_t>{}(addr.device);
		return h1 ^ (h2 << 1);
	}
};
