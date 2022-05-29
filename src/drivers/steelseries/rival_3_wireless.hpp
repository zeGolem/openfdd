#pragma once

#include "drivers/driver.hpp"
#include "usb/device.hpp"
#include <memory.h>

namespace drivers
{
namespace steelseries
{

class rival_3_wireless final : public driver
{
  public:
	rival_3_wireless(std::shared_ptr<usb::device>    dev,
	                 std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		create_actions();
		deserialize_config(config->get_device_config(config_id()));
	}

	static bool is_compatible(std::shared_ptr<usb::device>);

	std::string config_id() const noexcept final
	{
		return "steelseries:rival_3_wireless";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Rival 3 Wireless";
	};

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

	void create_actions() noexcept override final;

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
