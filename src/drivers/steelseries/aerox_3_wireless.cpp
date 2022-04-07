#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "steelseries.hpp"
#include <memory.h>

namespace drivers
{
namespace steelseries
{

bool aerox_3_wireless::is_compatible(std::shared_ptr<usb_device> dev)
{
	auto id = dev->get_id();
	if (id.id_vendor == steelseries::vendor_id && id.id_product == 0x1838)
		return true;
	return false;
}

const std::unordered_map<std::string, action const> aerox_3_wireless::
    get_actions() const noexcept
{
	return {};
}

void aerox_3_wireless::run_action(std::string const& action_id,
                                  std::vector<std::string> const&)
{
	throw std::runtime_error("Invalid action id: " + action_id);
}

nlohmann::json aerox_3_wireless::serialize_current_config() const noexcept
{
	return nlohmann::json::object_t{};
}

void aerox_3_wireless::deserialize_config(nlohmann::json const&) {}

} // namespace steelseries
} // namespace drivers
