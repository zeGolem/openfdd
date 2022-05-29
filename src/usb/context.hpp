#pragma once
#include "device.hpp"
#include <libusb-1.0/libusb.h>
#include <memory>
#include <unordered_map>

namespace usb
{

class context
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

	context();

	~context() noexcept { libusb_exit(m_context); }

	void log_level(std::uint8_t level) const noexcept;

	std::shared_ptr<device> get_device(std::uint16_t vendor_id,
	                                   std::uint16_t product_id) const;

	std::unordered_map<address, std::shared_ptr<device>> get_devices() const;

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

} // namespace usb
