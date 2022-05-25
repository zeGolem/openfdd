#pragma once

#include "3rd_party/json.hpp"
#include "config.hpp"
#include "usb.hpp"
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace drivers
{

struct parameter {
	enum type {
		uint,
		string,
		rgb_color,
		bool_,
	};

	struct uint_typeinfo {
		std::uint32_t min = std::numeric_limits<std::uint32_t>::min();
		std::uint32_t max = std::numeric_limits<std::uint32_t>::max();
	};

	type type;

	union {
		uint_typeinfo uint;
	} type_info = {};

	std::string name;
	std::string description;

	static std::string const type_to_string(enum type const&) noexcept;
};

struct action {
	std::string            name;
	std::string            description;
	std::vector<parameter> parameters;

	typedef std::function<void(std::vector<std::string> const&)> handler;
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

	const std::unordered_map<std::string, action const>& get_actions()
	    const noexcept
	{
		return m_actions;
	}

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters);

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

	void register_action(std::string const& id,
	                     action const&      the_action,
	                     action::handler const&);

	virtual void create_actions() noexcept = 0;

	std::shared_ptr<usb_device>     m_device;
	std::shared_ptr<config_manager> m_config_manager;

	std::unordered_map<std::string, action const>    m_actions;
	std::unordered_map<std::string, action::handler> m_action_handlers;
};

} // namespace drivers
