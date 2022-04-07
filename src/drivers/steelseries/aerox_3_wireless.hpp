#pragma once
#include "drivers/driver.hpp"
#include "usb.hpp"

namespace drivers
{
namespace steelseries
{

class aerox_3_wireless : public driver
{
  public:
	aerox_3_wireless(std::shared_ptr<usb_device>     dev,
	         std::shared_ptr<config_manager> config)
	    : driver(dev, config)
	{
		deserialize_config(config->get_device_config(config_id()));
	}

	static bool is_compatible(std::shared_ptr<usb_device>);

	std::string config_id() const noexcept final
	{
		return "steelseries:aerox_3_wireless";
	}

	const std::string name() const noexcept final
	{
		return "SteelSeries Aerox 3 Wireless";
	};

	const std::unordered_map<std::string, action const> get_actions()
	    const noexcept final;

	void run_action(std::string const&              action_id,
	                std::vector<std::string> const& parameters) final;


  protected:
	nlohmann::json serialize_current_config() const noexcept override final;
	void           deserialize_config(
	              nlohmann::json const& config_on_disk) override final;

  private:
	struct {
	} m_config;
};

} // namespace steelseries
} // namespace drivers
