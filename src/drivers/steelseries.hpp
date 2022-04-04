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
		deserialize_config(config->get_device_config(config_id()));
	}

	static bool is_compatible(std::shared_ptr<usb_device>);

	std::string config_id() const noexcept final
	{
		return "steelseries:apex_100";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Apex 100";
	};

	const std::unordered_map<std::string, action const> get_actions()
	    const noexcept final;

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters) final;

	void set_backlight_luminosity(std::uint8_t) const;
	void set_backlight_pattern(backlight_pattern) const;
	void set_polling_interval(std::uint8_t) const;
	void save() const;

  protected:
	nlohmann::json serialize_current_config() const noexcept override final;
	void           deserialize_config(
	              nlohmann::json const& config_on_disk) override final;

  private:
	struct {
		std::uint8_t      backlight_luminosity;
		backlight_pattern pattern;
		std::uint8_t      polling_interval;
	} m_config;
};

class rival_3_wireless : public driver
{
  public:
	rival_3_wireless(std::shared_ptr<usb_device>     dev,
	                 std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		deserialize_config(config->get_device_config(config_id()));
	}

	static bool is_compatible(std::shared_ptr<usb_device>);

	std::string config_id() const noexcept final
	{
		return "steelseries:apex_100";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Rival 3 Wireless";
	};

	const std::unordered_map<std::string, action const> get_actions()
	    const noexcept final;

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters) final;

	void set_static_color(std::uint8_t r, std::uint8_t g, std::uint8_t b) const;
	void set_dpi(std::uint8_t               active_profile_id,
	             std::vector<std::uint16_t> dpi_profiles) const;
	void set_poll_interval(std::uint8_t interval) const;
	void set_powersaving_options(bool          ultra_power_saving,
	                             bool          smart_lighting,
	                             std::uint16_t sleep_time) const;
	void save() const;

  protected:
	nlohmann::json serialize_current_config() const noexcept override final;
	void           deserialize_config(
	              nlohmann::json const& config_on_disk) override final;

  private:
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
