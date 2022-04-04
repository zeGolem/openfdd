#pragma once

#include "3rd_party/json.hpp"
#include "config.hpp"
#include "usb.hpp"
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace drivers
{

struct parameter {
	enum param_type {
		param_uint,
		param_string,
	} type;
	std::string name;
	std::string description;
};

struct action {
	std::string            description;
	std::vector<parameter> parameters;
};

class driver
{
  public:
	driver(std::shared_ptr<usb_device>     dev,
	       std::shared_ptr<config_manager> config)
	    : m_device(dev), m_config_manager(config){};

	static bool is_compatible(usb_device*) { return false; }

	virtual std::string config_id() const noexcept = 0;

	virtual const std::string name() const noexcept = 0;

	virtual const std::unordered_map<std::string, action const> get_actions()
	    const noexcept = 0;

	virtual void run_action(std::string const&              option_id,
	                        std::vector<std::string> const& parameters) = 0;

  protected:
	virtual nlohmann::json serialize_current_config() const noexcept      = 0;
	virtual void deserialize_config(nlohmann::json const& config_on_disk) = 0;
	// TODO: What about sending the config to the device?
	// 		 Right now, we assume that the device and config file have the same
	//       content. Should there be a "device initialization" phase that sends
	//       over the config stored on disk to the device?

	void save_config() const
	{
		m_config_manager->update_config(config_id(),
		                                serialize_current_config());
	}

	std::shared_ptr<usb_device>     m_device;
	std::shared_ptr<config_manager> m_config_manager;
};

} // namespace drivers
