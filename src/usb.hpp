#pragma once

#include "utils.hpp"
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <libusb-1.0/libusb.h>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class usb_device
{
  public:
	// Identifies a USB device plugged into the computer
	struct identifier {
		std::uint8_t bus;
		std::uint8_t device;

		std::string const stringify() const noexcept
		{
			// TODO: This is a hack until we get std::format. String formatting
			//       in c++ sucks without it.
			char buffer[8] = {0};

			std::snprintf(buffer, sizeof(buffer), "%03u:%03u", bus, device);

			return std::string(buffer);
		}

		static identifier from(std::string const& identifier_string)
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

	struct id {
		std::uint16_t id_vendor;
		std::uint16_t id_product;
	};

	usb_device(libusb_device_handle* dev_handle) noexcept : m_handle(dev_handle)
	{
		m_device = libusb_get_device(dev_handle);
	}

	usb_device(libusb_device* device) noexcept : m_device(device) {}

	~usb_device() noexcept
	{
		if (is_opened()) libusb_close(m_handle);
		m_handle = nullptr;
	}

	bool is_opened() const noexcept { return m_handle; }
	void open();

	id get_id() const;

	// TODO: Difference between this and id? Are both needed?
	identifier get_identifier() const noexcept;

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

class usb_context
{
	struct hotplug_event {
		bool arrived = false;
		bool left    = false;
	};

  public:
	// TODO: Is there a better place for this?
	static bool supports_hotplug() noexcept
	{
		return libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
	}

	usb_context();

	~usb_context() noexcept { libusb_exit(m_context); }

	void log_level(std::uint8_t level) const noexcept;

	std::shared_ptr<usb_device> get_device(std::uint16_t vendor_id,
	                                       std::uint16_t product_id) const;

	std::unordered_map<usb_device::identifier, std::shared_ptr<usb_device>>
	get_devices() const;

	// TODO: This feels a bit too C-y with the libusb_hotplug_callback_fn &
	//       void*. Are there way to wrap those into C++ types?
	//       std::function<libusb_hotplug_callback_fn> doesn't work because
	//       it's an incomplete type...
	void register_hotplug_callback(hotplug_event const& events_to_register,
	                               libusb_hotplug_callback_fn,
	                               void* data) const;

	void wait_for_event() const;

  private:
	libusb_context* m_context;
};

// Hashing fucntion to use usb_device::identifier as a key in a map
// TODO: Is there a better place for this?
// (shamelessly stolent from: https://stackoverflow.com/a/17017281/16158640)
template <> struct std::hash<usb_device::identifier> {
	std::size_t operator()(usb_device::identifier const& id) const noexcept
	{
		std::size_t h1 = std::hash<std::uint8_t>{}(id.bus);
		std::size_t h2 = std::hash<std::uint8_t>{}(id.device);
		return h1 ^ (h2 << 1);
	}
};

// TODO: This shoud most likely be somewhere else...
inline bool operator==(const usb_device::identifier& lhs,
                       const usb_device::identifier& rhs)
{
	return lhs.bus == rhs.bus && lhs.device == rhs.device;
}
