#pragma once
#include "drivers/driver.hpp"
#include "usb.hpp"

namespace drivers
{
namespace steelseries
{

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

} // namespace steelseries
} // namespace drivers
