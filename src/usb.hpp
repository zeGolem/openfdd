#pragma once

#include <cstdint>
#include <libusb-1.0/libusb.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

class usb_device
{
  public:
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

	// TODO: Find better arguements to pass (request direction, recipient, ...)
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

	std::vector<std::shared_ptr<usb_device>> get_devices() const;

	// TODO: This feels a bit too C-y with the libusb_hotplug_callback_fn &
	//       void*. Are there way to wrap those into C++ types?
	//       std::function<libusb_hotplug_callback_fn> doesn't work because
	//       it's an incomplete type...
	void register_hotplug_callback(hotplug_event const& events_to_register,
	                               libusb_hotplug_callback_fn,
	                               void* data) const;

  private:
	libusb_context* m_context;
};
