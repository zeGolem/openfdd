#pragma once

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
	    : m_device(dev), m_config(config){};

	static bool is_compatible(usb_device*) { return false; }

	virtual const std::string name() const noexcept = 0;

	virtual const std::unordered_map<std::string, action const> get_actions()
	    const noexcept = 0;

	virtual void run_action(
	    std::string const&              option_id,
	    std::vector<std::string> const& parameters) const = 0;

  protected:
	std::shared_ptr<usb_device>     m_device;
	std::shared_ptr<config_manager> m_config;
};

} // namespace drivers
