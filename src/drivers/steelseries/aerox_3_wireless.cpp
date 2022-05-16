#include "drivers/steelseries/aerox_3_wireless.hpp"
#include "drivers/driver.hpp"
#include "steelseries.hpp"
#include "utils.hpp"
#include <array>
#include <cstddef>
#include <iostream>
#include <memory.h>
#include <stdexcept>
#include <string>
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

void aerox_3_wireless::create_actions() noexcept
{
#define CREATE_ACTION_HANDLER(action_name)                                     \
	auto action_name##_handler = [this](                                       \
	    std::vector<std::string> const& parameters)

#define REGISTER_ACTION(action_name)                                           \
	register_action(#action_name, action_name, action_name##_handler)

#define CHECK_PARAMS_SIZE(minimum_size, action_name)                           \
	if (parameters.size() < minimum_size)                                      \
		throw std::runtime_error("Missing arguements for " #action_name);

	action dpi_profile{
	    .description = "Set the current DPI profile",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "profile",
	         .description = "The DPI profile to switch to",
        }},
	};

	CREATE_ACTION_HANDLER(dpi_profile)
	{
		CHECK_PARAMS_SIZE(1, dpi_profile);

		std::uint8_t profile =
		    utils::stoi_safe(parameters[0], {.min = 1, .max = 5}, "Profile");

		m_config.active_dpi_profile = profile;
		set_dpi(profile, m_config.dpi_profiles);
		save_config();
	};

	REGISTER_ACTION(dpi_profile);

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

	CREATE_ACTION_HANDLER(define_dpi_profile)
	{
		CHECK_PARAMS_SIZE(2, define_dpi_profile);

		std::uint8_t profile = utils::stoi_safe(
		    parameters[0], {.min = 1, .max = 5}, "DPI Profile");
		std::uint16_t value = utils::stoi_safe(
		    parameters[1], {.min = 100, .max = 18000}, "DPI Value");

		m_config.dpi_profiles[profile - 1] = value;
		set_dpi(m_config.active_dpi_profile, m_config.dpi_profiles);
		save_config();
	};

	REGISTER_ACTION(define_dpi_profile);

	action lighting_color{
	    .description = "Set the color of the lights",
 // Clang doesn't like this for some reason :/
  // clang-format off
	    .parameters = {
			{
				.type        = parameter::param_uint,
				.name        = "zone",
				.description = "The zone to edit (between 1 and 3)",
			},
			{
				 .type = parameter::param_string,
				 .name = "color",
				 .description =
				 "The RGB color to change it to (ex. 'FFFFFF')",
			},
		},
  // clang-format on
	};

	CREATE_ACTION_HANDLER(lighting_color)
	{
		CHECK_PARAMS_SIZE(2, lighting_color);

		std::uint8_t zone =
		    utils::stoi_safe(parameters[0], {.min = 1, .max = 3}, "Color zone");

		// TODO Error checking...
		auto         color = parameters[1];
		std::uint8_t r     = std::stoi(color.substr(0, 2), nullptr, 16);
		std::uint8_t g     = std::stoi(color.substr(2, 2), nullptr, 16);
		std::uint8_t b     = std::stoi(color.substr(4, 2), nullptr, 16);

		m_config.lighting_colors = {r, g, b};
		set_lighting_color(zone, {r, g, b});
		save_config();
	};

	REGISTER_ACTION(lighting_color);

	action polling_interval{
	    .description = "Sets the polling interval",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "interval",
	         .description = "Interval between polls (1 to 4, 1 is faster)",
        }},
	};

	CREATE_ACTION_HANDLER(polling_interval)
	{
		CHECK_PARAMS_SIZE(1, polling_interval);

		std::uint8_t interval =
		    utils::stoi_safe(parameters[0], {.min = 1, .max = 4}, "Interval");

		m_config.poll_interval = interval;
		set_poll_interval(interval);
		save_config();
	};

	REGISTER_ACTION(polling_interval);

	action sleep_timeout{
	    .description = "Sets the sleep timeout, in seconds",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "timeout",
	         .description = "The time in second before the mouse goes to sleep",
        }},
	};

	CREATE_ACTION_HANDLER(sleep_timeout)
	{
		CHECK_PARAMS_SIZE(1, sleep_timeout);

		std::uint32_t timeout = utils::stoi_safe(
		    parameters[0], {.min = 0, .max = 1'200'000}, "Timeout");

		m_config.sleep_timeout = timeout;
		set_sleep_timeout(timeout);
		save_config();
	};

	REGISTER_ACTION(sleep_timeout);

	action save{
	    .description = "Save to onboard memory",
	    .parameters  = {},
	};

	CREATE_ACTION_HANDLER(save)
	{
		(void)parameters;
		this->save();
		save_config();
	};

	REGISTER_ACTION(save);

#undef CREATE_ACTION_HANDLER
#undef REGISTER_ACTION
#undef CHECK_PARAMS_SIZE
}

void aerox_3_wireless::set_dpi(std::uint8_t               active_profile_id,
                               std::vector<std::uint16_t> dpi_profiles) const
{
	utils::ensure_range(active_profile_id, 1, 5, "Profile ID");
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
		return utils::scale((float)human_dpi, 100, 18'000, 0, 0xd6);
	};

	for (std::size_t i = 0; i < dpi_profiles.size(); ++i)
		data[i + 3] /* The DPI profiles start at data[3] */ =
		    to_driver_dpi(dpi_profiles[i]);

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void aerox_3_wireless::set_lighting_color(
    std::uint8_t zone, std::array<std::uint8_t, 3> color) const
{
	utils::ensure_range(zone, 1, 3, "Zone");
	--zone; // The zone ID is from 1 to 3 to be human readable, but the
	        // driver wants it to be 0 to 2, so this converts it.

	std::vector<std::uint8_t> data = {
	    // clang-format off
	    0x61, 0x01, // Packet ID
	    zone,
		color[0], color[1], color[2],
	    // clang-format on
	};

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void aerox_3_wireless::set_poll_interval(std::uint8_t interval) const
{
	utils::ensure_range(interval, 1, 4, "Interval");
	--interval; // The interval is from 1 to 4 to be human readable, but the
	            // driver wants it to be 0 to 3, so this converts it.

	std::vector<std::uint8_t> data = {
	    0x6b, // Packet ID
	    interval,
	};

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void aerox_3_wireless::set_sleep_timeout(std::uint32_t timeout_in_seconds) const
{
	std::vector<std::uint8_t> data = {
	    0x69, // Nice packet ID
	    // Timeout value (in reverse byte order to match what the driver
	    // expects).
	    // This uses some bit manipulation magic that I don't fully
	    // understand, but it works, and the code looks nice, so I'm fine
	    // with it
	    static_cast<std::uint8_t>((timeout_in_seconds & 0x000000ff) >> 0),
	    static_cast<std::uint8_t>((timeout_in_seconds & 0x0000ff00) >> 8),
	    static_cast<std::uint8_t>((timeout_in_seconds & 0x00ff0000) >> 16),
	    static_cast<std::uint8_t>((timeout_in_seconds & 0xff000000) >> 24),

	};

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
	    {   "lighting_colors",    m_config.lighting_colors},
	    {     "poll_interval",      m_config.poll_interval},
	    {     "sleep_timeout",      m_config.sleep_timeout},
	};
}

void aerox_3_wireless::deserialize_config(nlohmann::json const& config_on_disk)
{
	m_config.active_dpi_profile = config_on_disk.value("active_dpi_profile", 1);
	m_config.dpi_profiles = config_on_disk.value<std::vector<std::uint16_t>>(
	    "dpi_profiles", {400, 800, 1200, 2400, 3200});
	m_config.lighting_colors =
	    config_on_disk.value<std::array<std::uint8_t, 3>>("lighting_colors",
	                                                      {0xff, 0xff, 0xff});
	m_config.poll_interval = config_on_disk.value("poll_interval", 1);
	m_config.sleep_timeout = config_on_disk.value("sleep_timeout", 0);
}

} // namespace steelseries
} // namespace drivers
