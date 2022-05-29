#pragma once
#include "drivers/driver.hpp"
#include "usb/device.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace drivers
{
namespace steelseries
{

class aerox_3_wireless final : public driver
{
  public:
	aerox_3_wireless(std::shared_ptr<usb::device>    dev,
	                 std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		deserialize_config(config->get_device_config(config_id()));
		create_actions();
	}

	static bool is_compatible(std::shared_ptr<usb::device>);

	std::string config_id() const noexcept final
	{
		return "steelseries:aerox_3_wireless";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Aerox 3 Wireless";
	};

	void set_dpi(std::uint8_t               active_profile_id,
	             std::vector<std::uint16_t> dpi_profiles) const;
	void set_lighting_color(std::uint8_t                zone,
	                        std::array<std::uint8_t, 3> color) const;
	void set_poll_interval(std::uint8_t interval) const;
	void set_sleep_timeout(std::uint32_t timeout) const;
	void save() const;

  protected:
	nlohmann::json serialize_current_config() const noexcept override final;
	void           deserialize_config(
	              nlohmann::json const& config_on_disk) override final;

	virtual void create_actions() noexcept override final;

  private:
	struct {
		std::uint8_t                active_dpi_profile = 1;
		std::vector<std::uint16_t>  dpi_profiles = {400, 800, 1200, 2400, 3200};
		std::array<std::uint8_t, 3> lighting_colors = {0xff, 0xff, 0xff};
		std::uint8_t                poll_interval   = 1;
		std::uint32_t               sleep_timeout   = 0;
	} m_config;
};

} // namespace steelseries
} // namespace drivers
