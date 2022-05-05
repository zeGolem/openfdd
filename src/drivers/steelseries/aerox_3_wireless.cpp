#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "drivers/driver.hpp"
#include "steelseries.hpp"
#include <cstddef>
#include <memory.h>
#include <stdexcept>
#include <vector>

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
	action dpi_profile{
	    .description = "Set the current DPI profile",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "profile",
	         .description = "The DPI profile to switch to",
        }},
	};

	action define_dpi_profile{
	    .description = "Set the value of a DPI profile",
 // Clang doesn't like this for some reason :/
  // clang-format off
	    .parameters  = {
			{
				.type = parameter::param_uint,
				.name = "profile",
				.description = "The profile to update",
			},
			{
				.type = parameter::param_uint,
				.name = "dpi",
				.description = "The DPI to set the profile to",
			},
		},
  // clang-format on
	};

	action save{
	    .description = "Save to onboard memory",
	    .parameters  = {},
	};

	return {
	    {       "dpi_profile",        dpi_profile},
	    {"define_dpi_profile", define_dpi_profile},
	    {              "save",               save},
	};
}

void aerox_3_wireless::run_action(std::string const&              action_id,
                                  std::vector<std::string> const& parameters)
{
	if (action_id == "dpi_profile") {
		if (parameters.size() < 1)
			throw std::runtime_error("Missig arguements for dpi_profile");

		std::uint8_t profile;
		try {
			auto _profile = std::stoi(parameters[0]);
			if (_profile < 1 || _profile > 5)
				throw std::runtime_error("Invalid DPI profile");
			profile = _profile;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("DPI profile value isn't a number (" +
			                         parameters[0] + ")");
		}

		m_config.active_dpi_profile = profile;
		set_dpi(profile, m_config.dpi_profiles);
		save_config();

	} else if (action_id == "define_dpi_profile") {
		if (parameters.size() < 2)
			throw std::runtime_error(
			    "Missig arguements for define_dpi_profile");

		std::uint8_t  profile;
		std::uint16_t value;
		try {
			auto _profile = std::stoi(parameters[0]);
			if (_profile < 1 || _profile > 5)
				throw std::runtime_error("Invalid DPI profile");
			profile = _profile;

			auto _value = std::stoi(parameters[1]);
			if (_value < 100 || _value > 18000)
				throw std::runtime_error("Invalid DPI value");
			value = _value;
		} catch (std::invalid_argument const& e) {
			throw std::runtime_error("Error trying to parse string");
		}

		m_config.dpi_profiles[profile - 1] = value;
		set_dpi(m_config.active_dpi_profile, m_config.dpi_profiles);
		save_config();

	} else if (action_id == "save") {
		save();
		save_config();

	} else
		throw std::runtime_error("Invalid action id: " + action_id);
}

void aerox_3_wireless::set_dpi(std::uint8_t               active_profile_id,
                               std::vector<std::uint16_t> dpi_profiles) const
{
	if (active_profile_id < 1 || active_profile_id > 5)
		throw std::runtime_error("Invalid profile id, must be in [1,5]");
	--active_profile_id; // The profile ID is from 1 to 5 to be human
	                     // readable, but the driver wants it to be 0 to 4,
	                     // so this converts it.

	if (dpi_profiles.size() > 5)
		throw std::runtime_error("Too many DPI profiles, must be <5");

	std::vector<std::uint8_t> data = {
	    0x6d, // Packet ID
	    (std::uint8_t)
	        dpi_profiles.size(), // It's safe to cast to u8 because we check
	                             // the value at the sstart of the function
	    active_profile_id,
	    // Those are the DPI values for each profile. We will set them right
	    // after
	    // clang-format off
	    0, 0, 0, 0, 0,
	    // clang-format on
	};

	// The driver takes a DPI value between 0 and 0xd6, which is equivalent
	// to the 100->18000 DPI we can set in the software. This function
	// converts from the 100-18000 human-readable value to the driver value
	// for 0 to 0xd6
	auto to_driver_dpi = [](std::uint16_t human_dpi) -> std::uint8_t {
		float unscaled_value = ((float)human_dpi - 100) / (18000 - 100);
		// 0xd6 is the driver DPI equivalent of 18000
		return unscaled_value * 0xd6;
	};

	for (std::size_t i = 0; i < dpi_profiles.size(); ++i)
		data[i + 3] /* The DPI profiles start at data[3] */ =
		    to_driver_dpi(dpi_profiles[i]);

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void aerox_3_wireless::save() const
{
	// 0x51 is the save command ID
	m_device->control_transfer(0x21, 0x09, 0x0200, 3, {0x51}, 1000);
}

nlohmann::json aerox_3_wireless::serialize_current_config() const noexcept
{
	return {
	    {"active_dpi_profile", m_config.active_dpi_profile},
	    {      "dpi_profiles",       m_config.dpi_profiles},
	};
}

void aerox_3_wireless::deserialize_config(nlohmann::json const& config_on_disk)
{
	m_config.active_dpi_profile = config_on_disk.value("active_dpi_profile", 1);
	m_config.dpi_profiles = config_on_disk.value<std::vector<std::uint16_t>>(
	    "dpi_profiles", {400, 800, 1200, 2400, 3200});
}

} // namespace steelseries
} // namespace drivers
