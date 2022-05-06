#include "rival_3_wireless.hpp"
#include "steelseries.hpp"
#include "usb.hpp"
#include <memory.h>
#include <string>
#include <vector>

namespace drivers
{
namespace steelseries
{

bool rival_3_wireless::is_compatible(std::shared_ptr<usb_device> device)
{
	auto id = device->get_id();
	if (id.id_vendor == steelseries::vendor_id && id.id_product == 0x1830)
		return true;
	return false;
}

void rival_3_wireless::create_actions() noexcept
{
	// TODO: Use those macros to create the actions and register them, like in
	// steelseries::aerox_3_wireless
#define CREATE_ACTION_HANDLER(action_name)                                     \
	auto action_name##_handler = [this](                                       \
	    std::vector<std::string> const& parameters)

#define REGISTER_ACTION(action_name)                                           \
	register_action(#action_name, action_name, action_name##_handler)

	action dpi_presset{
	    .description = "Set the active DPI presset",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "presset",
	         .description = "The presset to enable (1-5)",
        }},
	};

	auto dpi_presset_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 1)
			    throw std::runtime_error("Missig arguements for dpi_presset");

		    std::uint8_t profile;
		    try {
			    auto val = std::stoi(parameters[0]);
			    if (val < 1 || val > 5)
				    throw std::runtime_error("Invalid profile");
			    profile = val;
		    } catch (std::invalid_argument const& e) {
			    throw std::runtime_error("Couldn't parse number.");
		    }

		    m_config.active_profile = profile;
		    set_dpi(profile, m_config.dpi_values);
		    save_config();
	    };

	register_action("dpi_presset", dpi_presset, dpi_presset_handler);

	action dpi_presset_config{
	    .description = "Configure a DPI presset",
	    .parameters =
	        {// clang-format off
				{
					.type        = parameter::param_uint,
					.name        = "presset",
					.description = "The presset to change (1-5)",
				},
				{
					.type        = parameter::param_uint,
					.name        = "value",
					.description = "DPI value (100-18000)",
				}
			},
	    // clang-format on
	};

	auto dpi_presset_config_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 2)
			    throw std::runtime_error(
			        "Missig arguements for dpi_presset_config");

		    std::uint8_t  profile;
		    std::uint16_t new_value;
		    try {
			    auto _profile = std::stoi(parameters[0]);
			    if (_profile < 1 || _profile > 5)
				    throw std::runtime_error("Invalid profile");
			    profile = _profile;

			    auto _new_value = std::stoi(parameters[1]);
			    if (_new_value < 100 || _new_value > 18000)
				    throw std::runtime_error("Invalid DPI value");
			    new_value = _new_value;

		    } catch (std::invalid_argument const& e) {
			    throw std::runtime_error("Couldn't parse number.");
		    }

		    m_config.dpi_values[profile - 1] = new_value;
		    set_dpi(m_config.active_profile, m_config.dpi_values);
		    save_config();
	    };

	register_action(
	    "dpi_presset_config", dpi_presset_config, dpi_presset_config_handler);

	action poll_interval{
	    .description = "Set the poll interval",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "interval",
	         .description = "Interval between polls, in ms (1-4)",
        }},
	};

	auto poll_interval_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 1)
			    throw std::runtime_error(
			        "Missig arguements for set_poll_interval");

		    std::uint8_t interval;
		    try {
			    auto _interval = std::stoi(parameters[0]);
			    if (_interval > 4 || _interval < 1)
				    throw std::runtime_error(
				        "Invalid poll interval, value must be between 1 and 4");
			    interval = _interval;
		    } catch (std::invalid_argument const& e) {
			    throw std::runtime_error("Couldn't parse number.");
		    }

		    // TODO: Save to m_config
		    set_poll_interval(interval);
	    };

	register_action("poll_interval", poll_interval, poll_interval_handler);

	action ultra_power_saving{
	    .description = "Ultra power saving mode",
	    .parameters  = {{
	         .type = parameter::param_string,
	         .name = "enabled",
	         .description =
                "Is Ultra Power Saving mode enabled? ('true' or 'false')",
        }},
	};

	auto ultra_power_saving_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 1)
			    throw std::runtime_error(
			        "Missig arguements for ultra_power_saving");

		    bool is_active = parameters[0] == "true";

		    m_config.ultra_power_saving_mode = is_active;
		    set_powersaving_options(
		        is_active, m_config.smart_lighting_mode, m_config.sleep_time);
		    save_config();
	    };

	register_action(
	    "ultra_power_saving", ultra_power_saving, ultra_power_saving_handler);

	action smart_lighting{
	    .description = "Smart lighting mode",
	    .parameters  = {{
	         .type = parameter::param_string,
	         .name = "enabled",
	         .description =
                "Is smart lighting mode enabled? ('true' or 'false')",
        }},
	};

	auto smart_lighting_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 1)
			    throw std::runtime_error(
			        "Missig arguements for smart_lighting");

		    bool is_active = parameters[0] == "true";

		    m_config.smart_lighting_mode = is_active;
		    set_powersaving_options(m_config.ultra_power_saving_mode,
		                            is_active,
		                            m_config.sleep_time);
		    save_config();
	    };

	register_action("smart_lighting", smart_lighting, smart_lighting_handler);

	action sleep_time{
	    .description = "Sleep time",
	    .parameters  = {{
	         .type        = parameter::param_uint,
	         .name        = "time",
	         .description = "Time before going to sleep (in seconds)",
        }},
	};

	auto sleep_time_handler =
	    [this](std::vector<std::string> const& parameters) {
		    if (parameters.size() < 1)
			    throw std::runtime_error("Missig arguements for sleep_time");

		    std::uint16_t sleep_time;
		    try {
			    auto _sleep_time = std::stoi(parameters[0]);
			    // TODO: Is there a max. value? The GUI goes up to 20 minutes...
			    if (_sleep_time < 0)
				    throw std::runtime_error("Invalid value for sleep_time");

			    sleep_time = _sleep_time;
		    } catch (std::runtime_error const& e) {
			    throw std::runtime_error("Couldn't parse number");
		    }

		    m_config.sleep_time = sleep_time;
		    set_powersaving_options(m_config.ultra_power_saving_mode,
		                            m_config.smart_lighting_mode,
		                            sleep_time);
		    save_config();
	    };

	register_action("sleep_time", sleep_time, sleep_time_handler);

	action save{
	    .description = "Save to onboard memory",
	    .parameters  = {},
	};

	auto save_handler = [this](std::vector<std::string> const&) {
		this->save();
		save_config();
	};

	register_action("save", save, save_handler);

#undef CREATE_ACTION_HANDLER
#undef REGISTER_ACTION
}

void rival_3_wireless::set_dpi(std::uint8_t               active_profile_id,
                               std::vector<std::uint16_t> dpi_profiles) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x20;                // This is the ID
	data[1] = dpi_profiles.size(); // Profile count
	data[2] = active_profile_id;   // Profile ID

	// Need to scale the dpi from 100-18000 to 0-0xd6
	std::vector<std::uint8_t> scaled_dpis(dpi_profiles.size());

	auto scale_value = [](std::uint16_t dpi) -> std::uint8_t {
		auto new_val = ((float)(dpi - 100) / (18000 - 100)) * 0xd6;
		if (new_val > 0xd6) return 0xd6;
		if (new_val < 0) return 0;
		return new_val;
	};

	// TODO: Handle profile_count < 5 properly
	for (std::size_t i = 0; i < dpi_profiles.size(); ++i)
		scaled_dpis[i] = scale_value(dpi_profiles[i]);

	data[3]  = scaled_dpis[0]; // Profile 1 DPI
	data[5]  = scaled_dpis[1]; // Profile 2 DPI
	data[7]  = scaled_dpis[2]; // Profile 3 DPI
	data[9]  = scaled_dpis[3]; // Profile 4 DPI
	data[11] = scaled_dpis[4]; // Profile 5 DPI

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::set_poll_interval(std::uint8_t interval) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x17; // This is the ID
	data[1] = interval + 1;

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::set_powersaving_options(bool          ultra_power_saving,
                                               bool          smart_lighting,
                                               std::uint16_t sleep_time) const
{
	// The data to send. This is a bit more than what we use for now
	std::vector<std::uint8_t> data(16);
	data.assign(16, 0); // zero-out all the data by default

	data[0] = 0x2b; // This is the ID
	data[1] = ultra_power_saving;

	// TODO: I don't really know how this value works :/
	data[2] = smart_lighting ? 0x32 : 0x00;
	if (ultra_power_saving) data[2] = 0x64;

	// TODO: data[3] && data[4] are probably for storing a delay, possibly the
	//       smart lighting delay, but I haven't tested yet.
	data[3] = 0x2c;
	data[4] = 0x01;

	data[5] = sleep_time & 0xff;        // Take the bottom 8 bits of sleep_time
	data[6] = (sleep_time >> 8) & 0xff; // Take the top 8 bits of sleep_time

	m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
}

void rival_3_wireless::save() const
{
	m_device->control_transfer(0x21, 0x09, 0x0200, 3, {0x09}, 1000);
}

// TODO: This doesn't do anything ;(
void rival_3_wireless::set_static_color(std::uint8_t r,
                                        std::uint8_t g,
                                        std::uint8_t b) const
{
	// ================ NOTE =================
	// This is very much a work in progress,
	// I have yet to figure out how most of it
	// all works. It seems more complicated
	// due to the mouse being wireless
	// =======================================

	// Part 1: Sending the color
	{
		// Create a vector for the USB data we're going to send.
		// We will send 64 bytes
		// TODO: Only 48 bytes seem to be used?
		std::vector<std::byte> data(64);

		data.assign(64,
		            std::byte{0x00}); // Make sure everything is 0-initialized

		// This is what setting the color to ff ff ff looks like:
		//
		// 03 00 00 00 30 00 88 13 00 00 00 00 00 00 00 00 00 00 00 00 00 00
		// 01 00 00 00 00 00 00 00 01 ff ff ff ff ff ff 00 00 00

		data[0] = std::byte{0x03}; // Should be the command ID, though it
		                           // seems to be shared by many functions
		                           // of the Windows software...
		// This is just matching the hexdump I have for setting ff ff ff...
		// I have no idea what any of those mean
		data[4]  = std::byte{0x30};
		data[6]  = std::byte{0x88};
		data[7]  = std::byte{0x13};
		data[22] = std::byte{0x01};
		data[30] = std::byte{0x01};

		// The color (twice in a row, still haven't figured out why
		data[31] = std::byte{r};
		data[32] = std::byte{g};
		data[33] = std::byte{b};
		data[34] = std::byte{r};
		data[35] = std::byte{g};
		data[36] = std::byte{b};

		// m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
	}
	// Part 2: Sending the weird packet to apply it..?
	{
		// Create a vector for the USB data we're going to send.
		// We will send 5 bytes
		std::vector<std::byte> data(64);
		data.assign(64,
		            std::byte{0x00}); // Make sure everything is 0-initialized

		// This is what one of these weird packets look like
		//
		// 03 00 30 00 2c

		data[0] = std::byte{0x03}; // Should be the command ID, though it
		                           // seems to be shared by many functions
		                           // of the Windows software...
		// This is just matching the hexdump I have for setting ff ff ff...
		// I have no idea what any of those mean
		data[1] = std::byte{0x00};
		data[2] = std::byte{0x30};
		data[3] = std::byte{0x00};
		data[4] = std::byte{0x2c};

		// m_device->control_transfer(0x21, 0x09, 0x0200, 3, data, 1000);
	}
}

nlohmann::json rival_3_wireless::serialize_current_config() const noexcept
{
	return {
	    {             "dpi_values",              m_config.dpi_values},
	    {	     "active_profile",          m_config.active_profile},
	    {"ultra_power_saving_mode", m_config.ultra_power_saving_mode},
	    {    "smart_lighting_mode",     m_config.smart_lighting_mode},
	    {             "sleep_time",              m_config.sleep_time},
	};
}

// TODO: Check if values are valid?
void rival_3_wireless::deserialize_config(nlohmann::json const& config_on_disk)
{
	auto const& config_dpi_values =
	    config_on_disk.value<std::vector<std::uint16_t>>("dpi_values", {});
	m_config.dpi_values = config_dpi_values;

	m_config.active_profile = config_on_disk.value("active_profile", 1);

	m_config.ultra_power_saving_mode =
	    config_on_disk.value("ultra_power_saving_mode", false);

	m_config.smart_lighting_mode =
	    config_on_disk.value("smart_lighting_mode", true);

	m_config.sleep_time = config_on_disk.value("sleep_time", 300);
}

} // namespace steelseries
} // namespace drivers
