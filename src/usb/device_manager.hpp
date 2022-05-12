#pragma once

#include "usb.hpp"
#include <memory>
#include <vector>

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
		m_device_list.push_back(device);
	}

	void handle_hotplugs() const;

  private:
	std::shared_ptr<usb_context>             m_context;
	std::vector<std::shared_ptr<usb_device>> m_device_list;
};

} // namespace usb
