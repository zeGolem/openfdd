#pragma once

#include "driver.hpp"
#include "usb.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace drivers
{
namespace steelseries
{

constexpr auto vendor_id = 0x1038;

class apex_100 : public driver
{
  public:
	enum backlight_pattern : std::uint8_t {
		static_ = 1,
		slow    = 2,
		medium  = 3,
		fast    = 4,
	};

	apex_100(std::shared_ptr<usb_device>     dev,
	         std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
	}

	static bool is_compatible(std::shared_ptr<usb_device>);

	const std::string name() const noexcept final
	{
		return "SteelSeries Apex 100";
	};
	const std::unordered_map<std::string, action const> get_actions()
	    const noexcept final;

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters) const final;

	void set_backlight_luminosity(std::uint8_t) const;
	void set_backlight_pattern(backlight_pattern) const;
	void set_polling_interval(std::uint8_t) const;
	void save() const;
};

class rival_3_wireless : public driver
{
  public:
	rival_3_wireless(std::shared_ptr<usb_device>     dev,
	                 std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		init_config();
	}

	static bool is_compatible(std::shared_ptr<usb_device>);

	static std::string get_id() noexcept
	{
		return std::to_string(steelseries::vendor_id) + ":1830";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Rival 3 Wireless";
	};

	// TODO: This should be on class driver
	void init_config() noexcept;

	const std::unordered_map<std::string, action const> get_actions()
	    const noexcept final;

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters) const final;

	void set_static_color(std::uint8_t r, std::uint8_t g, std::uint8_t b) const;
	void set_dpi(std::uint8_t               active_profile_id,
	             std::vector<std::uint16_t> dpi_profiles) const;
	void set_poll_interval(std::uint8_t interval) const;
	void set_powersaving_options(bool          ultra_power_saving,
	                             bool          smart_lighting,
	                             std::uint16_t sleep_time) const;
	void save() const;

  private:
	// TODO: Save to file!
	// TODO: Make a `class configurable_driver: public driver{}` with config
	//       saving options
	struct {
		std::vector<std::uint16_t> dpi_values{400, 800, 1200, 2400, 3200};
		std::uint8_t               active_profile          = 1;
		bool                       ultra_power_saving_mode = false;
		bool                       smart_lighting_mode     = true;
		std::uint16_t              sleep_time              = 300;
	} m_config;
};

} // namespace steelseries
} // namespace drivers
